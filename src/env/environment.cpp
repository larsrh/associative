#include <vector>

#include <boost/uuid/uuid_io.hpp>

#include "environment.hpp"

associative::Environment::Environment(const boost::shared_ptr<Process>& process, const boost::shared_ptr<VFS>& vfs, const boost::shared_ptr<Connection>& conn, const boost::shared_ptr<Logger>& logger)
: id(boost::none), process(process), vfs(vfs), conn(conn), logger(logger)
{
}

associative::Environment::~Environment()
{
	buffer.clear();
}

associative::VFS& associative::Environment::getVFS()
{
	return *vfs;
}

associative::Connection& associative::Environment::getConnection()
{
	return *conn;
}

boost::optional<uint64_t> associative::Environment::getSessionID()
{
	return id;
}

void associative::Environment::cleanSessions(bool)
{
	auto t = conn->transaction();
	// TODO rollback open sessions which don't exist any more
	
	conn->executeStatement(
		"delete from handle where not exists ("
		"  select * from session where id = handle.session_id"
		")"
	);
	
	t->commit();
}

void associative::Environment::startSession()
{
	if (id)
		throw Exception("already in a session");
	
	auto t = conn->transaction();
	id = conn->nextID("session");
	auto stmt = conn->prepareStatement("insert into session values (?, 0, ?)", std::string("env.session.add"));
	stmt->execute(convertAll(*id, getpid()));
	t->commit();
}

void associative::Environment::commitSession(const associative::IsolationLevel& level)
{
	if (!id)
		throw Exception("not in a session");
	
	auto lock = process->getMemLock("session")->timedLockOrThrow();
	
	auto dbT = conn->transaction();
	
	// Step 0.1: Set to 'ready'
	// TODO check whether transaction is already 'ready'
	auto stmt = conn->prepareStatement("update session set ready = 1 where id = ?", std::string("env.session.ready"));
	stmt->execute(convertAll(*id));
	
	// Step 0.2: Check whether all relevant handles are closed
	bool isolated = level.isIsolated(conn, *id);
	
	boost::optional<CommitException::Reason> reason;
	if (isolated)
	{
		// Step 0.3: Make sure no other session invalidated this one
		// (a) store into a blob which has been removed
		// (b) add metadata to a blob which has been removed
		auto query = conn->prepareQuery(
			"select * from journal "
			"where journal.session_id = ? and journal.relation = ? "
			"and journal.operation = ? and not exists (select * from `blob` where blob.id = journal.relation_id) "
			"union all "
			"select journal.* from journal inner join metadata "
			"on journal.relation = ? and journal.relation_id = metadata.id "
			"where journal.session_id = ? and ("
			"  not exists (select * from `blob` where blob.id = metadata.blob_id) or "
			"  (metadata.object_type_id = ? and not exists (select * from `blob` where blob.id = metadata.object))"
			")",
		std::string("env.session.invalid"));
		if (query->execute(convertAll(*id, Connection::Relation::Blob, Blob::Operation::Store, Connection::Relation::Metadata, *id, ASSOCIATIVE_SYS_BLOB_TYPE)).rows.size())
			reason = CommitException::Reason::Invalidated;
	}
	else
	{
		reason = CommitException::Reason::ConflictingHandles;
	}
	
	dbT->commit();
	
	if (reason)
		throw CommitException((boost::format("session %1% cannot be commited") % *id).str(), *reason);
	
	auto vfsT = vfs->apply(*this);
	
	dbT = conn->transaction();
	
	// Step 1: Make new files visible
	stmt = conn->prepareStatement(
		"update file set visible = 1 where exists ("
		"  select * from journal "
		"  where journal.relation_id = file.id and journal.relation = ? "
		"  and journal.operation = ? and journal.session_id = ? "
		")",
	std::string("env.session.file.add"));
	stmt->execute(convertAll(Connection::Relation::File, File::Operation::Add, *id));
	
	// Step 2: Make new blobs visible
	stmt = conn->prepareStatement(
		"update `blob` set visible = 1 where exists ("
		"  select * from journal "
		"  where journal.relation_id = blob.id and journal.relation = ? "
		"  and journal.operation = ? and journal.session_id = ? "
		")",
	std::string("env.session.blob.add"));
	stmt->execute(convertAll(Connection::Relation::Blob, Blob::Operation::Add, *id));
	
	// Step 3: Remove blobs
	stmt = conn->prepareStatement(
		"delete from `blob` where exists ("
		"  select * from journal "
		"  where journal.relation_id = blob.id and journal.relation = ? "
		"  and journal.operation = ? and journal.session_id = ? "
		")",
	std::string("env.session.blob.remove"));
	stmt->execute(convertAll(Connection::Relation::Blob, Blob::Operation::Remove, *id));
	
	// Step 4: Make new metadata visible
	stmt = conn->prepareStatement(
		"update metadata set visible = 1 where exists ("
		"  select * from journal "
		"  where journal.relation_id = metadata.id and journal.relation = ? "
		"  and journal.operation = ? and journal.session_id = ? "
		")",
	std::string("env.session.metadata.add"));
	stmt->execute(convertAll(Connection::Relation::Metadata, Triple::Operation::Add, *id));
	
	// Step 5: Remove metadata
	stmt = conn->prepareStatement(
		"delete from metadata where exists ("
		"  select * from journal "
		"  where journal.relation_id = metadata.id and journal.relation = ? "
		"  and journal.operation = ? and journal.session_id = ? "
		")",
	std::string("env.session.blob.remove"));
	stmt->execute(convertAll(Connection::Relation::Metadata, Triple::Operation::Remove, *id));
	
	// Step 6: Flush journal
	stmt = conn->prepareStatement("delete from journal where session_id = ?", std::string("env.session.journal.flush"));
	stmt->execute(convertAll(*id));
	
	// Step 7: Remove session
	stmt = conn->prepareStatement("delete from session where id = ?", std::string("env.session.remove"));
	stmt->execute(convertAll(*id));
	
	dbT->commit();
	
	vfsT->finish();
	
	buffer.clear();
	
	id = boost::none;
}

void associative::Environment::rollbackSession()
{
	if (!id)
		throw Exception("not in a session");
	
	buffer.clear();
	
	auto t = conn->transaction();
	auto stmt = conn->prepareStatement(
		"delete from file where exists ("
		"  select * from journal"
		"  where relation = ? and session_id = ? and relation_id = file.id"
		")",
	std::string("env.session.rollback.files"));
	stmt->execute(convertAll(Connection::Relation::File, *id));
	
	stmt = conn->prepareStatement(
		"delete from `blob` where exists ("
		"  select * from journal"
		"  where relation = ? and session_id = ? and relation_id = blob.id"
		")",
	std::string("env.session.rollback.blobs"));
	stmt->execute(convertAll(Connection::Relation::Blob, *id));
	
	stmt = conn->prepareStatement(
		"delete from metadata where exists ("
		"  select * from journal"
		"  where relation = ? and session_id = ? and relation_id = metadata.id"
		")",
	std::string("env.session.rollback.blobs"));
	stmt->execute(convertAll(Connection::Relation::Metadata, *id));
	
	stmt = conn->prepareStatement("delete from journal where session_id = ?", std::string("env.session.rollback.journal"));
	stmt->execute(convertAll(*id));
	
	stmt = conn->prepareStatement("delete from session where id = ?", std::string("env.session.remove"));
	stmt->execute(convertAll(*id));
	t->commit();
	
	id = boost::none;
}

associative::WeakPtr<associative::File> associative::Environment::createFile()
{
	if (!id)
		throw Exception("not in a session");
	
	boost::shared_ptr<File> ptr(new File(*this));
	buffer.set(boost::lexical_cast<std::string>(ptr->uuid), ptr);
	return ptr;
}

associative::WeakPtr<associative::File> associative::Environment::getFile(const std::string& uuid)
{
	if (!id)
		throw Exception("not in a session");
	
	auto func = [&]() { return boost::shared_ptr<File>(new File(*this, boost::lexical_cast<boost::uuids::uuid>(uuid))); };
	return buffer.getOrElse(uuid, func);
}

associative::CommitException::CommitException(const std::string& message, const associative::CommitException::Reason& reason)
: Exception(message), reason(reason)
{
}

associative::CommitException::~CommitException() throw()
{
}