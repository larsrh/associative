#include "../isolation.hpp"

namespace associative
{

	class FileExclusiveIsolation : public IsolationLevel
	{
		ISOLEVEL_DECL;
		
	public:
		FileExclusiveIsolation()
		: IsolationLevel("file-exclusive")
		{
		}
		
		virtual bool isIsolated(const boost::shared_ptr<Connection>& conn, uint64_t sessionID) const
		{
			// other sessions may be alive, but they must not have any open handles
			// on files in which we have pending modifications
			// That is, no handles from other sessions on at least one of the following
			// objects may exist:
			// (a) files we have modified
			// (b) blobs we have modified
			// (c) files which contain a blob we have modified
			// (d) blobs which contain a triple we have modified
			auto query = conn->prepareQuery(
				"select * from journal where journal.session_id = ? and ("
				"  (exists ("
				"    select * from handle "
				"    where handle.relation = journal.relation and handle.relation_id = journal.relation_id and "
				"    handle.session_id != journal.session_id "
				"  )) or "
				"  (journal.relation = ? and exists ("
				"    select * from file "
				"      inner join handle on handle.relation_id = file.id and handle.relation = ? "
				"      inner join `blob` on blob.file_id = file.id"
				"    where handle.session_id != journal.session_id and journal.relation_id = blob.id"
				"  )) or "
				"  (journal.relation = ? and exists ("
				"    select * from `blob` "
				"      inner join file on file.id = blob.file_id "
				"      inner join handle on handle.relation_id = file.id and handle.relation = ? "
				"      inner join metadata on ( "
				"        metadata.blob_id = blob.id or "
				"        (metadata.object = blob.id and metadata.object_type_id = ?)"
				"      )"
				"    where handle.session_id != journal.session_id and journal.relation_id = metadata.id"
				"  ))"
				")",
			std::string("isolation.file-exclusive"));
			return !query->execute(convertAll(sessionID,
				Connection::Relation::Blob, Connection::Relation::File,
				Connection::Relation::Metadata, Connection::Relation::File, ASSOCIATIVE_SYS_BLOB_TYPE
				)).rows.size();
		}
	};

}

ISOLEVEL_DEF(FileExclusive);
