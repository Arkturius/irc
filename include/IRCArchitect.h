#ifndef IRCARCHITECT_H
# define IRCARCHITECT_H

#include <irc.h>
#include <cstdarg>
#include <sstream>
#include <iomanip>

typedef enum
{
	RPL_CODE_WELCOME			=	1,
	RPL_CODE_YOURHOST			=	2,
	RPL_CODE_CREATED			=	3,
	RPL_CODE_MYINFO				=	4,
	RPL_CODE_ISUPPORT			=	5,
	RPL_CODE_CHANNELMODEIS		=	324,
	RPL_CODE_NOTOPIC			=	331,
	RPL_CODE_TOPIC				=	332,
	RPL_CODE_TOPICWHOTIME		=	333,
	RPL_CODE_INVITING			=	341,
	RPL_CODE_NAMREPLY			=	353,
	RPL_CODE_ENDOFNAMES			=	366,

	RPL_CODE_MOTD				=	371,
	RPL_CODE_MOTDSTART			=	375,
	RPL_CODE_ENDOFMOTD			=	376,

	ERR_CODE_NOSUCHNICK			=	401,
	ERR_CODE_NOSUCHCHANNEL		=	403,
	ERR_CODE_NONICKNAMEGIVEN	=	431,
	ERR_CODE_ERRONEUSNICKNAME	=	432,
	ERR_CODE_NICKNAMEINUSE		=	433,
	ERR_CODE_USERNOTINCHANNEL	=	441,
	ERR_CODE_NOTONCHANNEL		=	442,
	ERR_CODE_USERONCHANNEL		=	443,
	ERR_CODE_NEEDMOREPARAMS		=	461,
	ERR_CODE_ALREADYREGISTERED	=	462,
	ERR_CODE_PASSWDMISMATCH		=	463,
	ERR_CODE_CHANNELISFULL		=	471,
	ERR_CODE_UNKNOWNMODE		=	472,
	ERR_CODE_INVITEONLYCHAN		=	473,
	ERR_CODE_BADCHANNELKEY		=	475,
	ERR_CODE_CHANOPRIVSNEEDED	=	482,
	ERR_CODE_INVALIDMODEPARAM	=	696,

	RPL_CODE_LAST				=	909
}	IRCReplyCode;

#define CMD_TOPIC(source, ...)		build(source, " TOPIC ", ##__VA_ARGS__)
#define CMD_JOIN(source, ...)		build(source, " JOIN ", ##__VA_ARGS__)
#define CMD_MODE(source, ...)		build(source, " MODE ", ##__VA_ARGS__)
#define CMD_KICK(source, ...)		build(source, " KICK ", ##__VA_ARGS__)

#define RPL_MSG_WELCOME				"Welcome to the ft_irc server, "
#define RPL_WELCOME(...)			build(RPL_CODE_WELCOME, ##__VA_ARGS__, NULL)

#define	RPL_MSG_YOURHOST			"Your host is ft_irc server. running version 0.0"
#define RPL_YOURHOST(...)			build(RPL_CODE_YOURHOST, ##__VA_ARGS__, RPL_MSG_YOURHOST, NULL)

#define	RPL_MSG_CREATED				"This server was created "
#define	RPL_CREATED(...)			build(RPL_CODE_CREATED, ##__VA_ARGS__, NULL)

#define RPL_MSG_MYINFO				""
#define RPL_MYINFO(...)				build(RPL_CODE_MYINFO, ##__VA_ARGS__, NULL)

#define RPL_MSG_ISUPPORT			"are supported by this server"
#define RPL_ISUPPORT(...)			build(RPL_CODE_ISUPPORT, ##__VA_ARGS__, RPL_MSG_ISUPPORT, NULL)

#define RPL_MSG_CHANNELMODEIS		""
#define RPL_CHANNELMODEIS(...)		build(RPL_CODE_CHANNELMODEIS, ##__VA_ARGS__, RPL_MSG_CHANNELMODEIS, NULL)

#define RPL_MSG_TOPIC				""
#define RPL_TOPIC(...)				build(RPL_CODE_TOPIC, ##__VA_ARGS__, RPL_MSG_TOPIC, NULL)

#define RPL_MSG_NOTOPIC				"No topic is set"
#define RPL_NOTOPIC(...)			build(RPL_CODE_NOTOPIC, ##__VA_ARGS__, RPL_MSG_NOTOPIC, NULL)

#define RPL_MSG_TOPICWHOTIME		""
#define RPL_TOPICWHOTIME(...)		build(RPL_CODE_TOPICWHOTIME, ##__VA_ARGS__, RPL_MSG_TOPICWHOTIME, NULL)

#define RPL_MSG_INVITING			""
#define RPL_INVITING(...)			build(RPL_CODE_INVITING, ##__VA_ARGS__, RPL_MSG_INVITING, NULL)

#define RPL_MSG_NAMREPLY			""
#define RPL_NAMREPLY(...)			build(RPL_CODE_NAMREPLY, ##__VA_ARGS__, RPL_MSG_NAMREPLY, NULL)
	
#define RPL_MSG_ENDOFNAMES			""
#define RPL_ENDOFNAMES(...)			build(RPL_CODE_ENDOFNAMES, ##__VA_ARGS__, RPL_MSG_ENDOFNAMES, NULL)

#define RPL_MSG_MOTD				"hello bro."
#define RPL_MOTD(...)				build(RPL_CODE_MOTD, ##__VA_ARGS__, RPL_MSG_MOTD, NULL)

#define RPL_MSG_MOTDSTART			"- ft_irc Message of the day -"
#define RPL_MOTDSTART(...)			build(RPL_CODE_MOTDSTART, ##__VA_ARGS__, RPL_MSG_MOTDSTART, NULL)

#define RPL_MSG_ENDOFMOTD			"End of /MOTD command."
#define RPL_ENDOFMOTD(...)			build(RPL_CODE_ENDOFMOTD, ##__VA_ARGS__, RPL_MSG_ENDOFMOTD, NULL)

#define ERR_MSG_NOSUCHNICK			"No such nick/channel"
#define ERR_NOSUCHNICK(...)			build(ERR_CODE_NOSUCHNICK, ##__VA_ARGS__, ERR_MSG_NOSUCHNICK, NULL)

#define ERR_MSG_NOSUCHCHANNEL		"No such Channel"
#define ERR_NOSUCHCHANNEL(...)		build(ERR_CODE_NOSUCHCHANNEL, ##__VA_ARGS__, ERR_MSG_NOSUCHCHANNEL, NULL)

#define ERR_MSG_NONICKNAMEGIVEN		"No nickname given"
#define ERR_NONICKNAMEGIVEN(...)	build(ERR_CODE_NONICKNAMEGIVEN, ##__VA_ARGS__, ERR_MSG_NONICKNAMEGIVEN, NULL)

#define ERR_MSG_ERRONEUSNICKNAME	"Erroneus nickname"
#define ERR_ERRONEUSNICKNAME(...)	build(ERR_CODE_ERRONEUSNICKNAME, ##__VA_ARGS__, ERR_MSG_ERRONEUSNICKNAME, NULL)

#define ERR_MSG_NICKNAMEINUSE		"Nickname is already in use"
#define ERR_NICKNAMEINUSE(...)		build(ERR_CODE_NICKNAMEINUSE, ##__VA_ARGS__, ERR_MSG_NICKNAMEINUSE, NULL)

#define ERR_MSG_USERNOTINCHANNEL	"They aren't on that channel"
#define ERR_USERNOTINCHANNEL(...)	build(ERR_CODE_USERNOTINCHANNEL, ##__VA_ARGS__, ERR_MSG_USERNOTINCHANNEL, NULL)

#define ERR_MSG_NOTONCHANNEL		"You're not on that channel"
#define ERR_NOTONCHANNEL(...)		build(ERR_CODE_NOTONCHANNEL, ##__VA_ARGS__, ERR_MSG_NOTONCHANNEL, NULL)

#define ERR_MSG_USERONCHANNEL		"is already on channel"
#define ERR_USERONCHANNEL(...)		build(ERR_CODE_USERONCHANNEL, ##__VA_ARGS__, ERR_MSG_USERONCHANNEL, NULL)

#define ERR_MSG_NEEDMOREPARAMS		"Not enough parameters"
#define ERR_NEEDMOREPARAMS(...)		build(ERR_CODE_NEEDMOREPARAMS, ##__VA_ARGS__, ERR_MSG_NEEDMOREPARAMS, NULL)

#define ERR_MSG_ALREADYREGISTERED	"You may not reregister"
#define ERR_ALREADYREGISTERED(...)	build(ERR_CODE_ALREADYREGISTERED, ##__VA_ARGS__, ERR_MSG_ALREADYREGISTERED, NULL)

#define ERR_MSG_PASSWDMISMATCH		"Password incorrect"
#define ERR_PASSWDMISMATCH(...)		build(ERR_CODE_PASSWDMISMATCH, ##__VA_ARGS__, ERR_MSG_PASSWDMISMATCH, NULL)

#define ERR_MSG_CHANNELISFULL		"Cannot join channel (+l)"
#define ERR_CHANNELISFULL(...)		build(ERR_CODE_CHANNELISFULL, ##__VA_ARGS__, ERR_MSG_CHANNELISFULL, NULL)

#define ERR_MSG_UNKNOWNMODE			"is unknown mode char to me"
#define ERR_UNKNOWNMODE(...)		build(ERR_CODE_UNKNOWNMODE, ##__VA_ARGS__, ERR_MSG_UNKNOWNMODE, NULL)

#define ERR_MSG_INVITEONLYCHAN		"Cannot join channel (+i)"
#define ERR_INVITEONLYCHAN(...)		build(ERR_CODE_INVITEONLYCHAN, ##__VA_ARGS__, ERR_MSG_INVITEONLYCHAN, NULL)

#define ERR_MSG_BADCHANNELKEY		"Cannot join channel (+k)"
#define ERR_BADCHANNELKEY(...)		build(ERR_CODE_BADCHANNELKEY, ##__VA_ARGS__, ERR_MSG_BADCHANNELKEY, NULL)

#define ERR_MSG_CHANOPRIVSNEEDED	"You're not channel operator"
#define ERR_CHANOPRIVSNEEDED(...)	build(ERR_CODE_CHANOPRIVSNEEDED, ##__VA_ARGS__, ERR_MSG_CHANOPRIVSNEEDED, NULL)

#define ERR_MSG_INVALIDMODEPARAM	""
#define ERR_INVALIDMODEPARAM(...)	build(ERR_CODE_INVALIDMODEPARAM, ##__VA_ARGS__, ERR_MSG_INVALIDMODEPARAM, NULL)


#include <IRCSeeker.h>

class IRCArchitect
{
	private:
		IRCSeeker	_seeker;

		str			_upgradeParams(str reply, va_list &list)
		{
			char				*curr;
			std::vector<str>	params;

			while (true)
			{
				curr = va_arg(list, char *);
				if (!curr) { break ; }
				params.push_back(str(curr));
			}
			va_end(list);

			for (size_t i = 0; i < params.size(); ++i)
			{
				const str	param = params[i];

				if (param.empty())	{ continue ; }

				reply += " ";
				if (i != params.size() - 1)
				{
					_seeker.feedString(param);
					_seeker.rebuild(R_CAPTURE(R_CHAR_INV_GROUP(" ") "*"));
					if (!_seeker.match())
						throw InvalidReplyParameterException();
				}
				else { reply += ":"; }

				reply += param;
			}
			return (reply);
		}

	public:
		IRCArchitect(void) {}
		~IRCArchitect(void) {}

		const str	build(uint32_t code, ...)
		{
			str					reply = "";
			va_list				list;
			
			std::stringstream	stream;
			stream << std::setfill('0') << std::setw(3) << code;
			reply += stream.str();

			va_start(list, code);
			try { reply = _upgradeParams(reply, list); } IRC_CATCH

			return (reply);
		}

		const str	build(const str &source, const char *command, ...)
		{
			str					reply = ":" + source + command;
			va_list				list;
			
			va_start(list, command);
			try { reply = _upgradeParams(reply, list); } IRC_CATCH

			return (reply);
		}
	
	EXCEPTION(InvalidReplyParameterException, "Invalid reply parameter.");
};

#endif	//	IRCARCHITECT_H
