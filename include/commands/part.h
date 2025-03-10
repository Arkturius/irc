#pragma once

# include <Server.h>
# include <Channel.h>
#include <exception>

IRC_COMMAND_DEF(PART)
{
	int					fd = client.get_fd();
	str					targetName;
	str					PartReason;
	std::vector<str>	trailing;
	std::vector<str>	argv;
	std::vector<str>	targets;

	IRC_ERR("command PART:  <%s>", command.c_str());
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
		try {target->havePerm(fd);}
		catch (std::exception &e) { goto notOnChannel; }

		target->sendMsg(_architect.CMD_PART(client.get_nickname(), target->getTargetName()));
		int size = target->removeClient(fd);
		if (size == 0)
			_channelMap.erase(s);
		client.leaveChannel(target);
	}
	return ;

needMoreParam:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "PART"));
notOnChannel:
	return _send(client, _architect.ERR_NOTONCHANNEL(client.getTargetName(), targetName.c_str()));
noSuchChannel:
	return _send(client, _architect.ERR_NOSUCHCHANNEL(client.getTargetName(), targetName.c_str()));
}
