#include <boost/uuid/uuid_io.hpp>
#include <boost/functional.hpp>
#include <boost/lambda/construct.hpp>

#include "vfs.hpp"
#include "../util/io.hpp"

associative::VFS::Operation::~Operation()
{
}

associative::VFS::Move::Move(const fs::path& src, const fs::path& dest, const boost::optional<fs::path>& destTemp)
: src(src), dest(dest), destTemp(destTemp)
{
}

associative::VFS::Move::~Move()
{
}

void associative::VFS::Move::apply()
{
	if (destTemp)
		fs::rename(dest, *destTemp);
	fs::create_directories(dest.parent_path());
	fs::rename(src, dest);
}

void associative::VFS::Move::unapply()
{
	fs::rename(dest, src);
	if (destTemp)
		fs::rename(*destTemp, dest);
}

void associative::VFS::Move::finish()
{
	if (destTemp)
		fs::remove(*destTemp);
}

associative::VFS::Remove::Remove(const fs::path& target, const fs::path& backup)
: target(target), backup(backup)
{
}

associative::VFS::Remove::~Remove()
{
}

void associative::VFS::Remove::apply()
{
	fs::create_directories(backup.parent_path());
	if (fs::exists(target))
		fs::rename(target, backup);
}

void associative::VFS::Remove::unapply()
{
	if (fs::exists(backup))
		fs::rename(backup, target);
}

void associative::VFS::Remove::finish()
{
	if (fs::exists(backup))
		fs::remove(backup);
}

associative::VFS::Transaction::Transaction(VFS* const parent)
: parent(parent)
{
}

void associative::VFS::Transaction::move(const fs::path& src, const fs::path& dest)
{
	auto m = new Move(src, dest, fs::exists(dest) ? boost::make_optional(parent->tempPath / parent->getTempPath(dest.filename().string())) : boost::none);
	operations.push_back(m);
	m->apply();
}

void associative::VFS::Transaction::remove(const fs::path& target)
{
	auto r = new Remove(target, parent->tempPath / parent->getTempPath(target.filename().string()));
	operations.push_back(r);
	r->apply();
}

void associative::VFS::Transaction::finish()
{
	forEach(operations, boost::mem_fun(&Operation::finish));
	forEach(operations, lambda::delete_ptr());
	parent->modified.clear();
	parent->transaction.reset(); // self-destruction, beep bopp
}

void associative::VFS::Transaction::rollback()
{
	forEach(operations, boost::mem_fun(&Operation::unapply));
}

associative::WeakPtr<associative::VFS::Transaction> associative::VFS::apply(Environment& env)
{
	if (!env.getSessionID())
		throw Exception("not in a session");
	
	transaction = boost::shared_ptr<Transaction>(new Transaction(this));
	
	auto& conn = env.getConnection();
	auto query = conn.prepareQuery(
		"select journal.id, journal.operation, journal.target, file.uuid, blob.name from journal "
		"inner join `blob` on blob.id = journal.relation_id "
		"inner join file on file.id = blob.file_id "
		"where journal.session_id = ? and journal.relation = ? and journal.operation in (?, ?) and journal.executed = 0 "
		"order by journal.id asc",
	std::string("vfs.journal.select"));
	
	auto result = query->execute(convertAll(*env.getSessionID(), Connection::Relation::Blob, Blob::Operation::Store, Blob::Operation::Remove));
	
	auto stmt = conn.prepareStatement("update journal set executed = 1 where id = ?");
	
	for (auto iter = result.rows.begin(); iter != result.rows.end(); ++iter)
	{
		auto path = blobPath / (*iter)[3] / (*iter)[4];
		if ((*iter)[1] == toString(Blob::Operation::Store))
			transaction->move(tempPath / (*iter)[2], path);
		else if ((*iter)[1] == toString(Blob::Operation::Remove))
			transaction->remove(path);
		stmt->execute(convertAll((*iter)[0]));
	}

	return transaction;
}

fs::path associative::VFS::getTempPath(const std::string& seed)
{
	auto handle = process->getFileLock()->timedLockOrThrow();
	
	fs::path prefix(root / "temp");
	fs::create_directory(prefix);
	for (int i = 0; i < std::numeric_limits<int>::max(); ++i)
	{
		fs::path candidate(seed + toString(i));
		if (!fs::exists(prefix / candidate))
			return candidate;
	}
	throw formatException(boost::format("VFS %1% couldn't allocate temporary file") % root.string());
}

fs::path associative::VFS::getBlobPath(Environment& env, Blob& blob, bool write)
{	
	Blob::Identifier pair(blob.getFile().uuid, blob.name);
	if (containsKey(modified, pair))
	{
		// we don't care about writing here, because it's already a new destination
		return tempPath / modified[pair];
	}
	else
	{
		auto& conn = env.getConnection();
		
		auto path = fs::path(toString(blob.getFile().uuid)) / blob.name;
		bool exists = fs::exists(blobPath / path);
		if (!exists || write)
		{
			if (!env.getSessionID())
				throw Exception("not in a session");
			
			// if not exists, we know that we have a new file (even if caller doesn't intend to write)
			// if caller intends to write, we have to copy the old contents
			
			// in any case, create a temporary storage first
			auto temp = getTempPath(boost::lexical_cast<std::string>(blob.getFile().uuid));
			modified[pair] = temp;
			
			if (write && exists)
			{
				std::ifstream in((blobPath / path).c_str(), std::ios_base::binary);
				std::ofstream out((tempPath / temp).c_str(), std::ios_base::binary);
				copyStreams(in, out);
			}
			
			auto t = conn.transaction();
			auto stmt = conn.prepareStatement("insert into journal values (?, ?, ?, ?, ?, ?, 0)", std::string("vfs.journal.move"));
			
			// TODO temp instead of temp.c_str() somehow produces quotes
			stmt->execute(convertAll(conn.nextID("journal"), *env.getSessionID(), Connection::Relation::Blob, blob.getID(), Blob::Operation::Store, temp.c_str()));
			t->commit();
			
			return tempPath / temp;
		}
		else
		{
			return blobPath / path;
		}
	}
}

associative::VFS::VFS(const fs::path& root, const boost::shared_ptr<Process>& process, const boost::shared_ptr<Logger>& logger)
: root(root), tempPath(root / "temp"), blobPath(root / "blobs"), transaction(), process(process), logger(logger)
{
}
