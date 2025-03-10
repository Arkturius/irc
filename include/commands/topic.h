#pragma once

# include <Server.h>

IRC_COMMAND_DEF(TOPIC)
{
	const std::vector<str>	&param = _parsingParam(command);

	str		channelName;
	str		topic;

	if (param.size() < 1)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "TOPIC"));
	if (param.size() > 2)
	{
		IRC_WARN("to many params");
		return ;
	}
	channelName = param[0];
	if (param.size() == 2)
		topic = param[1];

	Channel	*channel = _getChannelByName(channelName);
	int		perm = 0;

	if (!channel)
		goto errorNoSuchChannel;
	try { perm = channel->havePerm(client.get_fd()); }
	catch (std::exception &e)
	{
		goto errorNotOnChannel;
	}
	if (param.size() == 2)
	{
		if (!perm && IRC_FLAG_GET(channel->get_flag(), IRC_CHANNEL_TOPIC_PERM))
			goto errorPerm;
		else
			goto setNewTopic;
	}
	else
	{
		if (IRC_FLAG_GET(channel->get_flag(), IRC_CHANNEL_TOPIC_SET))
		{
			topic = channel->get_topic();
			_sendTopic(client, channel);
			return ;
		}
		else
			goto topicNotSet;
	}

setNewTopic:
	IRC_FLAG_SET(channel->get_flag(), IRC_CHANNEL_TOPIC_SET);
	if (topic.size() == 1)
		IRC_FLAG_DEL(channel->get_flag(), IRC_CHANNEL_TOPIC_SET);
	channel->set_topic(topic.substr(2));
	channel->set_topicSetterNickName(client.get_nickname());
	channel->set_topicSetTime(time(NULL));
	_sendTopic(client, channel);
	channel->sendMsg(_architect.CMD_TOPIC(client.get_nickname(), channelName.c_str(), channel->get_topic().c_str()));
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
