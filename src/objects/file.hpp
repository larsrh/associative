#ifndef ASSOCIATIVE_FILE_HPP
#define ASSOCIATIVE_FILE_HPP

#include <set>
#include <unordered_set>

#include <boost/uuid/uuid.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include "blob.hpp"
#include "../util/util.hpp"
#include "../db/connection.hpp"

namespace associative
{

	class Blob;
	class Environment;
	
	class File
	{
		friend class Environment;
		
	private:
		uint64_t id;
		uint64_t handleID;
		Environment& env;
		Buffer<boost::shared_ptr<Blob> > buffer;
		
		static boost::uuids::uuid getRandomUUID();
		
		void ensureFile(bool create);
		
		File(Environment& env, const boost::optional<boost::uuids::uuid>& uuid = boost::none);
		
	public:
		enum Operation
		{
			Add
		};
		
		const boost::uuids::uuid uuid;
		
		~File();
		File(File&) = delete;
		File& operator=(File&) = delete;
		
		uint64_t getID();
		std::set<std::string> getBlobNames();
		bool hasBlob(const std::string& name);
		WeakPtr<Blob> getBlob(const std::string& name);
		WeakPtr<Blob> addBlob(const std::string& name, const std::string& contentType);
		void removeBlob(const std::string& name);
		void removeFile();
	};
	
}

#endif