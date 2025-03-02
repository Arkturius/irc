#pragma once

#include <Server.h>

#define IRC_CAN_PONG	(IRC_CLIENT_PINGED | IRC_CLIENT_REGISTER)

IRC_COMMAND_DEF(PONG)
{
	if ((client.get_flag() & IRC_CAN_PONG) != IRC_CAN_PONG)
		return ;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);

	if (!_seeker.consume())
		return ;

	const std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() != 1 || argv[0] != "ft_irc")
		return ;

	client.set_flag(client.get_flag() ^ (IRC_CLIENT_PINGED | IRC_CLIENT_REGISTER | IRC_CLIENT_AUTH));
	_welcomeClient(client);
}
