#ifndef ASSOCIATIVE_BLOB_HPP
#define ASSOCIATIVE_BLOB_HPP

#include <string>
#include <vector>

#include "file.hpp"
#include "triple.hpp"
#include "prefix.hpp"
#include "type.hpp"

namespace associative
{
	
	class File;
	class Environment;
	class Triple;
	class TripleFilter;
	class Prefix;
	class Type;
	
	class Blob
	{
		friend class File;
		
	private:
		Environment& env;
		File& file;
		uint64_t id;
		uint64_t handleID;
		bool removed;
		std::list<Triple> newTriples;
		std::unordered_set<uint64_t> removedTriples;
		
		uint64_t ensureContentType();
		void createBlob();
		
		Blob(Environment& env, File& file, const std::string& name, const std::string& contentType, boost::optional<uint64_t> id);
		
		void addTriple(const Triple& triple);
		
	public:
		typedef std::pair<boost::uuids::uuid, std::string> Identifier;
		
		enum Operation
		{
			Add,
			Remove,
			Store
		};
		
		const std::string name;
		const std::string contentType;
		
		Blob(Blob&) = delete;
		Blob& operator=(Blob&) = delete;
		
		~Blob();
		
		uint64_t getID();
		const File& getFile() const;
		void remove();
		bool isRemoved();
		fs::path getPath(bool write = false);
		
		std::vector<Triple> getTriples(const TripleFilter& filter);
		
		Triple addTriple(
			const boost::shared_ptr<Prefix>& predicatePrefix, const std::string& predicate, 
			Blob& blobObject
		);
		
		Triple addTriple(
			const boost::shared_ptr<Prefix>& predicatePrefix, const std::string& predicate, 
			const boost::shared_ptr<Type>& objectType, const std::string& object
		);
	};
	
}

#endif