#pragma once

#include <Server.h>

IRC_COMMAND_DEF(USER)
{
	const std::vector<str>	&argv = _parsingParam(command);

	if (argv.size() != 4)
		goto needMoreParams;

	if (argv[1] != "0" || argv[2] != "*")
		return ;

	if (IRC_FLAG_GET(client.get_flag(), IRC_CLIENT_AUTH))
		goto alreadyRegistered;

	client.set_username(argv[0]);
	_registerClient(client);
	return ;

needMoreParams:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.get_nickname().c_str(), "USER"));
alreadyRegistered:
	return _send(client, _architect.ERR_ALREADYREGISTERED(client.get_nickname().c_str()));
}
