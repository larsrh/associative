#ifndef ASSOCIATIVE_PROCESS_HPP
#define ASSOCIATIVE_PROCESS_HPP

extern "C"
{
	#include <unistd.h>
}

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "../util/util.hpp"
#include "../util/log.hpp"
#include "../util/resource.hpp"

namespace associative
{

	class MemLock : public Lock<bi::interprocess_mutex>
	{
	public:
		MemLock(bi::interprocess_mutex* mutex);
	};
	
	class FileLock : public Lock<bi::file_lock>
	{
	public:
		FileLock(const fs::path& path);
		virtual ~FileLock();
	};
	
	class Process
	{
	private:
		const fs::path target;
		bi::managed_shared_memory* sharedMemory;
		MemLock* masterLock;
		FileLock* fileLock;
		const std::string digest;
		boost::shared_ptr<Logger> logger;
		
		MemLock* findMemLock(const std::string& name, bool create = true);
		
		static void _clearSharedMemory(const std::string& name);
		
	public:
		const int pid;
		
		Process(const fs::path& target, const std::string& dataSource, const boost::shared_ptr<Logger>& logger, bool clearShm = false);
		~Process();
		
		MemLock* getMemLock(const std::string& name, bool create = true);
		FileLock* getFileLock();
		
		static void clearSharedMemory(const std::string& dataSource);
		
	};
	
}

#endif