#pragma once

# include <Server.h>

IRC_COMMAND_DEF(INVITE)
{
	const std::vector<str>	&param = _parsingParam(command);

	if (param.size() < 2)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "INVITE"));
	if (param.size() > 2)
	{
		IRC_WARN("to many params");
		return ;
	}
	const str	&nickName = param[0];
	const str	&channelName = param[1];

	Channel *channel = _getChannelByName(channelName);
	Client	*target = _getClientByName(nickName);
	bool			perm = 0;

	if (!channel)
		goto errorNoSuchChannel;
	if (!target)
		goto errorNoSuchNick;
	try { perm = channel->havePerm(client.get_fd()); }
	catch (std::exception &e)
	{
		goto errorYouAreNotOnChannel;
	}
	if (!perm)
	{
		if (channelName == str("#") + client.get_nickname() + str("_table"))
			IRC_LOG("your not +o; but its still your table");
		else if (IRC_FLAG_GET(channel->get_flag(), IRC_CHANNEL_INVITE_ONLY))
			goto errorNoPerm;
	}
	try
	{
		channel->havePerm(target->get_fd());
		goto errorUserOnChannel;
	}
	IRC_CATCH

	_send(client, _architect.RPL_INVITING(client.getTargetName(), target->getTargetName(), channelName.c_str()));
	channel->invite(target->get_fd());
	return _send(*target, client.get_nickname() + " invited you to channel " + channelName);

errorUserOnChannel:
	return _send(client, _architect.ERR_USERONCHANNEL(client.getTargetName(), target->getTargetName(), channelName.c_str()));

errorNoPerm:
	return _send(client, _architect.ERR_CHANOPRIVSNEEDED(client.getTargetName(), channelName.c_str()));

errorYouAreNotOnChannel:
	return _send(client, _architect.ERR_NOTONCHANNEL(client.getTargetName(), channelName.c_str()));

errorNoSuchNick:
	return _send(client, _architect.ERR_NOSUCHNICK(client.getTargetName(), nickName.c_str()));

errorNoSuchChannel:
	return _send(client, _architect.ERR_NOSUCHCHANNEL(client.getTargetName(), channelName.c_str()));
}
