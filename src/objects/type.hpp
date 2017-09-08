#ifndef ASSOCIATIVE_TYPE_HPP
#define ASSOCIATIVE_TYPE_HPP

#include <boost/optional.hpp>

#include "prefix.hpp"
#include "blob.hpp"

namespace associative
{
	
	class Prefix;
	
	class Type
	{
		friend class Blob;
		friend class Prefix;
		
	private:
		Type(const uint64_t id, const std::string& name, const boost::shared_ptr<Prefix>& prefix);
		
	public:
		Type(const Type& type);
		
		const uint64_t id;
		const std::string name;
		const boost::shared_ptr<Prefix> prefix;
		
		static boost::shared_ptr<Type> get(Connection& conn, const std::string& name, const boost::shared_ptr<Prefix>& prefix);
		static boost::shared_ptr<Type> getBlobType(Connection& conn);
	};
	
}

#endif