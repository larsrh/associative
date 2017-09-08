#include "prefix.hpp"

associative::Prefix::Prefix(const uint64_t id, const std::string& name, const std::string& uri)
: id(id), name(name), uri(uri)
{
}

associative::Prefix::Prefix(const associative::Prefix& prefix)
: id(prefix.id), name(prefix.name), uri(prefix.uri)
{
}

boost::shared_ptr<associative::Prefix> associative::Prefix::get(associative::Connection& conn, const std::string& name, const boost::optional<std::string>& uri)
{
	auto query = conn.prepareQuery("select id, name, uri from prefix where name = ?", std::string("prefix.select"));
	auto result = query->execute(convertAll(name));
	uint64_t id;
	std::string actualURI;
	
	if (result.rows.size())
	{
		auto& firstRow = result.rows.front();
		actualURI = firstRow.at(2);
		if (uri && *uri != actualURI)
			throw formatException(boost::format("the actual URI for prefix %1% is %2% instead of %3%") % name % actualURI % *uri);
		
		id = boost::lexical_cast<uint64_t>(firstRow.at(0));
	}
	else
	{
		if (!uri)
			throw formatException(boost::format("prefix %1% not existing and no URI specified") % name);
		
		id = conn.nextID("prefix");
		conn.prepareStatement(
			"insert into prefix values (?, ?, ?)",
		std::string("prefix.add"))->execute(convertAll(id, name, *uri));
		
		actualURI = *uri;
	}
	
	return boost::shared_ptr<Prefix>(new Prefix(id, name, actualURI));
}
