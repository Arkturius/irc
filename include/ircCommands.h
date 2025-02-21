#include <irc.h>

#include <Server.h>
#include <Client.h>
#include <ircRegex.h>

IRC_COMMAND_DEF(PASS)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() == 0)									{ goto needMoreParams; }
	if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))	{ goto alreadyRegistered; }

	client->set_flag(client->get_flag() | IRC_CLIENT_REGISTER);
	client->set_lastPass(argv[0]);
	return ;

needMoreParams:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "PASS"));
alreadyRegistered:
	return _send(client, _architect.ERR_ALREADYREGISTERED(client->get_nickname().c_str(), "You may not reregister"));
}

IRC_COMMAND_DEF(USER)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() != 3)
	{
		_send(client, 
		_architect.ERR_NEEDMOREPARAMS
		(
			3,
			client->get_nickname().c_str(),
			"USER",
			"Not enough parameters"
		));
		return ;
	}

	if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))
	{
		_send(client, 
		_architect.ERR_ALREADYREGISTERED
		(
			2,
			client->get_nickname().c_str(),
			"You may not reregister"
		));
		return ;
	}

	const str	&newUser = argv[0];

	_seeker.feedString(newUser);
	_seeker.rebuild(R_USERNAME);
	if (!_seeker.consume())
	{
		_send(client,
		_architect.ERR_NEEDMOREPARAMS
		(
			3,
			client->get_nickname().c_str(),
			"USER",
			"Not enough parameters"
		));
		return ;
	}
	client->set_username(newUser);
	_registerClient(client);
}

IRC_COMMAND_DEF(QUIT)
{
	UNUSED(command);
	UNUSED(client);
	client->disconnect();
}

#define IRC_CAN_PONG	(IRC_CLIENT_PINGED | IRC_CLIENT_REGISTER)

IRC_COMMAND_DEF(PONG)
{
	if ((client->get_flag() & IRC_CAN_PONG) != IRC_CAN_PONG)
		return ;
	client->set_flag(client->get_flag() & ~(IRC_CLIENT_PINGED));

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	if (!_seeker.consume())
		return ;

	IRC_LOG("parameter found");

	const std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() != 1)
		return ;
	
	IRC_LOG("1 argument");

	if (argv[0] != "ft_irc")
		return ;

	IRC_LOG("good token");
	
	client->set_flag(client->get_flag() | IRC_CLIENT_AUTH);
	client->set_flag(client->get_flag() & ~(IRC_CLIENT_REGISTER));

	_welcomeClient();
}
