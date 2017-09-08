#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "blob.hpp"
#include "../env/environment.hpp"

uint64_t associative::Blob::ensureContentType()
{
	auto& conn = env.getConnection();
	auto query = conn.prepareQuery("select id from content_type where mime = ?", std::string("blob.content-type"));
	auto result = query->execute(convertAll(contentType));
	if (result.rows.size())
		return boost::lexical_cast<uint64_t>(result.rows.begin()->at(0));

	auto stmt = conn.prepareStatement("insert into content_type values (?, ?)", std::string("blob.content-type.add"));
	uint64_t id = conn.nextID("content_type");
	stmt->execute(convertAll(id, contentType));
	return id;
}

void associative::Blob::createBlob()
{
	if (!env.getSessionID())
		throw Exception("not in a session");
	
	auto& conn = env.getConnection();
	id = conn.nextID("blob");
	auto stmt = conn.prepareStatement("insert into `blob` values (?, ?, ?, ?, 0)", std::string("blob.add"));
	stmt->execute(convertAll(id, file.getID(), name, ensureContentType()));
	
	stmt = conn.prepareStatement("insert into journal values (?, ?, ?, ?, ?, null, 0)", std::string("blob.journal.add"));
	stmt->execute(convertAll(conn.nextID("journal"), *env.getSessionID(), Connection::Relation::Blob, id, Operation::Add));
}

associative::Blob::Blob(associative::Environment& env, associative::File& file, const std::string& name, const std::string& contentType, boost::optional<uint64_t> id)
: env(env), file(file), removed(false), name(name), contentType(contentType)
{
	if (id)
		this->id = *id;
	else
		createBlob();
	handleID = env.getConnection().openHandle(Connection::Relation::Blob, this->id, *env.getSessionID());
}

associative::Blob::~Blob()
{
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	conn.closeHandle(handleID);
	t->commit();
}

uint64_t associative::Blob::getID()
{
	return id;
}

const associative::File& associative::Blob::getFile() const
{
	return file;
}

void associative::Blob::remove()
{
	if (removed)
		throw formatException(boost::format("blob with name %1% from file with uuid %2% has been removed") % name % file.uuid);
	if (!env.getSessionID())
		throw Exception("not in a session");
	
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	auto stmt = conn.prepareStatement("insert into journal values (?, ?, ?, ?, ?, null, 0)", std::string("blob.journal.remove"));
	stmt->execute(convertAll(conn.nextID("journal"), *env.getSessionID(), Connection::Relation::Blob, id, Operation::Remove));
	
	newTriples.clear();
	
	removed = true;
	t->commit();
}

bool associative::Blob::isRemoved()
{
	return removed;
}

fs::path associative::Blob::getPath(bool write)
{
	if (removed)
		throw formatException(boost::format("blob with name %1% from file with uuid %2% has been removed") % name % file.uuid);
	return env.getVFS().getBlobPath(env, *this, write);
}

std::vector<associative::Triple> associative::Blob::getTriples(const associative::TripleFilter&)
{
	if (removed)
		throw formatException(boost::format("blob with name %1% from file with uuid %2% has been removed") % name % file.uuid);
	
	Buffer<boost::shared_ptr<Prefix> > prefixBuffer;
	Buffer<boost::shared_ptr<Type> > typeBuffer;
	
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	auto query = conn.prepareQuery(
		"select pprefix.id, pprefix.name, pprefix.uri, "
		"oprefix.id, oprefix.name, oprefix.uri, type.id, type.name, "
		"metadata.id, metadata.blob_id, metadata.predicate, metadata.object "
		"from metadata "
		"inner join type on metadata.object_type_id = type.id "
		"inner join prefix pprefix on metadata.predicate_prefix_id = pprefix.id "
		"inner join prefix oprefix on type.prefix_id = oprefix.id "
		"where metadata.visible = 1 and metadata.blob_id = ?",
	std::string("blobs.triples.get"));
	auto result = query->execute(convertAll(id));
	
	auto blobType = Type::getBlobType(conn);
	typeBuffer.set(toString(blobType->id), blobType);
	prefixBuffer.set(toString(blobType->prefix->id), blobType->prefix);
	t->commit();
	
	// As Triple is immutable, there is no assignment operator available.
	// Unfortunately, std::vector needs an assignment operator even if it is
	// never called (e. g. by ensuring the right capacity upon creation of the
	// vector). (Ugly hack: define an assignment operator which throws.)
	// Instead, we're using a std::list instead to buffer all the triples.
	std::list<Triple> triples;
	
	for (auto iter = result.rows.begin(); iter != result.rows.end(); ++iter)
	{
		auto tripleID = boost::lexical_cast<uint64_t>(iter->at(8));
		if (containsKey(removedTriples, tripleID))
			continue;
		
		// I'd rather use boost::lambda here, but that's not possible
		// because Prefix::Prefix and Type::Type are private
		auto predicatePrefix = prefixBuffer.getOrElse(iter->at(0), [&iter]() {
			return boost::shared_ptr<Prefix>(new Prefix(
				boost::lexical_cast<uint64_t>(iter->at(0)), iter->at(1), iter->at(2)
			));
		});
		auto objectPrefix = prefixBuffer.getOrElse(iter->at(3), [&iter]() {
			return boost::shared_ptr<Prefix>(new Prefix(
				boost::lexical_cast<uint64_t>(iter->at(3)), iter->at(4), iter->at(5)
			));
		});
		auto type = typeBuffer.getOrElse(iter->at(6), [&]() {
			return boost::shared_ptr<Type>(new Type(
				boost::lexical_cast<uint64_t>(iter->at(6)), iter->at(7), objectPrefix
			));
		});
		
		triples.push_back(Triple(tripleID, this, predicatePrefix, iter->at(10), type, iter->at(11)));
	}
	
	triples.insert(triples.end(), newTriples.begin(), newTriples.end());
	return std::vector<Triple>(triples.begin(), triples.end());
}

void associative::Blob::addTriple(const associative::Triple& triple)
{
	Connection& conn = env.getConnection();
	
	auto stmt = conn.prepareStatement("insert into metadata values (?, ?, ?, ?, ?, ?, 0)", std::string("blob.triple.add"));
	stmt->execute(convertAll(triple.id, this->id,
		triple.predicatePrefix->id, triple.predicate,
		triple.objectType->id, triple.object));
	
	stmt = conn.prepareStatement("insert into journal values (?, ?, ?, ?, ?, null, 0)", std::string("blob.triple.journal.add"));
	stmt->execute(convertAll(conn.nextID("journal"), *env.getSessionID(), Connection::Relation::Metadata, triple.id, Triple::Operation::Add));
	
	newTriples.push_back(triple);
}


associative::Triple associative::Blob::addTriple(
	const boost::shared_ptr<associative::Prefix>& predicatePrefix, const std::string& predicate, 
	Blob& blobObject
)
{
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	auto blobType = Type::getBlobType(conn);
	Triple triple(conn.nextID("metadata"), this, predicatePrefix, predicate, blobType, toString(blobObject.id));
	addTriple(triple);
	t->commit();
	return triple;
}

associative::Triple associative::Blob::addTriple(
	const boost::shared_ptr<associative::Prefix>& predicatePrefix, const std::string& predicate, 
	const boost::shared_ptr<associative::Type>& objectType, const std::string& object
)
{
	if (objectType->id == ASSOCIATIVE_SYS_BLOB_TYPE)
		throw Exception("cannot use a blob as an object for a tuple here, use other method instead");
	
	// TODO some duplicated code
	auto& conn = env.getConnection();
	auto t = conn.transaction();
	Triple triple(conn.nextID("metadata"), this, predicatePrefix, predicate, objectType, object);
	addTriple(triple);
	t->commit();
	return triple;
}
