#include "../isolation.hpp"

namespace associative
{

	class UnsafeIsolation : public IsolationLevel
	{
		ISOLEVEL_DECL;
		
	public:
		UnsafeIsolation()
		: IsolationLevel("unsafe")
		{
		}
		
		virtual bool isIsolated(const boost::shared_ptr<Connection>&, uint64_t) const
		{
			return true;
		}
	};

}

ISOLEVEL_DEF(Unsafe);
