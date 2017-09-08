#include <fstream>
#include <sstream>

#include "process.hpp"
#include "../util/exception.hpp"
#include "../util/io.hpp"
#include "../util/config.hpp"

associative::MemLock::MemLock(bi::interprocess_mutex* mutex)
: Lock(mutex, &bi::interprocess_mutex::lock, &bi::interprocess_mutex::timed_lock, &bi::interprocess_mutex::unlock)
{
}

associative::FileLock::FileLock(const fs::path& path)
: Lock(new bi::file_lock(path.c_str()), &bi::file_lock::lock, &bi::file_lock::timed_lock, &bi::file_lock::unlock)
{
}

associative::FileLock::~FileLock()
{
	delete ref;
}

void associative::Process::_clearSharedMemory(const std::string& name)
{
	bi::shared_memory_object::remove(name.c_str());
}

associative::Process::Process(const fs::path& target, const std::string& dataSource, const boost::shared_ptr<Logger>& logger, bool clearShm)
: target(target), digest(fnv1a(dataSource)), logger(logger), pid(getpid())
{
	auto lockFile = target / "lock";
	createEmptyFile(lockFile);
	fileLock = new FileLock(lockFile);
	
	if (clearShm) _clearSharedMemory(digest);
	
	sharedMemory = new bi::managed_shared_memory(bi::open_or_create, digest.c_str(), 65536);
	masterLock = findMemLock("master");
}

associative::Process::~Process()
{
	delete masterLock;
	delete sharedMemory;
	delete fileLock;
}

void associative::Process::clearSharedMemory(const std::string& dataSource)
{
	_clearSharedMemory(fnv1a(dataSource));
}

associative::MemLock* associative::Process::findMemLock(const std::string& name, bool create)
{
	// TODO investigate whether a Buffer would speed up things
	// This code operates under the assumption that shared_memory is a thread-safe thing
	// TODO there may be a race condition if two processes start at the same time
	
	auto ptr = sharedMemory->find<bi::interprocess_mutex>(name.c_str());
	bi::interprocess_mutex* mutex;
	if (ptr.first)
		mutex = ptr.first;
	else if (create)
		mutex = sharedMemory->construct<bi::interprocess_mutex>(name.c_str())();
	else
		throw formatException(boost::format("lock with the name %1% does not exist") % name);
	
	return new MemLock(mutex);
}

associative::MemLock* associative::Process::getMemLock(const std::string& name, bool create)
{
	if (name == "master")
		throw Exception("master lock is private");
	
	// We are leaking MemLock objects on purpose here. shared_ptrs are not the
	// best idea, because after calls like
	//   auto handle = getMemLock("foo")->lock();
	// the destructor of 'handle' crashes because the original 'parent'
	// (= MemLock) object has already been destructed due to the shared_ptr.
	// Note that a private buffer of shared_ptrs would solve that problem, but
	// that introduces thread-safety issues. Also, leaking is not too bad as
	// those objects are really small.
	return findMemLock(name, create);
}

associative::FileLock* associative::Process::getFileLock()
{
	return fileLock;
}
