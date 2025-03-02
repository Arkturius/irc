#pragma once

#include <irc.h>

#include <Server.h>
#include <Client.h>

IRC_COMMAND_DEF(QUIT)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_TRAILING_PARAM);
	if (!_seeker.consume())
		return ;

	const std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() != 1)
		return ;

	if (!_seeker.get_didMatch())
		return ;
	
	_send(client, "ERROR");
	if (client.get_channelMap().size() != 0)
		_broadcast("QUIT Quit: " + argv[0]);
	IRC_FLAG_SET(client.get_flag(), IRC_CLIENT_EOF);
}
