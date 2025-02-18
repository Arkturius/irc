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

#define CMD_TOPIC(source, ...)			generate(source, " TOPIC ", ##__VA_ARGS__)
#define CMD_JOIN(source, ...)			generate(source, " JOIN ", ##__VA_ARGS__)
#define CMD_MODE(source, ...)			generate(source, " MODE ", ##__VA_ARGS__)
#define CMD_KICK(source, ...)			generate(source, " KICK ", ##__VA_ARGS__)

#define	RPL_WELCOME(...)				generate(RPL_CODE_WELCOME, ##__VA_ARGS__)		
#define	RPL_YOUHOST(...)				generate(RPL_CODE_YOUHOST, ##__VA_ARGS__)		
#define	RPL_CREATED(...)				generate(RPL_CODE_CREATED, ##__VA_ARGS__)		
#define	RPL_MYINFO(...)					generate(RPL_CODE_MYINFO, ##__VA_ARGS__)		
#define	RPL_ISUPPORT(...)				generate(RPL_CODE_ISUPPORT, ##__VA_ARGS__)	
#define RPL_CHANNELMODEIS(...)			generate(RPL_CODE_CHANNELMODEIS, ##__VA_ARGS__)
#define	RPL_TOPIC(...)					generate(RPL_CODE_TOPIC, ##__VA_ARGS__)		
#define	RPL_NAMREPLY(...)				generate(RPL_CODE_NAMREPLY, ##__VA_ARGS__)	
#define	RPL_ENDOFNAMES(...)				generate(RPL_CODE_ENDOFNAMES, ##__VA_ARGS__)

#define RPL_NOTOPIC(...)				generate(RPL_CODE_NOTOPIC, ##__VA_ARGS__)
#define RPL_TOPICWHOTIME(...)			generate(RPL_CODE_TOPICWHOTIME, ##__VA_ARGS__)
#define RPL_INVITING(...)				generate(RPL_CODE_INVITING, ##__VA_ARGS__)
#define ERR_NOSUCHNICK(...)				generate(ERR_CODE_NOSUCHNICK, ##__VA_ARGS__)
#define ERR_NOSUCHCHANNEL(...)			generate(ERR_CODE_NOSUCHCHANNEL, ##__VA_ARGS__)
#define ERR_USERNOTINCHANNEL(...)		generate(ERR_CODE_USERNOTINCHANNEL, ##__VA_ARGS__)
#define ERR_NOTONCHANNEL(...)			generate(ERR_CODE_NOTONCHANNEL, ##__VA_ARGS__)
#define ERR_USERONCHANNEL(...)			generate(ERR_CODE_USERONCHANNEL, ##__VA_ARGS__)
#define ERR_NEEDMOREPARAMS(...)			generate(ERR_CODE_NEEDMOREPARAMS, ##__VA_ARGS__)
#define ERR_ALREADYREGISTERED(...)		generate(ERR_CODE_ALREADYREGISTERED, ##__VA_ARGS__)
#define ERR_PASSWDMISMATCH(...)			generate(ERR_CODE_PASSWDMISMATCH, ##__VA_ARGS__)
#define ERR_CHANNELISFULL(...)			generate(ERR_CODE_CHANNELISFULL, ##__VA_ARGS__)
#define ERR_UNKNOWNMODE(...)			generate(ERR_CODE_UNKNOWNMODE, ##__VA_ARGS__)
#define ERR_INVITEONLYCHAN(...)			generate(ERR_CODE_INVITEONLYCHAN, ##__VA_ARGS__)
#define ERR_BADCHANNELKEY(...)			generate(ERR_CODE_BADCHANNELKEY, ##__VA_ARGS__)
#define ERR_CHANOPRIVSNEEDED(...)		generate(ERR_CODE_CHANOPRIVSNEEDED, ##__VA_ARGS__)
#define ERR_INVALIDMODEPARAM(...)		generate(ERR_CODE_INVALIDMODEPARAM, ##__VA_ARGS__)

class IRCArchitect
{
	private:

	public:
		IRCArchitect(void) {}
		~IRCArchitect(void) {}

		const str	generate(IRCReplyCode code, uint32_t paramCount, ...) const
		{
			std::stringstream	stream;
			str					reply = "";
			va_list				list;

			stream << std::setfill('0') << std::setw(3) << code;
			reply += stream.str();
			va_start(list, paramCount);
			for (size_t i = 0; i < paramCount; ++i)
			{
				const str	param = va_arg(list, char *);

				reply += " ";
				if (param.find(" ") != str::npos || param.find(":") != str::npos || i == paramCount)
					reply += ":";
				reply += param;
			}
			va_end(list);
			return (reply);
		}
		const str	generate(const str &source, const str &command, uint32_t paramCount, ...) const
		{
			str					reply = ": ";
			va_list				list;

			reply += source + command;
			va_start(list, paramCount);
			for (size_t i = 0; i < paramCount; ++i)
			{
				const str	param = va_arg(list, char *);

				reply += " ";
				reply += param;
			}
			va_end(list);
			return (reply);
		}
};

#endif	//	IRCARCHITECT_H
