#include <algorithm>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/bind.hpp>

#include "file.hpp"
#include "../util/util.hpp"
#include "../env/environment.hpp"

boost::uuids::uuid associative::File::getRandomUUID()
{
	static auto gen = boost::uuids::random_generator();
	return gen();
}

void associative::File::ensureFile(bool create)
{
	auto& conn = env.getConnection();
	auto query = conn.prepareQuery("select id, visible from file where uuid = ?", std::string("file.select"));
	auto result = query->execute(convertAll(uuid));
	if (result.rows.size())
	{
		auto row = result.rows.front();
		id = boost::lexical_cast<uint64_t>(row.at(0));
		if (!boost::lexical_cast<bool>(row.at(1)))
			throw formatException(boost::format("some other process is creating the file with uuid %1%") % uuid);
	}
	else if (create)
	{
		if (!env.getSessionID())
			throw Exception("not in a session");
		
		auto stmt = conn.prepareStatement("insert into file values (?, ?, 0, 0)", std::string("file.add"));
		id = conn.nextID("file");
		stmt->execute(convertAll(id, uuid));
		
		stmt = conn.prepareStatement("insert into journal values (?, ?, ?, ?, ?, null, 0)", std::string("file.journal.add"));
		stmt->execute(convertAll(conn.nextID("journal"), *env.getSessionID(), Connection::Relation::File, id, Operation::Add));
	}
	else
	{
		throw formatException(boost::format("file with uuid %1% not found") % uuid);
	}
}

associative::File::File(Environment& env, const boost::optional<boost::uuids::uuid>& uuid)
: env(env), uuid(uuid ? *uuid : getRandomUUID())
{
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	ensureFile(!uuid);
	handleID = conn.openHandle(Connection::Relation::File, id, *env.getSessionID());
	t->commit();
}

associative::File::~File()
{
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	conn.closeHandle(handleID);
	t->commit();
}

uint64_t associative::File::getID()
{
	return id;
}

std::set<std::string> associative::File::getBlobNames()
{
	auto& conn = env.getConnection();
	auto query = conn.prepareQuery("select name from `blob` where file_id = ? and visible = 1", std::string("file.blobs.list"));
	
	auto result = query->execute(convertAll(id));
	
	std::set<std::string> names;
	for (auto iter = result.rows.begin(); iter != result.rows.end(); ++iter)
		names.insert(iter->at(0));
	
	auto keys = buffer.getKeys();
	
	// Blobs removed in this session do not appear here
	filter(keys, std::inserter(names, names.begin()), [&buffer](std::string key) { return !(*buffer.option(key))->isRemoved(); });
	return names;
}

bool associative::File::hasBlob(const std::string& name)
{
	return containsKey(getBlobNames(), name);
}

associative::WeakPtr<associative::Blob> associative::File::getBlob(const std::string& name)
{
	auto opt = buffer.option(name);
	if (opt)
	{
		if ((*opt)->isRemoved())
			throw formatException(boost::format("blob with name %1% from file with uuid %2% has been removed") % name % uuid);
		return *opt;
	}
	
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	auto query = conn.prepareQuery(
		"select blob.id, content_type.mime "
		"from `blob` inner join content_type "
			"on blob.content_type_id = content_type.id "
		"where blob.file_id = ? and blob.name = ? and blob.visible = 1",
	std::string("file.blobs.get"));
	auto result = query->execute(convertAll(id, name));
	
	if (result.rows.empty())
		throw formatException(boost::format("file with uuid %1% has no blob named %2%") % uuid % name);
	
	auto row = result.rows.front();
	boost::shared_ptr<Blob> ptr(new Blob(env, *this, name, row[1], boost::lexical_cast<uint64_t>(row[0])));
	t->commit();
	buffer.set(name, ptr);
	return ptr;
}

associative::WeakPtr<associative::Blob> associative::File::addBlob(const std::string& name, const std::string& contentType)
{
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	auto query = conn.prepareQuery("select id from `blob` where file_id = ? and name = ?", std::string("file.blobs.check-add"));
	auto result = query->execute(convertAll(id, name));
	if (!result.rows.empty())
		throw formatException(boost::format("blob with name %1% already existing") % name);
	
	boost::shared_ptr<Blob> ptr(new Blob(env, *this, name, contentType, boost::none));
	t->commit();
	buffer.set(name, ptr);
	return ptr;
}

void associative::File::removeBlob(const std::string& name)
{
	getBlob(name)->remove();
}

void associative::File::removeFile()
{
	forEach(getBlobNames(), lambda::bind(&File::removeBlob, this, lambda::_1));
}
