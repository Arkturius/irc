#include <poll.h>
#include <unistd.h>

#include <Client.h>
#include <Channel.h>
#include <Server.h>

void	Server::_send_join(Client *client, Channel *channel)
{
	str	clientName = client->get_nickname();
	str channelName = channel->get_name();
	str topic = channel->get_topic();

	str clientList = clientName; // prefix
	std::vector<int> fdClientList;
	fdClientList = channel->get_fdClient();
	for (IRC_AUTO it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList += " " + s->second.get_nickname();
	}
	fdClientList = channel->get_fdAdminClient();
	for (IRC_AUTO it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList += " " + s->second.get_nickname();
	}
	IRCArchitect	rpl;

	_send(client, ":" + clientName + " JOIN " + channelName);
	_send(client, rpl.RPL_TOPIC("", 3, clientName.c_str(), channelName.c_str(), topic.c_str()));
	_send(client, rpl.RPL_NAMREPLY("", 3, clientName.c_str(), channelName.c_str(), clientList.c_str()));
	_send(client, rpl.RPL_ENDOFNAMES("", 2, clientName.c_str(), channelName.c_str()));
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
