#include "ServerResponse.h"

boost::asio::mutable_buffer ServerResponse::AsBuffer()
{
	std::string json = static_cast<nlohmann::json>(*this).get<std::string>();
	auto data = json.data();
	return boost::asio::buffer(data, strlen(data));
}