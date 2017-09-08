#include "gen/config.hpp"
#ifdef ASSOCIATIVE_WITH_MYSQL

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <boost/algorithm/string.hpp>

#include "../../util/util.hpp"
#include "../../util/exception.hpp"
#include "../connection.hpp"

#define TRY_MYSQL(CODE) \
	try { CODE } \
	catch (const sql::SQLException& ex) { throw MySQLException(ex); }

namespace associative
{
	
	class MySQLException : public DBException
	{
	private:
		const sql::SQLException cause;
		
	public:
		MySQLException(const sql::SQLException& cause)
		: cause(cause)
		{
		}
		
		~MySQLException() throw()
		{
		}
		
		virtual const char* what() const throw()
		{
			return (boost::format("MySQL Exception\nCause: %1%\nError code: %2%\nSQL state: %3%") %
				cause.what() % cause.getErrorCode() % cause.getSQLState()
			).str().c_str();
		}
	};
	
	class MySQLConnection : public Connection
	{
	private:
		class PreparedBase
		{
		protected:
			sql::PreparedStatement* stmt;
			MySQLConnection* const outer;
			const std::string request;
			
			PreparedBase(MySQLConnection* const outer, const std::string& request)
			: outer(outer), request(request)
			{
				stmt = outer->conn->prepareStatement(request);
			}
			
			virtual ~PreparedBase()
			{
				stmt->clearParameters();
			}
			
			void bind(const std::vector<std::string>& parameters)
			{
				TRY_MYSQL(
					for (std::size_t i = 0; i < parameters.size(); ++i)
						stmt->setString(i + 1, parameters[i]);
				)
			}
		};
		
		class PreparedQuery : public PreparedBase, public associative::PreparedQuery
		{
		public:
			PreparedQuery(MySQLConnection* const outer, const std::string& request)
			: PreparedBase(outer, request)
			{
			}
			
			virtual ~PreparedQuery()
			{
			}
			
			virtual QueryResult execute(const std::vector<std::string>& parameters)
			{
				bind(parameters);
				return fetchResults(stmt->executeQuery());
			}
		};
		
		class PreparedStatement : public PreparedBase, public associative::PreparedStatement
		{
		public:
			PreparedStatement(MySQLConnection* const outer, const std::string& request)
			: PreparedBase(outer, request)
			{
			}
			
			virtual ~PreparedStatement()
			{
			}
			
			virtual uint64_t execute(const std::vector<std::string>& parameters)
			{
				bind(parameters);
				return stmt->execute();
			}
		};
		
		sql::Connection* conn;
		boost::shared_ptr<Logger> logger;
		
		static QueryResult fetchResults(sql::ResultSet* const rs)
		{
			TRY_MYSQL(
				sql::ResultSetMetaData* meta = rs->getMetaData();
				int colCount = meta->getColumnCount();
				std::vector<std::string> columnNames(colCount);
				for (int i = 0; i < colCount; ++i)
					columnNames[i] = meta->getColumnName(i + 1);
				
				std::list<std::vector<std::string> > rows;
				while (rs->next())
				{
					std::vector<std::string> row(colCount);
					for (int i = 0; i < colCount; ++i)
						row[i] = rs->getString(i + 1);
					rows.push_back(row);
				}
				
				delete rs;
				return QueryResult(columnNames, rows);
			)
		}
		
	protected:
		virtual associative::PreparedQuery* _prepareQuery(const std::string& query)
		{
			return new PreparedQuery(this, query);
		}
		
		virtual associative::PreparedStatement* _prepareStatement(const std::string& statement)
		{
			return new PreparedStatement(this, statement);
		}
		
		virtual void startTransaction()
		{
			TRY_MYSQL(
				conn->setAutoCommit(false);
				executeStatement("start transaction");
			)
		}
		
		virtual void endTransaction(bool commit = true)
		{
			TRY_MYSQL(
				if (commit)
					conn->commit();
				else
					conn->rollback();
				conn->setAutoCommit(true);
			)
		}
	
	public:
		MySQLConnection& operator=(MySQLConnection&) = delete;
		MySQLConnection(MySQLConnection&) = delete;
		
		MySQLConnection(const std::string& host, const std::string& database, const std::string& user, const std::string& password, const boost::shared_ptr<Logger>& logger)
		: logger(logger)
		{
			TRY_MYSQL(
				auto driver = sql::mysql::get_driver_instance();
				conn = driver->connect(host, user, password);
				conn->setAutoCommit(true);
				executeStatement("USE " + database);
			)
		}
		
		virtual ~MySQLConnection()
		{
			delete conn;
		}
		
		virtual QueryResult executeQuery(const std::string& query)
		{
			TRY_MYSQL(
				sql::Statement* stmt = conn->createStatement();
				return fetchResults(stmt->executeQuery(query));
			)
		}
		
		virtual uint64_t executeStatement(const std::string& statement)
		{
			TRY_MYSQL(
				sql::Statement* stmt = conn->createStatement();
				bool result = stmt->execute(statement);
				delete stmt;
				return result;
			)
		}
		
	};
	
	class MySQLProvider : public ConnectionProvider
	{
		PROVIDER_DECL;
		
	public:
		MySQLProvider()
		: ConnectionProvider("mysql")
		{
		}
		
		virtual Connection* createConnection(const std::string& dataSource, const boost::shared_ptr<Process>&, const boost::shared_ptr<Logger>& logger)
		{
			// Format: host:port:database:username:password
			
			std::vector<std::string> strs;
			boost::split(strs, dataSource, boost::is_any_of(":"));
			
			return new MySQLConnection(
				"tcp://" + strs.at(0) + ":" + strs.at(1),
				strs.at(2),
				strs.at(3),
				strs.at(4),
				logger
			);
		}
	};
	
}

PROVIDER_DEF(MySQL);

#else

// dummy
void _associative_MySQLProvider_init() {}

#endif