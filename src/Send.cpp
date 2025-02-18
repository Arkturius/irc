#include <climits>
#include <poll.h>
#include <sstream>
#include <unistd.h>

#include <Client.h>
#include <time.h>
#include <Channel.h>
#include <Server.h>

void	Server::_sendJoin(Client *client, Channel *channel)
{
	const str &clientName = client->get_nickname();
	const str &channelName = channel->get_name();
	const str &topic = channel->get_topic();

	str clientList = clientName; // prefix
	const std::vector<int> &fdClientList = channel->get_fdClient();
	for (IRC_AUTO it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList += " " + s->second.get_nickname();
	}
	const std::vector<int> &fdAdminClientList = channel->get_fdAdminClient();
	for (IRC_AUTO it = fdAdminClientList.begin(); it != fdAdminClientList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList += " " + s->second.get_nickname();
	}

	_send(client, ":" + clientName + " JOIN " + channelName);
	_send(client, _architect.RPL_TOPIC(3, clientName.c_str(), channelName.c_str(), topic.c_str()));
	_send(client, _architect.RPL_NAMREPLY(3, clientName.c_str(), channelName.c_str(), clientList.c_str()));
	_send(client, _architect.RPL_ENDOFNAMES(2, clientName.c_str(), channelName.c_str()));
	channel->_send(_architect.CMD_JOIN(clientName.c_str(), 1, channelName.c_str()));
}

void	Server::_sendTopic(Client *client, Channel *channel)
{
	_send(client, _architect.RPL_TOPIC(3, client->get_nickname().c_str(), channel->get_name().c_str(), channel->get_topic().c_str()));
	_send(client, _architect.RPL_TOPICWHOTIME(4, client->get_nickname().c_str(), channel->get_name().c_str(), channel->get_topicSetterNickName().c_str(), channel->get_topicSetTime()));
}

void	Server::_sendModeIs(Client *client, Channel *channel)
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
	
	_send(client, _architect.RPL_CHANNELMODEIS(4, client->get_nickname().c_str(), channel->get_name().c_str(), modeis.c_str(), modeArgs.c_str()));
}

void	Server::_send(Client *client, const str &string)
{
	struct pollfd	*pollfd = client->get_pfd();

	IRC_LOG("sending reply " BOLD(COLOR(RED,"%s")), string.c_str());
	write(pollfd->fd, (string + "\r\n").c_str(), string.size() + 2);
}

void	Channel::_send(const str &string)
{
	IRC_AUTO	it = _fdClient.begin();

	IRC_LOG("Channel Brodcast " BOLD(COLOR(YELLOW,"%s")), string.c_str());
	for (; it != _fdClient.end(); ++it)
		write(*it, (string + "\r\n").c_str(), string.size() + 2);
	for (; it != _fdAdminClient.end(); ++it)
		write(*it, (string + "\r\n").c_str(), string.size() + 2);
}

