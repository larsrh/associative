#include <fstream>

#include "util.hpp"

inline std::size_t skipWhitespaces(const std::string& str, std::size_t pos)
{
	for (; pos < str.size(); ++pos)
		if (!std::isspace(str[pos]))
			return pos;
	return pos + 1;
}

std::pair<std::size_t, std::string> readEscaped(const std::string& str, std::size_t pos, const boost::optional<char> delim)
{
	std::string buf;
	bool escaped = false;
	for (; pos < str.size(); ++pos)
	{
		char c = str[pos];
		if (escaped)
		{
			if (c == '\\')
				buf += '\\';
			else if (c == delim)
				buf += (*delim);
			else
				throw associative::formatException(boost::format("unknown escape symbol \\%1%") % c);
			escaped = false;
		}
		else
		{
			if (c == '\\')
			{
				if (delim)
					escaped = true;
				else
					buf += c;
			}
			else if (c == delim || (!delim && std::isspace(c)))
			{
				break;
			}
			else if (!delim && (c == '\'' || c == '"'))
			{
				--pos;
				break;
			}
			else
			{
				buf += c;
			}
		}
	}
	return std::make_pair(pos + 1, buf);
}

std::pair<std::size_t, std::string> readArgument(const std::string& str, std::size_t pos)
{
	pos = skipWhitespaces(str, pos);
	if (pos >= str.size())
		return std::make_pair(pos, std::string());
		
	char c = str.at(pos);
	if (c == '\'' || c == '"')
	{
		auto pair = readEscaped(str, pos+1, c);
		return pair;
	}
	else
	{
		return readEscaped(str, pos, boost::none);
	}
}

std::vector<std::string> associative::parseArguments(const std::string& str)
{
	std::vector<std::string> arguments;
	if (str.empty())
		return arguments;
	
	std::pair<std::size_t, std::string> pair;
	do
	{
		pair = readArgument(str, pair.first);
		if (!pair.second.empty())
			arguments.push_back(pair.second);
	}
	while (pair.first < str.size());
	return arguments;
}

/*
 * fnv - Fowler/Noll/Vo- hash code
 *
 * @(#) $Revision: 5.4 $
 * @(#) $Id: fnv.h,v 5.4 2009/07/30 22:49:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv.h,v $
 *
 ***
 *
 * Fowler/Noll/Vo- hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 ***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *	chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy! :-)
 */

std::string associative::fnv1a(const std::string& str)
{
	uint64_t magicPrime = 0x100000001b3ULL;
	uint64_t hash = 0xcbf29ce484222325ULL;
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		hash ^= (uint64_t)str[i];
		hash *= magicPrime;
	}
	std::stringstream convert;
	convert << std::hex << std::setw(8) << std::setfill('0') << hash;
	return convert.str();
}
