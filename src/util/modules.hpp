#ifndef ASSOCIATIVE_MODULES_HPP
#define ASSOCIATIVE_MODULES_HPP

#include <map>

#include <boost/optional.hpp>

#include "util.hpp"

#define MODULE_DECL \
	public: \
		static boost::none_t dummy

#define MODULE_DEF(NAME, SUFFIX, PARENT) \
	boost::none_t associative::NAME##SUFFIX::dummy = associative::PARENT::manager()._register<NAME##SUFFIX>(); \
	void _associative_##NAME##SUFFIX##_init() { associative::ignore(associative::NAME##SUFFIX::dummy); }

namespace associative
{
	
	template<typename Parent>
	class ModuleFactoryBase
	{
	public:
		virtual Parent* create() = 0;
	};
	
	template<typename Parent, typename A>
	class ModuleFactory : public ModuleFactoryBase<Parent>
	{
	public:
		virtual Parent* create()
		{
			return static_cast<Parent*>(new A());
		}
	};
	
	template<typename Parent>
	class ModuleManager
	{
	private:
		std::list<ModuleFactoryBase<Parent>*>* _factories;
		std::map<std::string, Parent*>* _modules;
		
		inline std::list<ModuleFactoryBase<Parent>*>& factories()
		{
			if (!_factories)
				_factories = new std::list<ModuleFactoryBase<Parent>*>();
			return *_factories;
		}
		
	public:
		ModuleManager()
		: _factories(0), _modules(0)
		{
		}
		
		template<typename A>
		boost::none_t _register()
		{
			factories().push_back(new ModuleFactory<Parent, A>());
			return boost::none;
		}
		
		std::map<std::string, Parent*>& modules()
		{
			if (!_modules)
				_modules = new std::map<std::string, Parent*>();
			if (factories().empty())
				return *_modules;
	
			for (auto iter = factories().begin(); iter != factories().end(); ++iter) {
				auto module = (*iter)->create();
				const std::string& name = module->name;
				if (containsKey(*_modules, name))
					throw formatException(boost::format("module with name %1% already existing") % name);
				(*_modules)[name] = module;
				delete *iter;
			}
			factories().clear();
			return *_modules;
		}
	};
	
	class Module
	{
		template<typename> friend class ModuleManager;
		
	private:
		const std::string name;
		
	protected:
		Module(const std::string& name);
	};
	
	template<typename T>
	void ignore(T) {}
	
}

#endif