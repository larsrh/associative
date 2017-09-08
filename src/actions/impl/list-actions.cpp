#include "../action.hpp"

using namespace po;

namespace associative
{

	class ListActionsAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("list-actions options");
			return desc;
		}
		
	public:
		ListActionsAction()
		: Action("list-actions")
		{
		}
		
		virtual int perform(const po::variables_map&, const std::vector<std::string>&, Environment&)
		{
			std::cout << "Available actions: " << std::endl;
			auto& map = Action::manager().modules();
			for (auto iter = map.begin(); iter != map.end(); ++iter)
				std::cout << "*  " << iter->first << std::endl;
			
			return 0;
		}
	};

}

COMMANDLINE_DEF(ListActions);
