#ifndef ASSOCIATIVE_PREFIX_HPP
#define ASSOCIATIVE_PREFIX_HPP

#include <string>

#include "blob.hpp"
#include "../db/connection.hpp"
#include "../util/util.hpp"

namespace associative
{
	
	class Blob;
	class Type;
	
	class Prefix
	{
		friend class Blob;
		friend class Type;
		
	private:
		Prefix(const uint64_t id, const std::string& name, const std::string& uri);
		
	public:
		Prefix(const Prefix& prefix);
		
		const uint64_t id;
		const std::string name;
		const std::string uri;
		
		static boost::shared_ptr<Prefix> get(Connection& conn, const std::string& name, const boost::optional<std::string>& uri = boost::none);
	};
	
}

#endif