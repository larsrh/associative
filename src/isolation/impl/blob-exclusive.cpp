#include "../isolation.hpp"

namespace associative
{

	class BlobExclusiveIsolation : public IsolationLevel
	{
		ISOLEVEL_DECL;
		
	public:
		BlobExclusiveIsolation()
		: IsolationLevel("blob-exclusive")
		{
		}
		
		virtual bool isIsolated(const boost::shared_ptr<Connection>& conn, uint64_t sessionID) const
		{
			// other sessions may be alive, but they must not have any open handles
			// on blobs in which we have pending modifications
			// That is, no handles from other sessions on at least one of the following
			// objects may exist:
			// (a) blobs we have modified
			// (b) blobs which contain a triple we have modified
			auto query = conn->prepareQuery(
				"select * from journal where journal.session_id = ? and ("
				"  (journal.relation = ? and exists ("
				"    select * from handle "
				"    where handle.relation = journal.relation and handle.relation_id = journal.relation_id and "
				"    handle.session_id != journal.session_id "
				"  )) or "
				"  (journal.relation = ? and exists ("
				"    select * from `blob` "
				"      inner join handle on handle.relation_id = blob.id and handle.relation = ? "
				"      inner join metadata on ( "
				"        metadata.blob_id = blob.id or "
				"        (metadata.object = blob.id and metadata.object_type_id = ?)"
				"      )"
				"    where handle.session_id != journal.session_id and journal.relation_id = metadata.id"
				"  ))"
				")",
			std::string("isolation.blob-exclusive"));
			return !query->execute(convertAll(sessionID, Connection::Relation::Blob, Connection::Relation::Metadata, Connection::Relation::Blob, ASSOCIATIVE_SYS_BLOB_TYPE)).rows.size();
		}
	};

}

ISOLEVEL_DEF(BlobExclusive);
