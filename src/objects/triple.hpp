#ifndef ASSOCIATIVE_TRIPLE_HPP
#define ASSOCIATIVE_TRIPLE_HPP

#include "blob.hpp"
#include "prefix.hpp"
#include "type.hpp"

namespace associative
{
	
	class Blob;
	class Environment;
	class Type;
	class Prefix;
	
	class Triple
	{
		friend class Blob;
		
	private:
		const Blob* const blob;
		
		Triple(
			const uint64_t id, const Blob* blob,
			const boost::shared_ptr<Prefix>& predicatePrefix, const std::string& predicate,
			const boost::shared_ptr<Type>& objectType, const std::string& object
		);
		
	public:
		const uint64_t id;
		const boost::shared_ptr<Prefix> predicatePrefix;
		const std::string predicate;
		const boost::shared_ptr<Type> objectType;
		const std::string object;
		
		enum Operation
		{
			Add,
			Remove
		};
		
		std::string toSimpleString() const;
		std::string toVerboseString() const;
		std::string toString(bool verbose) const;
	};
	
	class TripleFilter
	{
		// TODO implement
	};
	
};

#endif