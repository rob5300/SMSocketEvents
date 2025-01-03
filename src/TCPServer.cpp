#include "TCPServer.hpp"
#include "EventMessage.h"
#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include "socket_extension.h"

using json = nlohmann::json;
using boost::asio::ip::tcp;
using namespace boost;

//Max buffer size of 10MB;
constexpr long MAX_BUFFER_SIZE = 1024l * 1024l * 10l;

TCPServer::TCPServer(Config* config) :
	config(config), io_context(), endpoint(tcp::endpoint(tcp::v4(), config->port)), acceptor(io_context), socket(io_context)
{
	this->port = port;
	running = false;
	sigHelper = std::make_unique<SignatureHelper>(config->public_key_pem);
}

void TCPServer::Start()
{
	if (!running)
	{
		running = true;
		acceptor.open(endpoint.protocol());
		acceptor.set_option(tcp::acceptor::reuse_address(true));
		acceptor.bind(endpoint);
		acceptor.listen();
		thread = std::make_unique<std::thread>(&TCPServer::ServerRun, this);
	}
}

void TCPServer::Stop()
{
	running = false;
	try
	{
		acceptor.cancel();
		socket.close();
	}
	catch (const std::exception e)
	{
		std::cout << e.what() << std::endl;
	}

	thread->join();
}

void TCPServer::ParseMessageAndEnqueue(json& json)
{
	if (json.contains("event") && json.contains("args"))
	{
		EventMessage eventMessage;
		eventMessage.name = json["event"];
		//Allocate via new as a sp handle will take ownership later.
		eventMessage.args = new EventArgs(json["args"]);
		eventQueue.enqueue(eventMessage);
	}
	else
	{
		SocketExtension::Print("Received message was missing 'event' and /or 'args' from payload and will be ignored.");
	}
}

void TCPServer::ServerRun()
{
	while (running)
	{
		try
		{
			acceptor.accept(socket);
			AcceptMessageHeaderAndBody();
			socket.close();
		}
		catch (const std::exception& e)
		{
			SocketExtension::PrintError("Server Exception: " + std::string(e.what()));
		}
	}

	if (!socket.is_open())
	{
		SocketExtension::Print("Socket on port '" + std::to_string(port) + "' was closed.");
	}
}

void TCPServer::AcceptMessageHeaderAndBody()
{
	socket.wait(boost::asio::socket_base::wait_read);

	EventMessageHeader header;
	try
	{
		const size_t headerReadLen = boost::asio::read(socket, asio::buffer(&header, sizeof(EventMessageHeader)));
	}
	catch (const std::exception& e)
	{
		SocketExtension::PrintError("Header parse exception: " + std::string(e.what()));
		acceptor.cancel();
		return;
	}

	if (!running) return;

	if (std::strncmp(header.magicString, EVENT_MAGIC_STRING, 8) == 0)
	{
		if (header.dataLength > MAX_BUFFER_SIZE)
		{
			//Do not accept buffer with size larger than the max
			SocketExtension::PrintError("Message was rejected due to data size exceeding maximum.");
			acceptor.cancel();
			return;
		}

		socket.wait(tcp::socket::wait_read);

		//Make char buffer for incoming json.
		const size_t bufferSize = header.dataLength;
		std::unique_ptr<char[]> read_string = std::make_unique<char[]>(bufferSize + 1);
		
		try
		{
			const size_t readLen = boost::asio::read(socket, boost::asio::buffer(read_string.get(), header.dataLength));

			//Verify signature if enabled
			if (config->secure)
			{
				const bool sigMatch = sigHelper->IsValid(read_string.get(), bufferSize, reinterpret_cast<unsigned char*>(header.signature), sizeof(header.signature));
				if (!sigMatch)
				{
					SocketExtension::PrintError("Message signature validation failed");
					return;
				}
			}

			auto json = json::parse(read_string.get());
			ParseMessageAndEnqueue(json);
		}
		catch (const std::exception& e)
		{
			SocketExtension::Print("Message parse exception : \n" + std::string(e.what()));
			acceptor.cancel();
		}
	}
}
