#ifndef IRCARCHITECT_H
# define IRCARCHITECT_H

#include <irc.h>
#include <cstdarg>
#include <sstream>
#include <iomanip>

typedef enum
{
	RPL_CODE_WELCOME			=	1,
	RPL_CODE_YOUHOST			=	2,
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

#define RPL_NOTOPIC(...)			build(RPL_CODE_NOTOPIC, ##__VA_ARGS__)
#define RPL_TOPICWHOTIME(...)		build(RPL_CODE_TOPICWHOTIME, ##__VA_ARGS__)
#define RPL_INVITING(...)			build(RPL_CODE_INVITING, ##__VA_ARGS__)
#define ERR_NOSUCHNICK(...)			build(ERR_CODE_NOSUCHNICK, ##__VA_ARGS__)
#define ERR_NOSUCHCHANNEL(...)		build(ERR_CODE_NOSUCHCHANNEL, ##__VA_ARGS__)
#define ERR_NONICKNAMEGIVEN(...)	build(ERR_CODE_NONICKNAMEGIVEN, ##__VA_ARGS__)
#define ERR_ERRONEUSNICKNAME(...)	build(ERR_CODE_ERRONEUSNICKNAME, ##__VA_ARGS__)
#define ERR_NICKNAMEINUSE(...)		build(ERR_CODE_NICKNAMEINUSE, ##__VA_ARGS__)
#define ERR_USERNOTINCHANNEL(...)	build(ERR_CODE_USERNOTINCHANNEL, ##__VA_ARGS__)
#define ERR_NOTONCHANNEL(...)		build(ERR_CODE_NOTONCHANNEL, ##__VA_ARGS__)
#define ERR_USERONCHANNEL(...)		build(ERR_CODE_USERONCHANNEL, ##__VA_ARGS__)

#define ERR_MSG_NEEDMOREPARAMS		"Not enough parameters"
#define ERR_NEEDMOREPARAMS(...)		build(ERR_CODE_NEEDMOREPARAMS, ##__VA_ARGS__, ERR_MSG_NEEDMOREPARAMS, NULL)

#define ERR_ALREADYREGISTERED(...)	build(ERR_CODE_ALREADYREGISTERED, ##__VA_ARGS__)
#define ERR_PASSWDMISMATCH(...)		build(ERR_CODE_PASSWDMISMATCH, ##__VA_ARGS__)
#define ERR_CHANNELISFULL(...)		build(ERR_CODE_CHANNELISFULL, ##__VA_ARGS__)
#define ERR_UNKNOWNMODE(...)		build(ERR_CODE_UNKNOWNMODE, ##__VA_ARGS__)
#define ERR_INVITEONLYCHAN(...)		build(ERR_CODE_INVITEONLYCHAN, ##__VA_ARGS__)
#define ERR_BADCHANNELKEY(...)		build(ERR_CODE_BADCHANNELKEY, ##__VA_ARGS__)
#define ERR_CHANOPRIVSNEEDED(...)	build(ERR_CODE_CHANOPRIVSNEEDED, ##__VA_ARGS__)
#define ERR_INVALIDMODEPARAM(...)	build(ERR_CODE_INVALIDMODEPARAM, ##__VA_ARGS__)

#include <IRCSeeker.h>

class IRCArchitect
{
	private:
		IRCSeeker	_seeker;

	public:
		IRCArchitect(void) {}
		~IRCArchitect(void) {}

		const str	build(uint32_t code, ...)
		{
			va_list				list;
			char				*curr;
			std::vector<str>	params;
			
			va_start(list, code);
			while (true)
			{
				curr = va_arg(list, char *);
				if (!curr) { break ; }
				params.push_back(str(curr));
			}
			va_end(list);

			std::stringstream	stream;
			str					reply = "";

			stream << std::setfill('0') << std::setw(3) << code;
			reply += stream.str();
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

		const str	build(const str &source, const char *command, ...)
		{
			va_list				list;
			char				*curr;
			std::vector<str>	params;
			
			va_start(list, command);
			while (true)
			{
				curr = va_arg(list, char *);
				if (!curr) { break ; }
				params.push_back(str(curr));
			}
			va_end(list);

			str					reply = ": " + source + command;

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
	
	EXCEPTION(InvalidReplyParameterException, "Invalid reply parameter.");
};

#endif	//	IRCARCHITECT_H
