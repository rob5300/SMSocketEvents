#include "TCPServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

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

            const size_t bufferLength = 10;
            char* read_string = new char[bufferLength];
            boost::asio::mutable_buffer buffer(read_string, bufferLength);
            boost::asio::read(socket, buffer);

            std::cout << std::format("SOCKET READ: '{}'", read_string) << std::endl;
            int length = strlen(read_string);

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
