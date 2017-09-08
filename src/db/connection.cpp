#include <boost/lexical_cast.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>

#include "connection.hpp"
#include "../util/util.hpp"

#include "gen/db_impls.hpp"

associative::QueryResult::QueryResult(const std::vector<std::string>& columnNames, const std::list<std::vector<std::string> >& rows)
: columnNames(columnNames), rows(rows)
{
}

associative::PreparedQuery::~PreparedQuery()
{
}

associative::PreparedStatement::~PreparedStatement()
{
}

void associative::TransactionHandle::commit()
{
	end(true);
}

void associative::TransactionHandle::rollback()
{
	end(false);
}

void associative::TransactionHandle::end(bool commit)
{
	if (ended)
		throw DBException("transaction already ended");
	
	ended = true;
	conn->endTransaction(commit);
}

associative::TransactionHandle::TransactionHandle(associative::Connection* conn)
: ended(false), conn(conn)
{
	conn->startTransaction();
}

associative::TransactionHandle::~TransactionHandle()
{
	if (!ended)
		conn->endTransaction(false);
}

associative::Connection::Connection(bool buffer)
: buffer(buffer)
{
}

associative::Connection::~Connection()
{
}

boost::shared_ptr<associative::PreparedQuery> associative::Connection::prepareQuery(const std::string& query, const boost::optional<std::string>& key)
{
	auto func = lambda::bind(lambda::constructor<boost::shared_ptr<PreparedQuery> >(), lambda::bind(&Connection::_prepareQuery, this, query));
	if (!buffer || !key) return func();
	return queryBuffer.getOrElse(*key, func);
}

boost::shared_ptr<associative::PreparedStatement> associative::Connection::prepareStatement(const std::string& statement, const boost::optional<std::string>& key)
{
	auto func = lambda::bind(lambda::constructor<boost::shared_ptr<PreparedStatement> >(), lambda::bind(&Connection::_prepareStatement, this, statement));
	if (!buffer || !key) return func();
	return statementBuffer.getOrElse(*key, func);
}

boost::shared_ptr<associative::TransactionHandle> associative::Connection::transaction()
{
	return boost::shared_ptr<TransactionHandle>(new TransactionHandle(this));
}

boost::shared_ptr<associative::TransactionHandle> associative::Connection::use()
{
	return transaction();
}

uint64_t associative::Connection::nextID(const std::string& table)
{
	auto result = prepareQuery(
		"select next_id from ids where table_name = ?", 
	std::string("connection.next_id.select"))->execute(convertAll(table));
	uint64_t id;
	if (!result.rows.size())
	{
		auto result = executeQuery("select max(id) from ids");
		uint64_t entryID;
		if (result.rows.empty())
		{
			entryID = 0;
		}
		else
		{
			auto& value = result.rows.front().at(0);
			entryID = value.empty() ? 0 : (boost::lexical_cast<uint64_t>(value) + 1);
		}
		
		id = 0;
		prepareStatement(
			"insert into ids values (?, ?, 1)", 
		std::string("connection.next_id.insert"))->execute(convertAll(entryID, table));
	}
	else
	{
		id = boost::lexical_cast<uint64_t>(result.rows.front().at(0));
		prepareStatement(
			"update ids set next_id = next_id + 1 where table_name = ?", 
		std::string("connection.next_id.update"))->execute(convertAll(table));
	}
	return id;
}

uint64_t associative::Connection::openHandle(int relation, uint64_t id, uint64_t sessionID)
{
	auto handleID = nextID("handle");
	auto stmt = prepareStatement("insert into handle values (?, ?, ?, ?)", std::string("connection.handle.open"));
	stmt->execute(convertAll(handleID, relation, id, sessionID));
	return handleID;
}

void associative::Connection::closeHandle(uint64_t handleID)
{
	auto stmt = prepareStatement("delete from handle where id = ?", std::string("connection.handle.close"));
	stmt->execute(convertAll(handleID));
}

associative::ModuleManager<associative::ConnectionProvider>& associative::ConnectionProvider::manager()
{
	static auto manager = new ModuleManager<ConnectionProvider>();
	return *manager;
}

associative::ConnectionProvider::ConnectionProvider(const std::string& name)
: Module(name)
{
}

associative::Connection* associative::ConnectionProvider::dispatch(const std::string& dataSource, const boost::shared_ptr<Process>& process, const boost::shared_ptr<Logger>& logger)
{
	ASSOCIATIVE_DB_INIT;
	
	auto pos = dataSource.find_first_of(':');
	if (pos == std::string::npos)
		throw formatException(boost::format("%1% is not a valid data source") % dataSource);
	
	auto provider = dataSource.substr(0, pos);
	if (!containsKey(manager().modules(), provider))
		throw formatException(boost::format("%1% is not a valid connection provider") % provider);
	
	return manager().modules()[provider]->createConnection(dataSource.substr(pos + 1), process, logger);
}

associative::DBException::DBException()
: Exception()
{
}

associative::DBException::DBException(const std::string& message)
: Exception(message)
{
}

associative::DBException::~DBException() throw()
{
}
