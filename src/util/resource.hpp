#ifndef ASSOCIATIVE_RESOURCE_HPP
#define ASSOCIATIVE_RESOURCE_HPP

#include "util.hpp"
#include "config.hpp"

namespace associative
{
	
	class HandleBase
	{	
	public:
		virtual ~HandleBase();
	};
	
	template<typename H>
	class Resource
	{
	protected:
		typedef H Handle;
		
	public:
		virtual ~Resource()
		{
		}
		
		virtual boost::shared_ptr<H> use() = 0;
	};
	
	class DefaultResourceBase;
	
	class DefaultHandle : public HandleBase
	{
		friend class DefaultResourceBase;
	
	protected:
		DefaultResourceBase* parent;
		
		DefaultHandle(DefaultResourceBase* parent);
	
	public:
		virtual ~DefaultHandle();
	};
	
	class DefaultResourceBase
	{
		friend class DefaultHandle;
		
	protected:
		virtual void close() = 0;
		
		boost::shared_ptr<DefaultHandle> createHandle()
		{
			return boost::shared_ptr<DefaultHandle>(new DefaultHandle(this));
		}
	};
	
	template<typename R, typename H = DefaultHandle>
	class DefaultResource : public Resource<H>, public DefaultResourceBase
	{
	protected:
		typedef void (R::*Open)();
		typedef void (R::*Close)();
		typedef typename Resource<H>::Handle Handle;
		
		R* ref;
		Open _open;
		Close _close;
		
		DefaultResource(R* ref, Open open, Close close)
		: ref(ref), _open(open), _close(close)
		{
		}
		
		virtual void close()
		{
			(ref->*_close)();
		}
		
		virtual boost::shared_ptr<H> use()
		{
			(ref->*_open)();
			return createHandle();
		}
	
	public:
		virtual ~DefaultResource()
		{
		}
	};
	
	template<class L>
	class Lock : public DefaultResource<L>
	{
	public:
		typedef DefaultResource<L> Parent;
		typedef typename Parent::Handle Handle;
		
	protected:
		typedef bool (L::*TimedOpen)(const bpt::ptime&);
		typedef typename Parent::Open Open;
		typedef typename Parent::Close Close;
		
		TimedOpen timedOpen;
		
		Lock(L* lock, Open open, TimedOpen timedOpen, Close close)
		: Parent(lock, open, close), timedOpen(timedOpen)
		{
		}
	
	public:
		virtual ~Lock()
		{
		}
		
		virtual boost::shared_ptr<Handle> lock()
		{
			return Parent::use();
		}
		
		virtual boost::optional<boost::shared_ptr<Handle> > timedLock()
		{
			auto time = Configuration::maxLockTime();
			
			if (!time)
				return lock();
			else if ((Parent::ref->*timedOpen)(bpt::ptime() + bpt::seconds(*time)))
				return boost::make_optional(DefaultResourceBase::createHandle());
			else
				return boost::none;
		}
		
		virtual boost::shared_ptr<Handle> timedLockOrThrow()
		{
			auto handle = timedLock();
			if (!handle)
				throw Exception("lock timeout");
			return *handle;
		}
	};

}

#endif