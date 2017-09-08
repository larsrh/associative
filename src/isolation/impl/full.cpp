#include "../isolation.hpp"

namespace associative
{

	class FullIsolation : public IsolationLevel
	{
		ISOLEVEL_DECL;
		
	public:
		FullIsolation()
		: IsolationLevel("full")
		{
		}
		
		virtual bool isIsolated(const boost::shared_ptr<Connection>& conn, uint64_t sessionID) const
		{
			// no other session alive
			auto query = conn->prepareQuery("select * from session where id != ?", std::string("isolation.full"));
			return !query->execute(convertAll(sessionID)).rows.size();
		}
	};

}

ISOLEVEL_DEF(Full);
