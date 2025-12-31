#pragma once
#include <string>
#include <nlohmann\json.hpp>
#include <boost\asio\buffer.hpp>

class ServerResponse
{
public:
	bool success = false;
	std::string message;

	ServerResponse(bool success, const char* message = "")
	{
		this->success = success;
		this->message = message;
	}

	boost::asio::mutable_buffer AsBuffer();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ServerResponse, success, message)
