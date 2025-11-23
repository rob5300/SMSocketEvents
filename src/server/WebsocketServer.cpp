#include "WebsocketServer.h"
#include "../../ext/nlohmann/json.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;

WebsocketServer::WebsocketServer(Config* config) : io_context(1), acceptor(io_context, port)
{
	this->config = config;
	running = false;
}

void WebsocketServer::Start()
{
	if (!running)
	{
		running = true;
		thread = std::make_unique<std::thread>(&WebsocketServer::ServerRun, this);
	}
}

void WebsocketServer::Stop()
{
	running = false;
	try
	{
		acceptor.cancel();
	}
	catch (const std::exception e)
	{
		std::cout << e.what() << std::endl;
	}

	thread->join();
}

void WebsocketServer::ServerRun()
{
	try
    {
        tcp::socket socket{io_context};
        acceptor.accept(socket);

        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-sync");
            }));

        // Accept the websocket handshake
        ws.accept();

        for(;;)
        {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            // Read a message
            const size_t size = ws.read(buffer);

            if (ws.got_text())
            {
                //Parse json and process message
                json json = json::parse(static_cast<char*>(buffer.data().data()));
                const MessageParseStatus status = ParseMessageAndEnqueue(std::move(json));
                if (status == MessageParseStatus::AUTH_FAILED)
                {
                    //Close connection due to auth failure
                    acceptor.cancel();
                    running = false;

                    return;
                }
            }
        }
    }
    catch(beast::system_error const& se)
    {
        // This indicates that the session was closed
        if(se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}