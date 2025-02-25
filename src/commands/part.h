#ifndef PART_H
# define PART_H

# include <Server.h>
# include <Channel.h>
#include <exception>

IRC_COMMAND_DEF(PART)
{
	int					fd = client->get_pfd()->fd;
	str					targetName;
	str					PartReason;
	std::vector<str>	trailing;
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
	trailing = _seeker.get_matches();
	if (trailing.size() == 0)
		PartReason = "";
	else
		PartReason = trailing[0];

	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	targets = _seeker.get_matches();
	if (targets.size() == 0)
		goto needMoreParam;

	for (IRC_AUTO it = targets.begin(); it != targets.end(); ++it)
	{
		str				targetName = *it;
		IRC_AUTO		s = _channelMap.find(targetName);
		Channel			*target = NULL;

		if (s == _channelMap.end())
			goto noSuchChannel;
		target = s->second;
		try {target->havePerm(client->get_pfd()->fd);}
		catch (std::exception &e) { goto notOnChannel; }

		target->_broadcast(_architect.CMD_PART());
		int size = target->removeClient(fd);
		if (size == 0)
			_channelMap.erase(s);
		client->leaveChannel(target);
	}


needMoreParam:
	_send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "PRIVMSG"));
	return ;
notOnChannel:
	_send(client, _architect.ERR_NOTONCHANNEL(client->get_nickname().c_str(), targetName.c_str()));
	return ;
noSuchChannel:
	_send(client, _architect.ERR_NOSUCHCHANNEL(client->get_nickname().c_str(), targetName.c_str()));
	return ;
}

#endif
