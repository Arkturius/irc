#pragma once

# include <Server.h>

IRC_COMMAND_DEF(PING)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);

	if (!_seeker.consume())
		return ;

	const std::vector<str>	&argv = _seeker.get_matches();
	_send(client, "PONG " + argv[0]);
}
