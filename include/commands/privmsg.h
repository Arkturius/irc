#pragma once

#include <unistd.h>

#include <irc.h>

#include <Server.h>
#include <Client.h>
#include <Channel.h>

IRC_COMMAND_DEF(PRIVMSG)
{
	str					targetName;
	std::vector<str>	topics;
	std::vector<str>	argv;
	std::vector<str>	targets;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();
	argv = _seeker.get_matches();
	if (argv.size() != 1)
		goto needMoreParam;

	_seeker.rebuild(R_TRAILING_PARAM);
	_seeker.consumeMany();
	topics = _seeker.get_matches();
	if (topics.size() != 1)
		goto noTextToSend;

	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
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
		target->sendMsg("j envoie un msg xd"); //TODO
	}


needMoreParam:
	_send(client, _architect.ERR_NEEDMOREPARAMS(client.get_nickname().c_str(), "PRIVMSG"));
	return ;
noSuchNick:
	_send(client, _architect.ERR_NOSUCHNICK(client.get_nickname().c_str(), targetName.c_str()));
	return ;
noRecipient:
	_send(client, _architect.ERR_NORECIPIENT(client.get_nickname().c_str()));
	return ;
noTextToSend:
	_send(client, _architect.ERR_NOTEXTTOSEND(client.get_nickname().c_str()));
	return ;
}
