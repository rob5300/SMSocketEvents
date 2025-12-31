#include "WebsocketServer.h"
#include "../../ext/nlohmann/json.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include "ServerResponse.h"
#include "../socket_extension.h"
#include <exception>
#include <memory>
#include <utility>
#include "..\Config.h"
#include "Server.h"
#include <boost\asio\ip\basic_endpoint.hpp>
#include <boost\asio\ip\impl\address.ipp>
#include <boost\beast\core\error.hpp>
#include <boost\beast\core\flat_buffer.hpp>
#include <boost\beast\http\field.hpp>
#include <boost\beast\version.hpp>
#include <boost\beast\websocket\error.hpp>
#include <boost\beast\websocket\rfc6455.hpp>
#include <boost\beast\websocket\stream.hpp>
#include <boost\beast\websocket\stream_base.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using json = nlohmann::json;

WebsocketServer::WebsocketServer(Config* config) :
io_context(1),
acceptor(io_context, { net::ip::make_address("127.0.0.1"), static_cast<boost::asio::ip::port_type>(config->port)})
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
                char* dataPtr = static_cast<char*>(buffer.data().data());

                //Null terminate string
                dataPtr[size] = 0;

                //Parse json
                json _json = json::parse(dataPtr, dataPtr + size);

                const MessageParseStatus status = ParseMessageAndEnqueue(std::move(_json));
                if (status == MessageParseStatus::AUTH_FAILED)
                {
                    //Write back auth failed response
                    auto response = ServerResponse(false, "Auth failed");
                    ws.write(response.AsBuffer());

                    //Close connection due to auth failure
                    acceptor.cancel();
                    running = false;

                    return;
                }
                else
                {
                    auto response = ServerResponse(true);
                    ws.write(response.AsBuffer());
                }
            }
            else
            {
                auto response = ServerResponse(false, "Malformed message");
                ws.write(response.AsBuffer());
            }
        }
    }
    catch(beast::system_error const& se)
    {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
        {
            SocketExtension::PrintError("Websocket Error: " + se.code().message());
        }
        else
        {
            SocketExtension::Print("Websocket was closed");
        }
    }
    catch(std::exception const& e)
    {
        SocketExtension::PrintError("Websocket Error: " + std::string(e.what()));
    }
}