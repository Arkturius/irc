#include <RplGenerator.h>


RplGenerator::RplGenerator(str client, str channel, str topic, str clientList):
	_client(client), _channel(channel), _topic(topic), _clientList(clientList)
{}

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


