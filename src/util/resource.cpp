#include "resource.hpp"

associative::HandleBase::~HandleBase()
{
}

associative::DefaultHandle::DefaultHandle(associative::DefaultResourceBase* parent)
: parent(parent)
{
}

associative::DefaultHandle::~DefaultHandle()
{
	parent->close();
}
