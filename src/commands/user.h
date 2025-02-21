#include <irc.h>

#include <Server.h>
#include <Client.h>

IRC_COMMAND_DEF(USER)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() != 3)									{ goto needMoreParams; }
	if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))	{ goto alreadyRegistered; }

	client->set_username(argv[0]);
	_registerClient(client);
	return ;

needMoreParams:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "USER"));
alreadyRegistered:
	return _send(client, _architect.ERR_ALREADYREGISTERED(client->get_nickname().c_str()));
}
