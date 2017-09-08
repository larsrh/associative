#ifndef ASSOCIATIVE_ENVIRONMENT_HPP
#define ASSOCIATIVE_ENVIRONMENT_HPP

#include <boost/uuid/uuid.hpp>

#include "vfs.hpp"
#include "process.hpp"
#include "../isolation/isolation.hpp"
#include "../db/connection.hpp"
#include "../objects/file.hpp"
#include "../util/log.hpp"

namespace associative
{
	
	class Connection;
	class VFS;
	
	class Environment
	{
	private:
		boost::optional<uint64_t> id;
		Buffer<boost::shared_ptr<File> > buffer;
		boost::shared_ptr<Process> process;
		boost::shared_ptr<VFS> vfs;
		boost::shared_ptr<Connection> conn;
		boost::shared_ptr<Logger> logger;
		
	public:
		Environment(const boost::shared_ptr<Process>& process, const boost::shared_ptr<VFS>& vfs, const boost::shared_ptr<Connection>& conn, const boost::shared_ptr<Logger>& logger);
		Environment(Environment& env) = delete;
		Environment& operator=(Environment& env) = delete;
		virtual ~Environment();
		
		VFS& getVFS();
		Connection& getConnection();
		
		boost::optional<uint64_t> getSessionID();
		
		void cleanSessions(bool forceRollback = false);
		
		void startSession();
		void commitSession(const IsolationLevel& level);
		void rollbackSession();
		
		WeakPtr<File> createFile();
		WeakPtr<File> getFile(const std::string& uuid);
		
	};
	
	class CommitException : public Exception
	{
	public:
		enum Reason
		{
			Invalidated,
			ConflictingHandles
		};
		
		const Reason reason;
		
		CommitException(const std::string& message, const Reason& reason);
		~CommitException() throw();
	};
	
	
}

#endif