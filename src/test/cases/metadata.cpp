#include <sstream>

#include <boost/uuid/uuid_io.hpp>

#include "../test.hpp"
#include "../../util/io.hpp"
#include "../../util/util.hpp"
#include "../../objects/type.hpp"

#include "gen/isolevel_impls.hpp"

namespace associative { namespace test {

class Metadata : public Test {};

TEST_F(Metadata, SimpleAdd)
{
	auto& env = createBench()->env;
	auto& conn = env.getConnection();
	
	env.startSession();
	auto file = env.createFile();
	auto blob = file->addBlob("default", "text/plain");
	auto uuid = toString(file->uuid);
	blob->addTriple(Prefix::get(conn, "default", boost::make_optional(std::string("/"))), "test", *blob);
	env.commitSession(IsolationLevels::Full);
	
	env.startSession();
	blob = env.getFile(uuid)->getBlob("default");
	auto triples = blob->getTriples(TripleFilter());
	ASSERT_EQ((unsigned) 1, triples.size()) << "Different number of triples read than written";
	
	auto triple = triples.front();
	
	ASSERT_EQ(Prefix::get(conn, "default")->id, triple.predicatePrefix->id);
	ASSERT_EQ("test", triple.predicate);
	ASSERT_EQ(Type::getBlobType(conn)->id, triple.objectType->id);
	
	env.commitSession(IsolationLevels::Full);
}

TEST_F(Metadata, IsolationBlobExclusive)
{
	auto& env = createBench()->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.getFile(uuid);
	file1->addBlob("second", "text/plain");
	
	auto file2 = env2.getFile(uuid);
	auto blob = file2->getBlob("default");
	blob->addTriple(Prefix::get(env.getConnection(), "default", boost::make_optional(std::string("/"))), "test", *blob);
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::BlobExclusive);
	env2.commitSession(IsolationLevels::Full);
}

TEST_F(Metadata, IsolationFileExclusive)
{
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	env1.createFile();
	
	auto file = env2.createFile();
	auto blob = file->addBlob("default", "text/plain");
	blob->addTriple(Prefix::get(env2.getConnection(), "default", boost::make_optional(std::string("/"))), "test", *blob);
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::FileExclusive);
	env2.commitSession(IsolationLevels::Full);
}

TEST_F(Metadata, IsolationUnsafeSubject)
{
	auto& env = createBench()->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	file->addBlob("second", "text/plain");
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.getFile(uuid);
	file1->removeBlob("default");
	
	auto file2 = env2.getFile(uuid);
	auto blob1 = file2->getBlob("default");
	auto blob2 = file2->getBlob("second");
	blob1->addTriple(Prefix::get(env.getConnection(), "default", boost::make_optional(std::string("/"))), "test", *blob2);
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::BlobExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::BlobExclusive), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::Unsafe);
	
	ASSERT_THROW(env2.commitSession(IsolationLevels::Unsafe), CommitException) << "Expected exception";
	
	env2.rollbackSession();
}

TEST_F(Metadata, IsolationUnsafeObject)
{
	auto& env = createBench()->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	file->addBlob("second", "text/plain");
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.getFile(uuid);
	file1->removeBlob("default");
	
	auto file2 = env2.getFile(uuid);
	auto blob1 = file2->getBlob("default");
	auto blob2 = file2->getBlob("second");
	blob2->addTriple(Prefix::get(env.getConnection(), "default", boost::make_optional(std::string("/"))), "test", *blob1);
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::BlobExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::BlobExclusive), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::Unsafe);
	
	ASSERT_THROW(env2.commitSession(IsolationLevels::Unsafe), CommitException) << "Expected exception";
	
	env2.rollbackSession();
}

}}