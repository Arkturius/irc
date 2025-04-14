#pragma once

# include <Server.h>

IRC_COMMAND_DEF(PING)
{
	const std::vector<str>	&argv = _parsingParam(command);

	if (argv.size() == 0)
		goto needMoreParams;
	if (argv.size() != 1)
	{
		IRC_WARN("PING: too many params.");
		return ;
	}

	_send(client, "PONG " + argv[0]);
	return ;

needMoreParams:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.get_nickname().c_str()));
}

