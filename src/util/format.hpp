#ifndef ASSOCIATIVE_FORMAT_HPP
#define ASSOCIATIVE_FORMAT_HPP

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace associative
{

	template<typename _CharT>
	class CollFormatBase
	{
	public:
		typedef typename std::basic_string<_CharT> String;
		
		const String start, end, separator;
		
		CollFormatBase(const String& start, const String& end, const String& separator)
		: start(start), end(end), separator(separator)
		{
		}
	};
	
	template<typename _CharT, typename _Coll>
	class CollFormat : public CollFormatBase<_CharT>
	{
	public:
		typedef typename _Coll::const_iterator ConstIterator;
		typedef typename CollFormatBase<_CharT>::String String;
		
		CollFormat(const String& start, const String& end, const String& separator)
		: CollFormatBase<_CharT>(start, end, separator)
		{
		}
		
		virtual String toString(const ConstIterator&) const = 0;
	};
	
	template<typename _Coll>
	class SimpleCollFormat : public CollFormat<char, _Coll>
	{
	public:
		typedef typename _Coll::const_iterator ConstIterator;
		typedef typename CollFormatBase<char>::String String;
		
		SimpleCollFormat()
		: CollFormat<char, _Coll>("[", "]", ",")
		{
		}
		
		SimpleCollFormat(const String& start, const String& end, const String& separator)
		: CollFormat<char, _Coll>(start, end, separator)
		{
		}
		
		virtual String toString(const ConstIterator& iter) const
		{
			return boost::lexical_cast<String>(*iter);
		}
	};
	
	template<typename _Coll>
	class ConvertingCollFormat : public CollFormat<char, _Coll>
	{
	public:
		typedef typename _Coll::const_iterator ConstIterator;
		typedef typename CollFormatBase<char>::String String;
		typedef typename ConstIterator::value_type ElemType;
		typedef String (ElemType::*ToString)() const;
		
	private:
		const ToString& _toString;
		
	public:
		ConvertingCollFormat(const ToString& _toString)
		: CollFormat<char, _Coll>("[", "]", ","), _toString(_toString)
		{
		}
		
		ConvertingCollFormat(const String& start, const String& end, const String& separator, const ToString& _toString)
		: CollFormat<char, _Coll>(start, end, separator), _toString(_toString)
		{
		}
		
		virtual String toString(const ConstIterator& iter) const
		{
			return ((*iter).*_toString)();
		}
	};
	
	template<typename _CharT, typename _Coll, typename _ElemFormat>
	class NestedCollFormat : public CollFormat<_CharT, _Coll>
	{
	public:
		typedef typename CollFormat<_CharT, _Coll>::ConstIterator ConstIterator;
		typedef typename CollFormat<_CharT, _Coll>::String String;
		
	private:
		const _ElemFormat elemFormat;
		
	public:
		NestedCollFormat(const String& start, const String& end, const String& separator, const _ElemFormat& elemFormat = _ElemFormat())
		: CollFormat<_CharT, _Coll>(start, end, separator), elemFormat(elemFormat)
		{
		}
		
		virtual String toString(const ConstIterator& iter) const { return collToString(*iter, elemFormat); }
	};
	
	template<typename _Coll, typename _CharT = char, typename _Format = CollFormat<_CharT, _Coll> >
	std::basic_string<_CharT> collToString(const _Coll& coll, const _Format& format = SimpleCollFormat<_Coll>())
	{
		// TODO port to ostream_iterator
		std::basic_ostringstream<_CharT> ostream;
		ostream << format.start;
		for (auto iter = coll.begin(); iter != coll.end(); ++iter)
		{
			if (iter != coll.begin())
				ostream << format.separator;
			ostream << format.toString(iter);
		}
		ostream << format.end;
		return ostream.str();
	}
	
}

#endif