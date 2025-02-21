#include <irc.h>

#include <Server.h>
#include <Client.h>

#define IRC_CAN_PONG	(IRC_CLIENT_PINGED | IRC_CLIENT_REGISTER)

IRC_COMMAND_DEF(PONG)
{
	if ((client->get_flag() & IRC_CAN_PONG) != IRC_CAN_PONG) { return ; }

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consume();
	const std::vector<str>	&argv = _seeker.get_matches();

	if (!_seeker.get_didMatch())	{ return ; }
	if (argv.size() != 1)			{ return ; }
	if (argv[0] != "ft_irc")		{ return ; }
	
	client->set_flag(client->get_flag() & ~(IRC_CLIENT_PINGED));
	client->set_flag(client->get_flag() & ~(IRC_CLIENT_REGISTER));
	client->set_flag(client->get_flag() | IRC_CLIENT_AUTH);

	_welcomeClient(client);
}
