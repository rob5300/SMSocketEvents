#pragma once
#include <thread>
#include <memory.h>
#include <string.h>
#include <nlohmann/json.hpp>
#include "KeyValues.h"
#include "concurrentqueue.h"
#include "EventMessage.h"

constexpr const char* EVENT_MAGIC_STRING = "EVENTMSG";

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

		/// <summary>
		/// Parse message json and enqueue a new event message
		/// </summary>
		void ParseMessageAndEnqueue(nlohmann::json& json);
};