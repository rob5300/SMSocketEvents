#pragma once
#include <thread>
#include <memory.h>
#include <string.h>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include "concurrentqueue.h"
#include "EventMessage.h"
#include "SignatureHelper.h"
#include "Config.h"

constexpr const char* EVENT_MAGIC_STRING = "EVENTMSG";

struct EventMessageHeader
{
	//Magic string which should be 'EVENTMSG'
	char magicString[8];

	// Length of data
	int64 dataLength;

	//SHA-256 signature for data body
	char signature[384];
};

class TCPServer
{
	public:
		TCPServer(Config* config);
		void Start();
		void Stop();
		moodycamel::ConcurrentQueue<EventMessage> eventQueue;

	private:
		bool running;
		Config* config;
		int32_t port;
		std::unique_ptr<std::thread> thread;
		boost::asio::io_context io_context;
		boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::socket socket;
		boost::asio::ip::tcp::endpoint endpoint;
		std::unique_ptr<SignatureHelper> sigHelper;

		void ServerRun();
		void StartAccept();
		void HandleAccept(const boost::system::error_code& error);
		void AcceptMessageHeaderAndBody();

		/// <summary>
		/// Parse message json and enqueue a new event message
		/// </summary>
		void ParseMessageAndEnqueue(nlohmann::json& json);
};