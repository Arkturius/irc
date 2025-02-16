#include "irc.h"
#include <RplGenerator.h>
#include <cstdarg>

RplGenerator::RplGenerator() {}

RplGenerator::RplGenerator(str client, str channel, str topic, str clientList):
	_client(client), _channel(channel), _topic(topic), _clientList(clientList)
{}

#include <sstream>
#include <iomanip>

str	RplGenerator::generate(str prefix, IRCReplyCode code, uint32_t paramCount, ...)
{
	std::stringstream	stream;
	str					reply = "";
	va_list				list;

	reply += prefix;
	stream << std::setfill('0') << std::setw(3) << code;
	reply += stream.str();
	va_start(list, paramCount);
	for (size_t i = 0; i < paramCount; ++i)
	{
		str	param = va_arg(list, char *);

		reply += " ";
		if (param.find(" ") != str::npos || param.find(":") != str::npos || i == paramCount)
			reply += ":";
		reply += param;
	}
	va_end(list);
	IRC_LOG("generated reply %d : [%s]", code, reply.c_str());
	return (reply);
}

str	RplGenerator::rpl_join()
{
	return ":" + _client + " JOIN " + _channel;
}
str RplGenerator::rpl_topic()
{
	return "332 " + _client + " " + _channel + " :" + _topic;
}
str RplGenerator::rpl_namreply()
{
	return "353 " + _client + " = " + _channel + " :" + _clientList;
}
str RplGenerator::rpl_endofnames()
{
	return "366 " + _client + " " + _channel;
}


