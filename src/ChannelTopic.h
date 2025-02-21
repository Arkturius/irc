#ifndef CHANNELTOPIC_H
# define CHANNELTOPIC_H

# include <Server.h>
# include <Client.h>
# include <Channel.h>
# include <poll.h>


void	Server::_topic(const str command, Client *client)
{
	//TOPIC channel => print (can be not set (erreur))
	//TOPIC channel : => clear 
	//TOPIC channel : topicContent => rempli

	//TODO parsing
	(void)command;(void)client;
	str		channelName;
	Channel	*channel = _getChannelByName(channelName);
	int		perm = 0;
	if (!channel)
		return _send(client, _architect.ERR_NOSUCHCHANNEL(3, client->get_nickname().c_str(), channelName.c_str(), "No such channel"));
	try { perm = channel->havePerm(client->get_pfd()->fd); }
	catch (std::exception &e)
	{
		return _send(client, _architect.ERR_NOTONCHANNEL(3, client->get_nickname().c_str(), channelName.c_str(), "You're not on that channel"));
	}
	bool	newTopic = 1;
	if (newTopic)
	{
		str	topic;
		
		if (!perm && channel->get_topicPermNeeded())
			return _send(client, _architect.ERR_CHANOPRIVSNEEDED(3, client->get_nickname().c_str(), channelName.c_str(), "You're not channel operator"));
		else
		{
			channel->set_topic(topic);
			channel->set_topicIsSet(1);
			channel->set_topicSetterNickName(client->get_nickname());
			channel->set_topicSetTime(time(NULL));
			_sendTopic(client, channel);
			channel->_send(_architect.CMD_TOPIC(client->get_nickname().c_str(), 1, channelName.c_str()));
		}
	}
	else
	{
		if (channel->get_topicIsSet())
		{
			str	topic = channel->get_topic();
			_sendTopic(client, channel);
		}
		else
			return _send(client, _architect.RPL_NOTOPIC(3, client->get_nickname().c_str(), channelName.c_str(), "No topic is set"));
	}
}



#endif
