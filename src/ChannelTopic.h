#ifndef CHANNELTOPIC_H
# define CHANNELTOPIC_H

# include <Server.h>
# include <Client.h>
# include <Channel.h>
# include <poll.h>


void	Server::_topic(const str &command, Client *client)
{
	str		channelName;
	str		topic;
	bool	newTopic;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();
	std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() == 1)
		channelName = argv[0];
	else
		return ;

	_seeker.rebuild(R_TRAILING_PARAM);
	_seeker.consumeMany();
	std::vector<str>	&topics = _seeker.get_matches();
	if (topics.size() == 1)
	{
		topic = topics[0];
		newTopic = 1;
	}
	if (topics.size() > 1)
		return ;
	Channel	*channel = _getChannelByName(channelName);
	int		perm = 0;
	if (!channel)
		goto errorNoSuchChannel;
	try { perm = channel->havePerm(client->get_pfd()->fd); }
	catch (std::exception &e)
	{
		goto errorNotOnChannel;
	}
	if (newTopic)
	{
		if (!perm && channel->get_topicPermNeeded())
			goto errorPerm;
		else
			goto setNewTopic;
	}
	else
	{
		if (channel->get_topicIsSet())
		{
			str	topic = channel->get_topic();
			_sendTopic(client, channel);
		}
		else
			goto topicNotSet;
	}

setNewTopic:
	channel->set_topicIsSet(topic.size() != 1);
	channel->set_topic(topic.substr(1));
	channel->set_topicSetterNickName(client->get_nickname());
	channel->set_topicSetTime(time(NULL));
	_sendTopic(client, channel);
	channel->_broadcast(_architect.CMD_TOPIC(client->getTargetName(), 1, channelName.c_str()));
	return ;
errorNotOnChannel:
	_send(client, _architect.ERR_NOTONCHANNEL(client->getTargetName(), channelName.c_str()));
	return ;
errorNoSuchChannel:
	_send(client, _architect.ERR_NOSUCHCHANNEL(client->getTargetName(), channelName.c_str()));
	return ;
errorPerm:
	_send(client, _architect.ERR_CHANOPRIVSNEEDED(client->getTargetName(), channelName.c_str()));
	return ;
topicNotSet:
	_send(client, _architect.RPL_NOTOPIC(client->getTargetName(), channelName.c_str()));
	return ;
}



#endif
