#include <Client.h>
#include <Channel.h>
#include <poll.h>
#include <unistd.h>
#include <Server.h>
#include <RplGenerator.h>

void	Server::_send_join(Client *client, Channel *channel)
{
	str	clientName = client->get_nickname();
	str channelName = channel->get_name();
	str topic = channel->get_topic();

	str clientList = clientName; // prefix
	std::vector<int> fdClientList;
	fdClientList = channel->get_fdClient();
	for (auto it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		auto s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList += " " + s->second.get_nickname();
	}
	fdClientList = channel->get_fdAdminClient();
	for (auto it = fdClientList.begin(); it != fdClientList.end(); ++it)
	{
		int fdClient = *it;
		auto s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList += " " + s->second.get_nickname();
	}
	RplGenerator	rpl(clientName, clientName, topic, clientList);

	rpl.rpl_join();
	rpl.rpl_topic();
	rpl.rpl_namreply();
	rpl.rpl_endofnames();
}

void	Server::_send(Client *client, const str &string)
{
	struct pollfd	*pollfd = client->get_pfd();

	write(pollfd->fd, (string + "\r\n").c_str(), string.size() + 2);
}
