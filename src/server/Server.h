#pragma once
#include "concurrentqueue.h"
#include "../EventMessage.h"
#include "..\Config.h"
#include <nlohmann\json.hpp>

enum MessageParseStatus: char {OK, PARSE_ERROR, AUTH_FAILED};

class Server
{
public:
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() = 0;

	moodycamel::ConcurrentQueue<EventMessage> eventQueue;

protected:
	Config* config;

	/// Parse message json and enqueue a new event message
	const MessageParseStatus ParseMessageAndEnqueue(nlohmann::json& json);
};