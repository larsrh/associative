#include "type.hpp"

associative::Type::Type(const uint64_t id, const std::string& name, const boost::shared_ptr<associative::Prefix>& prefix)
: id(id), name(name), prefix(prefix)
{
}

associative::Type::Type(const associative::Type& type)
: id(type.id), name(type.name), prefix(type.prefix)
{
}

boost::shared_ptr<associative::Type> associative::Type::get(associative::Connection& conn, const std::string& name, const boost::shared_ptr<associative::Prefix>& prefix)
{
	auto query = conn.prepareQuery("select id from type where name = ? and prefix_id = ?", std::string("type.select"));
	auto result = query->execute(convertAll(name, prefix->id));
	uint64_t id;
	
	if (result.rows.size())
	{
		id = boost::lexical_cast<uint64_t>(result.rows.front().at(0));
	}
	else
	{
		id = conn.nextID("type");
		conn.prepareStatement(
			"insert into type values (?, ?, ?)",
		std::string("type.add"))->execute(convertAll(id, prefix->id, name));
	}
	
	return boost::shared_ptr<Type>(new Type(id, name, prefix));
}

boost::shared_ptr<associative::Type> associative::Type::getBlobType(associative::Connection& conn)
{
	auto query = conn.prepareQuery(
		"select prefix.id, prefix.name, prefix.uri, type.id, type.name "
		"from prefix inner join type on type.prefix_id = prefix.id "
		"where type.id = ? and prefix.id = ?",
	std::string("type.fromid"));
	auto result = query->execute(convertAll(ASSOCIATIVE_SYS_BLOB_TYPE, ASSOCIATIVE_SYS_PREFIX));
	
	if (!result.rows.size())
		throw Exception("internal error: blob type or system prefix not found");
	
	auto firstRow = result.rows.front();
	boost::shared_ptr<Prefix> prefix(new Prefix(boost::lexical_cast<uint64_t>(firstRow.at(0)), firstRow.at(1), firstRow.at(2)));
	return boost::shared_ptr<Type>(new Type(boost::lexical_cast<uint64_t>(firstRow.at(3)), firstRow.at(4), prefix));
}
