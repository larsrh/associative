#include <sstream>

#include <boost/uuid/uuid_io.hpp>

#include "../test.hpp"
#include "../../util/io.hpp"
#include "../../util/util.hpp"
#include "../../util/exception.hpp"

#include "gen/isolevel_impls.hpp"

namespace associative { namespace test {

class Concurrent : public Test {};
	
TEST_F(Concurrent, IsolationUnsafe1)
{
	auto& env = createBench()->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	std::istringstream iss("content");
	storeFile(file->getBlob("default")->getPath(true), iss);
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.getFile(uuid);
	file1->removeBlob("default");
	
	auto file2 = env2.getFile(uuid);
	iss.seekg(std::ios_base::beg);
	storeFile(file2->getBlob("default")->getPath(true), iss);
	
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

TEST_F(Concurrent, IsolationUnsafe2)
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
	auto file2 = env2.getFile(uuid);
	auto blob = file2->getBlob("default");
	file1->removeBlob("default");
	env1.commitSession(IsolationLevels::Unsafe);
	
	std::istringstream iss("content");
	storeFile(blob->getPath(true), iss);
	ASSERT_THROW(env2.commitSession(IsolationLevels::Unsafe), CommitException) << "Expected exception";
	
	env2.rollbackSession();
}

TEST_F(Concurrent, IsolationBlobExclusive)
{
	auto& env = createBench()->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	std::istringstream iss("content");
	storeFile(file->getBlob("default")->getPath(true), iss);
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.getFile(uuid);
	file1->addBlob("second", "text/plain");
	
	auto file2 = env2.getFile(uuid);
	iss.seekg(std::ios_base::beg);
	storeFile(file2->getBlob("default")->getPath(true), iss);
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::FileExclusive), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::BlobExclusive);
	env2.commitSession(IsolationLevels::Full);
}

TEST_F(Concurrent, IsolationFileExclusive)
{
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.createFile();
	file1->addBlob("default", "text/plain");
	
	auto file2 = env2.createFile();
	file2->addBlob("default", "text/plain");
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env1.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::AlmostFull), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::FileExclusive);
	env2.commitSession(IsolationLevels::Full);
}

TEST_F(Concurrent, IsolationAlmostFull)
{
	auto& env1 = createBench()->env;
	auto& env2 = createBench()->env;
	env1.startSession();
	env2.startSession();
	
	auto file1 = env1.createFile();
	file1->addBlob("default", "text/plain");
	
	ASSERT_THROW(env1.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	ASSERT_THROW(env2.commitSession(IsolationLevels::Full), CommitException) << "Expected exception";
	
	env1.commitSession(IsolationLevels::AlmostFull);
	env2.commitSession(IsolationLevels::Full);
}

TEST_F(Concurrent, IsolationFull)
{
	// This test case is trivial (as already tested in single_session.cpp),
	// but is included here for completeness
	
	auto& env = createBench()->env;
	env.startSession();
	
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	
	env.commitSession(IsolationLevels::Full);
}

}}