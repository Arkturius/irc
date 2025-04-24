#pragma once

#include <irc.h>

#include <Server.h>
#include <Client.h>

IRC_COMMAND_DEF(QUIT)
{
	const std::vector<str>	&argv = _parsingParam(command);

	if (argv.size() > 1)
		return ;

	_send(client, "ERROR");
	if (client.get_channelMap().size() != 0)
		_broadcast("QUIT Quit: " + (argv.size() ? argv[0] : ""));
	IRC_FLAG_SET(client.get_flag(), IRC_CLIENT_EOF);
}
