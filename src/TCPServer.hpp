#pragma once
#include <thread>
#include <memory.h>

class TCPServer
{
	public:
		TCPServer(int32_t port);
		void Start();
		void Stop();

	private:
		bool running;
		int32_t port;
		std::unique_ptr<std::thread> thread;
		void ServerLoop();
};