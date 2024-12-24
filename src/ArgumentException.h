#pragma once
#include <exception>

class ArgumentException : std::exception
{
	public:
	ArgumentException(const char* _message) : message(_message)
	{
	};

	const char* what()
	{
		return message;
	}

	private:
	const char* message;
};