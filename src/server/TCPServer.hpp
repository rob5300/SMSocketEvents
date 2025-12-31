#pragma once
#include <thread>
#include <memory.h>
#include <string.h>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include "../SignatureHelper.h"
#include "../Config.h"
#include "Server.h"
#include <platform.h>

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

class TCPServer : public Server
{
	public:
		TCPServer(Config* config);
		void Start();
		void Stop();
		bool IsRunning() { return running; }

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
		void AcceptMessageHeaderAndBody();
};