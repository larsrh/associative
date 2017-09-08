#include <iostream>

#include <boost/functional.hpp>

#include "../action.hpp"
#include "../../util/io.hpp"

using namespace po;

namespace associative
{
	
	class QueryDBAction : public Action
	{
		COMMANDLINE_DECL;
		
	private:
		static void performQuery(const std::string& query, Connection& conn)
		{
			auto result = conn.executeQuery(query);
			std::cout << collToString(result.columnNames, QueryResult::tableRowFormat()) << std::endl << std::endl;
			std::cout << collToString(result.rows, QueryResult::tableFormat()) << std::endl;
			std::cout << result.rows.size() << " row(s) fetched" << std::endl;
		}
		
		static void performStatement(const std::string& statement, Connection& conn)
		{
			std::cout << "Statement execution returned with status " << conn.executeStatement(statement) << std::endl;
		}
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("query-db options");
			desc->add_options()
				("query", "query database and print results")
				("statement", "execute a statement");
			return desc;
		}
		
	public:
		QueryDBAction()
		: Action("query-db")
		{
		}
		
		virtual int perform(const variables_map& vm, const std::vector<std::string>& parameters, Environment& env)
		{
			bool query = vm.count("query");
			bool statement = vm.count("statement");
			if (query == statement)
				return 1;
			
			if (query)
				doREPL(parameters, boost::bind2nd(boost::ptr_fun(&performQuery), env.getConnection()), "query> ");
			else
				doREPL(parameters, boost::bind2nd(boost::ptr_fun(&performStatement), env.getConnection()), "statement> ");
			
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(QueryDB);
