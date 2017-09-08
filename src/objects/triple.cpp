#include <sstream>

#include <boost/uuid/uuid_io.hpp>

#include "triple.hpp"

associative::Triple::Triple(
	const uint64_t id, const Blob* blob,
	const boost::shared_ptr<associative::Prefix>& predicatePrefix, const std::string& predicate,
	const boost::shared_ptr<associative::Type>& objectType, const std::string& object)
: blob(blob), id(id),
  predicatePrefix(predicatePrefix), predicate(predicate),
  objectType(objectType), object(object)
{
}

std::string associative::Triple::toSimpleString() const
{
	return toString(false);
}

std::string associative::Triple::toVerboseString() const
{
	return toString(true);
}

std::string associative::Triple::toString(bool verbose) const
{
	std::ostringstream stream;
	stream << "(";
	stream << blob->getFile().uuid << ":" << blob->name;
	stream << ",";
	if (verbose)
		stream << "<" << predicatePrefix->uri << predicate << ">";
	else
		stream << predicatePrefix->name << ":" << predicate;
	stream << ",";
	if (verbose)
		stream << object << " :: <" << objectType->prefix->uri << objectType->name << ">";
	else
		stream << object;
	stream << ")";
	return stream.str();
}
