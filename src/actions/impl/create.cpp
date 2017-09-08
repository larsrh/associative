#include <iostream>

#include <boost/uuid/uuid_io.hpp>

#include "../action.hpp"
#include "../../objects/file.hpp"

using namespace po;

namespace associative
{

	class CreateAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			return new options_description();
		}
		
	public:
		CreateAction()
		: Action("create")
		{
		}
		
		virtual int perform(const po::variables_map&, const std::vector<std::string>&, Environment& env)
		{
			std::cout << env.createFile()->uuid << std::endl;
			return 0;
		}
	};

}

COMMANDLINE_DEF(Create);
