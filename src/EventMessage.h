#pragma once
#include <string>
#include "KeyValues.h"

class EventMessage
{
public:
	std::string name;
	KeyValues* args;
};