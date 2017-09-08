#include "gen/config.hpp"
#ifdef ASSOCIATIVE_WITH_SQLITE

#include <sqlite3.h>

#include "../../util/util.hpp"
#include "../connection.hpp"

namespace associative
{
	
	class SQLite3Exception : public DBException
	{
	private:
		const int errorCode;
		const std::string errorMsg;
		
	public:
		SQLite3Exception(const std::string& message, const int errorCode, const std::string& errorMsg)
		: DBException(message), errorCode(errorCode), errorMsg(errorMsg)
		{
		}
		
		~SQLite3Exception() throw()
		{
		}
		
		virtual const char* what() const throw()
		{
			return (boost::format("%1%\nError Code: %2%\nError Message: %3%") % DBException::what() % errorCode % errorMsg).str().c_str();
		}

	};
	
	class SQLite3Connection : public Connection
	{
	private:
		class PreparedBase
		{
		protected:
			sqlite3_stmt* stmt;
			SQLite3Connection* const outer;
			const std::string request;
			
			PreparedBase(SQLite3Connection* const outer, const std::string& request)
			: outer(outer), request(request)
			{
				auto code = sqlite3_prepare_v2(outer->conn, request.c_str(), -1, &stmt, 0);
				if (code != SQLITE_OK)
					outer->throwException(boost::format("couldn't prepare request %1%") % request, code);
			}
			
			virtual ~PreparedBase()
			{
				sqlite3_finalize(stmt);
			}
			
			void bind(const std::vector<std::string>& parameters)
			{
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
				int i = 1;
				for (auto iter = parameters.begin(); iter != parameters.end(); ++iter, ++i)
					sqlite3_bind_text(stmt, i, iter->c_str(), -1, 0);
			}
			
		};
		
		class PreparedQuery : public PreparedBase, public associative::PreparedQuery
		{
		public:
			PreparedQuery(SQLite3Connection* const outer, const std::string& request)
			: PreparedBase(outer, request)
			{
			}
			
			virtual ~PreparedQuery()
			{
			}
			
			virtual QueryResult execute(const std::vector<std::string>& parameters)
			{
				bind(parameters);
				return outer->fetchResults(stmt);
			}
			
		};
		
		class PreparedStatement : public PreparedBase, public associative::PreparedStatement
		{
		public:
			PreparedStatement(SQLite3Connection* const outer, const std::string& request)
			: PreparedBase(outer, request)
			{
			}
			
			virtual ~PreparedStatement()
			{
			}
			
			virtual uint64_t execute(const std::vector<std::string>& parameters)
			{
				bind(parameters);
				auto code = sqlite3_step(stmt);
				if (code != SQLITE_DONE)
					outer->throwException(boost::format("couldn't execute statement %1%") % request, code);
				return 0;
			}
			
		};
		
		const fs::path file;
		sqlite3* conn;
		boost::shared_ptr<Logger> logger;
		
		void throwException(const boost::format& format, const int errorCode)
		{
			throw formatException<SQLite3Exception>(format, errorCode, sqlite3_errmsg(conn));
		}

		QueryResult fetchResults(sqlite3_stmt* const stmt)
		{
			int colCount = sqlite3_column_count(stmt);
			std::vector<std::string> columnNames(colCount);
			for (int i = 0; i < colCount; ++i)
				columnNames[i] = sqlite3_column_name(stmt, i);
			
			std::list<std::vector<std::string> > rows;
			while (true)
			{
				auto code = sqlite3_step(stmt);
				if (code == SQLITE_ROW)
				{
					std::vector<std::string> row(colCount);
					for (int i = 0; i < colCount; ++i)
					{
						// TODO fix
						auto entry = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
						row[i] = entry ? entry : "";
					}
					rows.push_back(row);
				}
				else if (code == SQLITE_DONE)
				{
					break;
				}
				else
				{
					throwException(boost::format("error fetching row"), code);
				}
			}
			
			return QueryResult(columnNames, rows);
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
			executeStatement("pragma autocommit=false");
			executeStatement("begin transaction");
		}

		virtual void endTransaction(bool commit = true)
		{
			executeStatement(commit ? "commit" : "rollback");
			executeStatement("pragma autocommit=true");
		}
	
	public:
		SQLite3Connection& operator=(SQLite3Connection&) = delete;
		SQLite3Connection(SQLite3Connection&) = delete;
		
		SQLite3Connection(const fs::path& file, const boost::shared_ptr<Logger>& logger)
		: file(file), conn(0), logger(logger)
		{
			if (!fs::exists(file))
				throw formatException<DBException>(boost::format("file %1% doesn't exist") % file);
			if (sqlite3_open(file.c_str(), &conn) != SQLITE_OK)
				throwException(boost::format("couldn't open connection to %1%") % file, SQLITE_OK);
		}
		
		virtual ~SQLite3Connection()
		{
			if (!conn) return;
			sqlite3_close(conn);
		}
		
		virtual QueryResult executeQuery(const std::string& query)
		{
			auto prepared = prepareQuery(query);
			QueryResult result = prepared->execute(std::vector<std::string>());
			return result;
		}
		
		virtual uint64_t executeStatement(const std::string& statement)
		{
			auto code = sqlite3_exec(conn, statement.c_str(), 0, 0, 0);
			if (code != SQLITE_OK)
				throwException(boost::format("couldn't execute statement %1%") % statement, code);
			return code;
		}
		
	};
	
	class SQLite3Provider : public ConnectionProvider
	{
		PROVIDER_DECL;
		
	public:
		SQLite3Provider()
		: ConnectionProvider("sqlite3")
		{
		}
		
		virtual Connection* createConnection(const std::string& dataSource, const boost::shared_ptr<Process>&, const boost::shared_ptr<Logger>& logger)
		{
			return new SQLite3Connection(dataSource, logger);
		}
		
	};
	
}

PROVIDER_DEF(SQLite3);

#else

// dummy
void _associative_SQLite3Provider_init() {}

#endif
