#include "TCPServer.hpp"
#include "EventMessage.h"
#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using boost::asio::ip::tcp;
using namespace boost;

//Max buffer size of 10MB;
constexpr long MAX_BUFFER_SIZE = 1024l * 1024l * 10l;

TCPServer::TCPServer(int32_t port)
{
	this->port = port;
    running = false;
}

void TCPServer::Start()
{
	if (!running)
	{
        running = true;
		thread = std::make_unique<std::thread>(&TCPServer::ServerLoop, this);
	}
}

void TCPServer::Stop()
{
    running = false;
    thread->join();
}

void TCPServer::ParseMessageAndEnqueue(json& json)
{
    if (json.contains("event") && json.contains("args"))
    {
        EventMessage eventMessage;
        eventMessage.name = json["event"];
        eventMessage.args = new EventArgs(json["args"]);
        eventQueue.enqueue(eventMessage);
    }
    else
    {
        std::cout << "Received message was missing 'event' and/or 'args' from payload " << std::endl;
    }
}

void TCPServer::ServerLoop()
{
	try
    {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

        while (running)
	    {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            socket.wait(tcp::socket::wait_read);

            EventMessageHeader header;
            try
            {
                const size_t headerReadLen = boost::asio::read(socket, asio::buffer(&header, sizeof(EventMessageHeader)));
            }
            catch (const std::exception& e)
            {
                std::cout << "Header parse exception: " << e.what() << std::endl;
                continue;
            }
            
            if (std::strncmp(header.magicString, EVENT_MAGIC_STRING, 8) == 0)
            {
                if (header.dataLength > MAX_BUFFER_SIZE)
                {
                    //Do not accept buffer with size larger than the max
                    continue;
                }

                socket.wait(tcp::socket::wait_read);
                //Make char buffer for incoming json.
                //Make it 1 bigger to ensure it is null terminated
                char* read_string = new char[header.dataLength + 1];
                read_string[header.dataLength] = 0;
                try
                {
                    const size_t readLen = boost::asio::read(socket, boost::asio::buffer(read_string, header.dataLength));
                    auto json = json::parse(read_string);
                    ParseMessageAndEnqueue(json);
                }
                catch (const std::exception& e)
                {
                    std::cout << "Message parse exception: " << e.what() << std::endl;
                }
                delete[] read_string;
            }

            //std::string message = "Test123abc!";
            //boost::system::error_code ignored_error;
            //boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
	    }
    }
    catch (std::exception& e)
    {
        std::cout << "Server Exception: " << e.what() << std::endl;
    }
}
