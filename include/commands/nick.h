#pragma once

# include <Server.h>

IRC_COMMAND_DEF(NICK)
{
	const std::vector<str>	&argv = _parsingParam(command);

	if (argv.size() == 0)
		goto noNicknameGiven;

	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_NICKNAME);

	if (!_seeker.consume() || !argv[0].compare(0, 6, "dealer"))
		goto erroneusNickname;

	for (IRC_AUTO it = _clients.begin(); it != _clients.end(); ++it)
		if (argv[0] == (*it).second.get_nickname())
			goto nicknameInUse;

	if (IRC_FLAG_GET(client.get_flag(), IRC_CLIENT_AUTH))
		client.sendMsg(":" + client.get_nickname() + " NICK " + argv[0]);

	client.set_nickname(argv[0]);
	client.set_targetName(argv[0]);
	if (client.get_username() == "")
		client.set_username(argv[0]);

	return ;

noNicknameGiven:
	return _send(client, _architect.ERR_NONICKNAMEGIVEN(client.get_nickname().c_str()));
erroneusNickname:
	return _send(client, _architect.ERR_ERRONEUSNICKNAME(client.get_nickname().c_str(), argv[0].c_str()));
nicknameInUse:
	return _send(client, _architect.ERR_NICKNAMEINUSE(client.get_nickname().c_str(), argv[0].c_str()));
}

