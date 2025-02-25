#include <irc.h>

#include <Server.h>
#include <Client.h>

IRC_COMMAND_DEF(PASS)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	const std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() == 0)
		goto needMoreParams;

	if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))
		goto alreadyRegistered;

	client->set_flag(client->get_flag() | IRC_CLIENT_REGISTER);
	client->set_lastPass(argv[0]);
	return ;

needMoreParams:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "PASS"));
alreadyRegistered:
	return _send(client, _architect.ERR_ALREADYREGISTERED(client->get_nickname().c_str()));
}
