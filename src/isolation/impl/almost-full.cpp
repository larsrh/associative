#include "../isolation.hpp"

namespace associative
{

	class AlmostFullIsolation : public IsolationLevel
	{
		ISOLEVEL_DECL;
		
	public:
		AlmostFullIsolation()
		: IsolationLevel("almost-full")
		{
		}
		
		virtual bool isIsolated(const boost::shared_ptr<Connection>& conn, uint64_t sessionID) const
		{
			// other sessions may be alive, but they must not have any open handles
			auto query = conn->prepareQuery("select * from handle where session_id != ?", std::string("isolation.almost-full"));
			return !query->execute(convertAll(sessionID)).rows.size();
		}
	};

}

ISOLEVEL_DEF(AlmostFull);
