#include "TCPServer.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using boost::asio::ip::tcp;

//Max buffer size of 100MB;
constexpr long MAX_BUFFER_SIZE = 1024l * 1024l * 100l;

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

void TCPServer::ServerLoop()
{
	try
    {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

        while (running)
	    {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            long dataLen = -1;
            boost::asio::mutable_buffer dataLenBuffer(&dataLen, sizeof(long));
            boost::asio::read(socket, dataLenBuffer);
            if (dataLen > MAX_BUFFER_SIZE)
            {
                //Proposed size is too large
                continue;
            }

            char* read_string = new char[dataLen + sizeof(long) + 1];
            read_string[dataLen + sizeof(long)] = 0;
            char* offset_string = read_string + sizeof(long);
            boost::asio::read(socket, boost::asio::buffer(read_string, dataLen + sizeof(long)));

            auto json = json::parse(offset_string);
            std::string stringValue = json["string"];

            delete(read_string);
            //std::string message = "Test123abc!";
            //boost::system::error_code ignored_error;
            //boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
	    }
    }
    catch (std::exception& const e)
    {
        std::cout << e.what() << std::endl;
    }
}
