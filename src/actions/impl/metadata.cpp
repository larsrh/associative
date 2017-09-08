#include "../action.hpp"
#include "../../objects/file.hpp"

using namespace po;

namespace associative
{
	
	class MetadataAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("metadata options");
			desc->add_options()
				("uuid", value<std::string>(), "UUID of the file")
				("blob-name", value<std::string>(), "subject blob name")
				("predicate-prefix", value<std::string>(), "short name of the predicate prefix")
				("predicate", value<std::string>(), "predicate")
				("object-prefix", value<std::string>(), "short name of the object prefix")
				("object-type", value<std::string>(), "name of the object type")
				("object", value<std::string>(), "object")
				("object-uuid", value<std::string>(), "UUID of the file of the object blob")
				("object-blob-name", value<std::string>(), "object blob name")
				("verbose", "Increase verbosity");
			return desc;
		}
		
	public:
		MetadataAction()
		: Action("metadata")
		{
		}
		
		virtual int perform(const variables_map& vm, const std::vector<std::string>& parameters, Environment& env)
		{
			if (!vm.count("uuid") || !vm.count("blob-name") || parameters.size() != 1)
				return 1;
			
			bool verbose = vm.count("verbose");
			auto op = parameters.front();
			auto blob = env.getFile(vm["uuid"].as<std::string>())->getBlob(vm["blob-name"].as<std::string>());
			
			if (op == "list")
			{
				std::cout << collToString(
					blob->getTriples(TripleFilter()),
					ConvertingCollFormat<std::vector<Triple> >("", "", "\n",
						verbose ? &Triple::toVerboseString : &Triple::toSimpleString
					)
				);
			}
			else if (op == "add")
			{
				if (!vm.count("predicate-prefix") || !vm.count("predicate"))
					return 1;
				
				bool normalObject = vm.count("object");
				if (vm.count("object-type") != normalObject || vm.count("object-prefix") != normalObject ||
					vm.count("object-uuid") == normalObject || vm.count("object-blob-name") == normalObject)
					return 1;
				
				auto predicatePrefix = Prefix::get(env.getConnection(), vm["predicate-prefix"].as<std::string>(), boost::none);
				auto predicate = vm["predicate"].as<std::string>();
				
				if (normalObject)
				{
					auto objectPrefix = Prefix::get(env.getConnection(), vm["object-prefix"].as<std::string>(), boost::none);
					auto objectType = Type::get(env.getConnection(), vm["object-type"].as<std::string>(), objectPrefix);
					blob->addTriple(predicatePrefix, predicate, objectType, vm["object"].as<std::string>());
				}
				else
				{
					auto blobObject = env.getFile(vm["object-uuid"].as<std::string>())->getBlob(vm["object-blob-name"].as<std::string>());
					blob->addTriple(predicatePrefix, predicate, *blobObject);
				}
			}
			else
			{
				return 1;
			}
			
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(Metadata);
