#pragma once
#include <string>
#include "EventArgs.h"

class EventMessage
{
public:
	std::string name;
	EventArgs* args;
};