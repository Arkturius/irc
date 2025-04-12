#pragma once

#include <irc.h>

#include <Server.h>
#include <Client.h>
#include <Channel.h>
#include <unistd.h>

IRC_COMMAND_DEF(PRIVMSG)
{
	str					targetName;
	str					msg;
	std::vector<str>	targets;
	const std::vector<str>	param = _parsingParam(command);

	if (param.size() == 0)
		goto needMoreParam;
	if (param.size() == 1)
		goto noTextToSend;
	if (param.size() > 2)
	{
		IRC_WARN("too many params");
		return ;
	}

	if (param[1].at(0) == ':')
		msg = param[1].substr(1);


	_seeker.feedString(param[0]);
	_seeker.rebuild(R_CAPTURE_TARGET_NAME);
	_seeker.findall();
	targets = _seeker.get_matches();
	if (targets.size() == 0)
		goto noRecipient;

	for (IRC_AUTO it = targets.begin(); it != targets.end(); ++it)
	{
		ATarget	*target;
		str		targetName = *it;

		target = _getTargetByName(targetName);
		if (!target)
			goto noSuchNick;
		
		if (target->get_targetIsChannel())
			target->ignoredFlag(client.get_fd(), IRC_CHANNEL_IGNORED);

		target->sendMsg(_architect.CMD_PRIVMSG(client.get_nickname(), target->getTargetName(), msg.c_str()));

		if (target->get_targetIsChannel())
			target->ignoredFlag(client.get_fd(), !IRC_CHANNEL_IGNORED);
	}
	return ;

needMoreParam:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "PRIVMSG"));
noSuchNick:
	return _send(client, _architect.ERR_NOSUCHNICK(client.getTargetName(), targetName.c_str()));
noRecipient:
	return _send(client, _architect.ERR_NORECIPIENT(client.getTargetName()));
noTextToSend:
	return _send(client, _architect.ERR_NOTEXTTOSEND(client.getTargetName()));
}
