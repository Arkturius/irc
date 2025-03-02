#include <climits>
#include <poll.h>
#include <sstream>
#include <unistd.h>

#include <Channel.h>
#include <Client.h>
#include <time.h>
#include <Server.h>

void	Server::_sendJoin(Client &client, Channel *channel)
{
	str	clientName = client.get_nickname();
	str channelName = channel->get_name();
	str topic = channel->get_topic();

	std::vector<str>	clientList;
	std::vector<int> fdClientList;
	fdClientList = channel->get_fdClient();
	for (IRC_AUTO it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList.push_back(s->second.get_nickname());
	}
	fdClientList = channel->get_fdAdminClient();
	for (IRC_AUTO it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList.push_back("@" + s->second.get_nickname());
	} 
	IRC_LOG("size Client: %zu", clientList.size());
	str	stringClientList;
	for (size_t i = 0; i < clientList.size(); i++)
	{
		if (i != 0)
			stringClientList += " ";
		stringClientList += clientList[i];
	}
	channel->_broadcast(_architect.CMD_JOIN(clientName, channelName.c_str()));
	_send(client, _architect.RPL_TOPIC(clientName.c_str(), channelName.c_str(), topic.c_str()));
	_send(client, _architect.RPL_NAMREPLY(clientName.c_str(), "=", channelName.c_str(), stringClientList.c_str()));
	_send(client, _architect.RPL_ENDOFNAMES(clientName.c_str(), channelName.c_str()));

}

void	Server::_sendTopic(Client &client, Channel *channel)
{
	str					time = "";

	std::stringstream	stream;
	stream << std::setfill('0') << std::setw(3) << channel->get_topicSetTime();
	time += stream.str();

	_send(client, _architect.RPL_TOPIC(client.getTargetName(), channel->getTargetName(), channel->get_topic().c_str()));
	IRC_LOG("%s", client.getTargetName());
	IRC_LOG("%s", channel->getTargetName());
	IRC_LOG("%s", channel->get_topicSetterNickName().c_str());
	IRC_LOG("%s", time.c_str());
	_send(client, _architect.RPL_TOPICWHOTIME(client.getTargetName(), channel->getTargetName(), channel->get_topicSetterNickName().c_str(), time.c_str()));
}

void	Server::_sendModeIs(Client &client, Channel *channel)
{
	str					modeArgs = "";
	std::stringstream	ss;
	const int			userLimit = channel->get_userLimit();
	str					modeis = "+";

	if (channel->get_inviteOnlyChannel())
		modeis += "i";
	if (channel->get_topicPermNeeded())
		modeis += "t";
	if (channel->get_activePassword())
		modeis += "k";
	if (userLimit != INT_MAX)
	{
		ss << userLimit;
		modeis += "l";
		modeArgs += ss.str();
	}
	
	_send(client, _architect.RPL_CHANNELMODEIS(client.getTargetName(), channel->getTargetName(), modeis.c_str(), modeArgs.c_str()));
}

void	Server::_send(Client *client, const str &string)
{
	IRC_LOG("sending reply " BOLD(COLOR(RED,"%s")), string.c_str());
	client->sendMsg(string);
}

void	Channel::_broadcast(const str &string) const
{
	IRC_AUTO	it = _fdClient.begin();

	IRC_LOG("Channel Brodcast " BOLD(COLOR(YELLOW,"%s")) " to %d client", string.c_str(), get_size());
	for (; it != _fdClient.end(); ++it)
		write(*it, (string + "\r\n").c_str(), string.size() + 2);
	it = _fdAdminClient.begin();
	for (; it != _fdAdminClient.end(); ++it)
		write(*it, (string + "\r\n").c_str(), string.size() + 2);
	IRC_LOG("succesfull Broadcast, it suprisely didnt segfault");
}

void	Server::_broadcast(const str &string)
{
	IRC_AUTO	it = _clients.begin();

	IRC_LOG("Server Brodcast " BOLD(COLOR(YELLOW,"%s")), string.c_str());
	for (; it != _clients.end(); ++it)
	{
		write(((*it).second.get_fd()), (string + "\r\n").c_str(), string.size() + 2);
	}
}

void	Client::sendMsg(const str &string) const
{
	write(_fd, (string + "\r\n").c_str(), string.size() + 2);
}

void	Channel::sendMsg(const str &string) const
{
	_broadcast(string);
}
