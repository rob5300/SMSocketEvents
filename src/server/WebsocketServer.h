#pragma once
#include "Server.h"
#include "Config.h"
#undef clamp

#include <boost/asio/ip/tcp.hpp>

class WebsocketServer : public Server
{
public:
	WebsocketServer(Config* config);

	void Start();
	void Stop();
	bool IsRunning() { return running; }

private:
	bool running;
	std::unique_ptr<std::thread> thread;
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::acceptor acceptor;

	void ServerRun();
};