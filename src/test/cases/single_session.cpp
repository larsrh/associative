#include <sstream>

#include <boost/uuid/uuid_io.hpp>

#include "../test.hpp"
#include "../../util/io.hpp"
#include "../../util/util.hpp"

#include "gen/isolevel_impls.hpp"

namespace associative { namespace test {

class Simple : public SingleTest {};
	
TEST_F(Simple, CreateWriteRead)
{
	auto& env = bench->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	std::istringstream iss("content");
	storeFile(file->getBlob("default")->getPath(true), iss);
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	env.startSession();
	std::ostringstream oss;
	file = env.getFile(uuid);
	std::ifstream ifs(file->getBlob("default")->getPath(false).c_str());
	copyStreams(ifs, oss);
	ASSERT_EQ("content", oss.str()) << "Content in file differs from content written";
	env.commitSession(IsolationLevels::Full);
}

TEST_F(Simple, CreateRollback)
{
	auto& env = bench->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	std::istringstream iss("content");
	storeFile(file->getBlob("default")->getPath(true), iss);
	auto uuid = boost::lexical_cast<std::string>(file->uuid);
	env.rollbackSession();
	
	ASSERT_EQ((unsigned) 0, bench->conn->prepareQuery(
		"select * from file where uuid = ?"
	)->execute(convertAll(uuid)).rows.size()) << "Supposedly non-existing file exists";
}

TEST_F(Simple, RemoveFile)
{
	auto& env = bench->env;
	env.startSession();
	auto file = env.createFile();
	file->addBlob("default", "text/plain");
	file->addBlob("second", "text/plain");
	auto uuid = toString(file->uuid);
	env.commitSession(IsolationLevels::Full);
	
	env.startSession();
	file = env.getFile(uuid);
	file->removeFile();
	ASSERT_THROW(file->getBlob("default"), Exception) << "Expected exception";
	ASSERT_THROW(file->getBlob("second"), Exception) << "Expected exception";
	env.commitSession(IsolationLevels::Full);
	
	env.startSession();
	file = env.getFile(uuid);
	ASSERT_THROW(file->getBlob("default"), Exception) << "Expected exception";
	ASSERT_THROW(file->getBlob("second"), Exception) << "Expected exception";
	env.rollbackSession();
}

}}