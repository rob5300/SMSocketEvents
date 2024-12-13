#pragma once
#include <thread>
#include <memory.h>
#include <string.h>
#include <nlohmann/json.hpp>
#include "KeyValues.h"
#include "concurrentqueue.h"

constexpr const char* EVENT_MAGIC_STRING = "EVENTMSG";

struct EventMessage
{
	std::string name;
	KeyValues args;
};

struct EventMessageHeader
{
	char magicString[8];
	int64 dataLength;
};

class TCPServer
{
	public:
		TCPServer(int32_t port);
		void Start();
		void Stop();
		moodycamel::ConcurrentQueue<EventMessage> eventQueue;

	private:
		bool running;
		int32_t port;
		std::unique_ptr<std::thread> thread;
		void ServerLoop();
		void EnqueueNewEventMessage(nlohmann::json& json);
};