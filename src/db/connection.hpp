#ifndef ASSOCIATIVE_CONNECTION_HPP
#define ASSOCIATIVE_CONNECTION_HPP

#include "../env/process.hpp"
#include "../util/format.hpp"
#include "../util/modules.hpp"
#include "../util/log.hpp"

#define PROVIDER_DECL \
	MODULE_DECL

#define PROVIDER_DEF(NAME) \
	MODULE_DEF(NAME, Provider, ConnectionProvider)

// TODO where to put them?
#define ASSOCIATIVE_SYS_BLOB_TYPE 0
#define ASSOCIATIVE_SYS_PREFIX 0

namespace associative
{
	
	class QueryResult
	{
	public:
		QueryResult(const std::vector<std::string>& columnNames, const std::list<std::vector<std::string> >& rows);
		
		const std::vector<std::string> columnNames;
		const std::list<std::vector<std::string> > rows;
		
		template<typename _Coll = std::vector<std::string> >
		static SimpleCollFormat<_Coll> tableRowFormat()
		{
			return SimpleCollFormat<_Coll>("|", "|", "|");
		}
		
		template<typename _ElemColl = std::vector<std::string>, typename _Coll = std::list<_ElemColl> >
		static NestedCollFormat<char, _Coll, SimpleCollFormat<_ElemColl> > tableFormat()
		{
			return NestedCollFormat<char, _Coll, SimpleCollFormat<_ElemColl> >("", "", "\n", tableRowFormat<_ElemColl>());
		}
	};
	
	class PreparedQuery
	{
	public:
		virtual ~PreparedQuery();
		
		virtual QueryResult execute(const std::vector<std::string>& parameters) = 0;
	};
	
	class PreparedStatement
	{
	public:
		virtual ~PreparedStatement();
		
		virtual uint64_t execute(const std::vector<std::string>& parameters) = 0;
	};
	
	class Connection;
	
	class TransactionHandle : public HandleBase
	{
		friend class Connection;
	
	private:
		bool ended;
		Connection* conn;
		
	protected:
		TransactionHandle(Connection* conn);
	
	public:
		void commit();
		void rollback();
		void end(bool commit = true);
		
		virtual ~TransactionHandle();
	};
	
	class Connection : public Resource<TransactionHandle>
	{
		friend class TransactionHandle;
		
	private:
		Buffer<boost::shared_ptr<PreparedStatement> > statementBuffer;
		Buffer<boost::shared_ptr<PreparedQuery> > queryBuffer;
		
	protected:
		const bool buffer;
		
		Connection(bool buffer = true);
		
		virtual PreparedStatement* _prepareStatement(const std::string& statement) = 0;
		virtual PreparedQuery* _prepareQuery(const std::string& query) = 0;
		
		virtual void startTransaction() = 0;
		virtual void endTransaction(bool commit = true) = 0;
		
	public:
		enum Relation
		{
			File,
			Blob,
			Metadata
		};
		
		Connection& operator=(Connection&) = delete;
		Connection(Connection&) = delete;
		
		virtual ~Connection();
		
		virtual uint64_t executeStatement(const std::string& statement) = 0;
		virtual QueryResult executeQuery(const std::string& query) = 0;
		boost::shared_ptr<PreparedStatement> prepareStatement(const std::string& statement, const boost::optional<std::string>& key = boost::none);
		boost::shared_ptr<PreparedQuery> prepareQuery(const std::string& query, const boost::optional<std::string>& key = boost::none);
		
		boost::shared_ptr<TransactionHandle> use();
		boost::shared_ptr<TransactionHandle> transaction();
		
		uint64_t nextID(const std::string& table);
		uint64_t openHandle(int relation, uint64_t id, uint64_t sessionID);
		void closeHandle(uint64_t handleID);
	};
	
	class ConnectionProvider : public Module
	{
	protected:
		ConnectionProvider(const std::string& name);
		
		static ModuleManager<ConnectionProvider>& manager();
		
		virtual Connection* createConnection(const std::string& dataSource, const boost::shared_ptr<Process>& process, const boost::shared_ptr<Logger>& logger) = 0;
		
	public:
		static Connection* dispatch(const std::string& dataSource, const boost::shared_ptr<Process>& process, const boost::shared_ptr<Logger>& logger);
	};
	
	class DBException : public Exception
	{
	public:
		DBException();
		DBException(const std::string& message);
		~DBException() throw();
	};
	
}

#endif