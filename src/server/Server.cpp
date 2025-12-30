#include "Server.h"
#include <nlohmann\json_fwd.hpp>
#include "..\socket_extension.h"

using json = nlohmann::json;

const MessageParseStatus Server::ParseMessageAndEnqueue(json& json)
{
	bool authenticated = false;
	if (config->tokens.size() == 0)
	{
		authenticated = true;
	}
	else if (json.contains("token"))
	{
		//Set authenticated to true if the token is valid
		authenticated = std::find(config->tokens.begin(), config->tokens.end(), json["token"].get<std::string>()) != config->tokens.end();
	}

	if (!authenticated)
	{
		SocketExtension::PrintError("Received event message but auth token was missing or invalid");

		return MessageParseStatus::AUTH_FAILED;
	}

	if (json.contains("event") && json.contains("args"))
	{
		EventMessage eventMessage;
		eventMessage.name = json["event"];
		//Allocate via new as a sp handle will take ownership later.
		eventMessage.args = new EventArgs(json["args"]);
		eventQueue.enqueue(eventMessage);
		
		return MessageParseStatus::OK;
	}
	else
	{
		SocketExtension::Print("Received message was missing 'event' and /or 'args' from payload and will be ignored.");
	}

	return MessageParseStatus::PARSE_ERROR;
}