#pragma once

# include <Server.h>

IRC_COMMAND_DEF(PING)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();

	std::vector<str>	argv = _seeker.get_matches();

	_seeker.rebuild(R_TRAILING_PARAM);
	_seeker.consume();

	const std::vector<str>	&trail = _seeker.get_matches();

	IRC_WARN("ARGV size before insert = %lu", argv.size());
	for (IRC_AUTO it = argv.begin(); it != argv.end(); ++it)
	{
		IRC_WARN("arg = %s", (*it).c_str());
	}

	argv.insert(argv.end(), trail.begin(), trail.end());

	if (argv.size() == 0)
		goto needMoreParams;
	
	if (argv.size() != 1)
		return ;

	_send(client, "PONG " + argv[0]);
	return ;

needMoreParams:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.get_nickname().c_str()));
}
