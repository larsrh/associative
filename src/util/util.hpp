#ifndef ASSOCIATIVE_UTIL_HPP
#define ASSOCIATIVE_UTIL_HPP

#ifndef BOOST_FILESYSTEM_VERSION
#define BOOST_FILESYSTEM_VERSION 3
#elif BOOST_FILESYSTEM_VERSION != 3
#error wrong BOOST_FILESYSTEM_VERSION
#endif

#include <unordered_map>
#include <string>
#include <iterator>
#include <algorithm>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "exception.hpp"

namespace fs = boost::filesystem3;
namespace po = boost::program_options;
namespace ba = boost::algorithm;
namespace lambda = boost::lambda;
namespace bl = boost::logic;
namespace bt = boost::tuples;
namespace bi = boost::interprocess;
namespace bpt = boost::posix_time;

namespace associative
{
	
	template<typename T>
	class WeakPtr
	{
	private:
		boost::weak_ptr<T> ptr;
		
	public:
		WeakPtr(const boost::weak_ptr<T>& ptr)
		: ptr(ptr)
		{
		}
		
		WeakPtr(const boost::shared_ptr<T>& ptr)
		: ptr(ptr)
		{
		}
		
		bool isDefined()
		{
			return !ptr.expired();
		}
		
		T& operator*()
		{
			return *ptr.lock();
		}
		
		T* operator->()
		{
			return get();
		}
		
		T* get()
		{
			return ptr.lock().get();
		}
	};
	
	template<typename T>
	class Buffer
	{
	private:
		std::unordered_map<std::string, T> buffer;
		
	public:
		template<typename Func>
		inline T getOrElse(const std::string& key, const Func& func)
		{
			if (containsKey(buffer, key))
				return buffer[key];
			else
				return buffer[key] = func();
		}
		
		inline boost::optional<T> option(const std::string& key)
		{
			if (containsKey(buffer, key))
				return boost::make_optional(buffer[key]);
			else
				return boost::none;
		}
		
		inline void set(const std::string& key, const T& value)
		{
			buffer[key] = value;
		}
		
		inline void reset(const std::string& key)
		{
			if (containsKey(buffer, key))
				buffer.erase(key);
		}
		
		std::set<std::string> getKeys()
		{
			std::set<std::string> keys;
			for (auto iter = buffer.begin(); iter != buffer.end(); ++iter)
				keys.insert(iter->first);
			return keys;
		}
		
		inline void clear()
		{
			buffer.clear();
		}
		
	};
	
	std::vector<std::string> parseArguments(const std::string& str);
	
	template<typename To>
	class LexicalCast
	{
	public:
		template<typename From>
		To operator()(const From& from) const
		{
			return boost::lexical_cast<To, From>(from);
		}
		
		To operator()(const To& to) const
		{
			return to;
		}
	};
	
	template<typename Coll, typename Key>
	bool containsKey(const Coll& coll, const Key& key)
	{
		return coll.find(key) != coll.end();
	}
	
	template<typename T, typename _CharT = char>
	std::basic_string<_CharT> toString(const T& value)
	{
		return boost::lexical_cast<std::basic_string<_CharT> >(value);
	}
	
	template<typename T, typename Iter, typename Func, typename Head, typename... Tail>
	inline void _mapAll(const Func& func, Iter output, const Head& head, const Tail&... tail)
	{
		*output = func(head);
		_mapAll<T>(func, ++output, tail...);
	}
	
	template<typename T, typename Iter, typename Func>
	inline void _mapAll(const Func&, Iter)
	{
		// intentionally left blank
	}
	
	template<typename T, typename Coll = std::vector<T>, typename Func, typename... Args>
	Coll mapAll(const Func& func, const Args&... args)
	{
		Coll coll;
		auto inserter = std::back_inserter(coll);
		_mapAll<T>(func, inserter, args...);
		return coll;
	}
	
	template<typename T = std::string, typename Coll = std::vector<T>, typename... Args>
	Coll convertAll(const Args&... args)
	{
		return mapAll<T, Coll>(LexicalCast<T>(), args...);
	}
	
	template<typename Coll, typename Func>
	void forEach(Coll& coll, const Func& func)
	{
		std::for_each(coll.begin(), coll.end(), func);
	}
	
	template<typename Coll, typename Func>
	void forEach(const Coll& coll, const Func& func)
	{
		std::for_each(coll.begin(), coll.end(), func);
	}
	
	template<typename Coll, typename OIter, typename Func>
	void filter(const Coll& coll, OIter copyTo, const Func& predicate)
	{
		for (auto iter = coll.begin(); iter != coll.end(); ++iter)
			if (predicate(*iter))
			{
				*copyTo = *iter;
				++copyTo;
			}
	}
	
	template<typename T>
	class Hash
	{
	public:
		size_t operator()(const T& t) const;
	};
	
	template<typename P>
	class Hash<boost::shared_ptr<P> >
	{
	public:
		std::size_t operator()(const boost::shared_ptr<P>& p) const
		{
			return std::hash<P*>()(p.get());
		}
	};
	
	std::string fnv1a(const std::string& str);
	
}

#endif