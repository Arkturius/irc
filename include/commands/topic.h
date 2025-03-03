#pragma once

# include <Server.h>

IRC_COMMAND_DEF(TOPIC)
{
	str		channelName;
	str		topic;
	bool	newTopic = 0;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();
	std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() == 1)
		channelName = argv[0];
	else
		return ;

	_seeker.rebuild(R_TRAILING_PARAM);
	_seeker.findall();
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
	try { perm = channel->havePerm(client.get_fd()); }
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
			return ;
		}
		else
			goto topicNotSet;
	}

setNewTopic:
	channel->set_topicIsSet(topic.size() != 1);
	channel->set_topic(topic.substr(1));
	channel->set_topicSetterNickName(client.get_nickname());
	channel->set_topicSetTime(time(NULL));
	_sendTopic(client, channel);
	channel->_broadcast(_architect.CMD_TOPIC(client.get_nickname(), channelName.c_str(), topic.substr(2).c_str()));
	return ;
errorNotOnChannel:
	_send(client, _architect.ERR_NOTONCHANNEL(client.getTargetName(), channelName.c_str()));
	return ;
errorNoSuchChannel:
	_send(client, _architect.ERR_NOSUCHCHANNEL(client.getTargetName(), channelName.c_str()));
	return ;
errorPerm:
	_send(client, _architect.ERR_CHANOPRIVSNEEDED(client.getTargetName(), channelName.c_str()));
	return ;
topicNotSet:
	_send(client, _architect.RPL_NOTOPIC(client.getTargetName(), channelName.c_str()));
	return ;
}

void	Server::_sendTopic(Client &client, Channel *channel)
{
	str					time = "";

	std::stringstream	stream;
	stream << std::setfill('0') << std::setw(3) << channel->get_topicSetTime();
	time += stream.str();

	_send(client, _architect.RPL_TOPIC(client.getTargetName(), channel->getTargetName(), channel->get_topic().c_str()));
	_send(client, _architect.RPL_TOPICWHOTIME(client.getTargetName(), channel->getTargetName(), channel->get_topicSetterNickName().c_str(), time.c_str()));
}
