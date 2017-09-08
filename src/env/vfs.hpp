#ifndef ASSOCIATIVE_VFS_HPP
#define ASSOCIATIVE_VFS_HPP

#include <deque>

#include "environment.hpp"
#include "process.hpp"
#include "../util/util.hpp"
#include "../objects/blob.hpp"
#include "../db/connection.hpp"

namespace associative
{
	
	class Blob;
	
	class VFS
	{
		friend class Environment;
		friend class Blob;
		
	public:
		class Operation
		{
		public:
			virtual ~Operation();
			
			virtual void apply() = 0;
			virtual void unapply() = 0;
			virtual void finish() = 0;
		};
		
		class Move : public Operation
		{
		public:
			const fs::path src;
			const fs::path dest;
			const boost::optional<fs::path> destTemp;
			
			Move(const fs::path& src, const fs::path& dest, const boost::optional<fs::path>& destTemp);
			
			virtual ~Move();
			
			virtual void apply();
			virtual void unapply();
			virtual void finish();
		};
		
		class Remove : public Operation
		{
		public:
			const fs::path target;
			const fs::path backup;
			
			Remove(const fs::path& target, const fs::path& backup);
			
			virtual ~Remove();
			
			virtual void apply();
			virtual void unapply();
			virtual void finish();
		};
		
		class Transaction
		{
			friend class VFS;
			
		private:
			VFS* const parent;
			std::deque<Operation*> operations;
			
			Transaction(VFS* const parent);
			
			void move(const fs::path& src, const fs::path& dest);
			void remove(const fs::path& target);
			
		public:
			void finish();
			void rollback();
		};
		
	private:
		const fs::path root;
		const fs::path tempPath;
		const fs::path blobPath;
		boost::shared_ptr<Transaction> transaction;
		boost::shared_ptr<Process> process;
		boost::shared_ptr<Logger> logger;
		std::map<Blob::Identifier, fs::path> modified;
		
		WeakPtr<Transaction> apply(Environment& env);
		
		fs::path getTempPath(const std::string& seed);
		fs::path getBlobPath(Environment& env, Blob& blob, bool write = false);
		
	public:
		VFS(const fs::path& root, const boost::shared_ptr<Process>& process, const boost::shared_ptr<Logger>& logger);
	};
	
}

#endif