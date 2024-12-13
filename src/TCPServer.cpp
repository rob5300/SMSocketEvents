#include "TCPServer.hpp"
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
            const size_t headerReadLen = boost::asio::read(socket, asio::buffer(&header, sizeof(EventMessageHeader)));
            
            if (std::strncmp(header.magicString, EVENT_MAGIC_STRING, 8) == 0)
            {
                socket.wait(tcp::socket::wait_read);

                char* read_string = new char[header.dataLength + 1];
                read_string[header.dataLength] = 0;
                const size_t readLen = boost::asio::read(socket, boost::asio::buffer(read_string, header.dataLength));
                auto json = json::parse(read_string);
                std::string stringValue = json["string"];
                std::cout << "Message: " << stringValue << std::endl;
                delete[] read_string;
            }

            //std::string message = "Test123abc!";
            //boost::system::error_code ignored_error;
            //boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
	    }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
