#include <irc.h>

#include <Server.h>
#include <Client.h>

#ifndef R_SPACE
#define R_SPACE				" "
#endif
#ifndef R_NOCRLF
#define R_NOCRLF			R_CHAR_INV_GROUP("\r\n")
#endif
#ifndef R_NOSPCRLFCL
#define R_NOSPCRLFCL		R_CHAR_INV_GROUP("\r\n :")
#endif

#ifndef R_MIDDLE
#define R_MIDDLE			R_NOSPCRLFCL R_0_OR_MORE(R_CAPTURE(":|" R_NOSPCRLFCL))
#endif
#ifndef R_MIDDLE_PARAM
#define R_MIDDLE_PARAM		R_SPACE R_0_OR_1(R_CAPTURE(R_MIDDLE))
#endif

#ifndef R_TRAILING
#define R_TRAILING			R_0_OR_MORE(R_NOCRLF)
#endif
#ifndef R_TRAILING_PARAM
#define R_TRAILING_PARAM	R_SPACE	R_0_OR_1(R_CAPTURE(R_TRAILING))
#endif

#ifndef ARCHITECT
#define ARCHITECT(c)		
#endif

#ifndef NICKNAME_CHAR
#define NICKNAME_CHAR		" ,\\*\\?!@#"
#endif
#ifndef NICKNAME_START
#define NICKNAME_START		NICKNAME_CHAR ":$"
#endif

#ifndef R_NICKNAME
#define R_NICKNAME			R_FULL_MATCH											\
							(														\
								R_CAPTURE											\
								(													\
									R_CHAR_INV_GROUP(NICKNAME_START)				\
									R_X_TO_Y(R_CHAR_INV_GROUP(NICKNAME_CHAR),0,8)	\
								)													\
							)
#endif

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
