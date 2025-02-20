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

#define	RPL_WELCOME(p, ...)				generate(p, RPL_CODE_WELCOME, ##__VA_ARGS__)		
#define	RPL_YOUHOST(p, ...)				generate(p, RPL_CODE_YOUHOST, ##__VA_ARGS__)		
#define	RPL_CREATED(p, ...)				generate(p, RPL_CODE_CREATED, ##__VA_ARGS__)		
#define	RPL_MYINFO(p, ...)				generate(p, RPL_CODE_MYINFO, ##__VA_ARGS__)		
#define	RPL_ISUPPORT(p, ...)			generate(p, RPL_CODE_ISUPPORT, ##__VA_ARGS__)	
#define RPL_CHANNELMODEIS(p, ...)		generate(p, RPL_CODE_CHANNELMODEIS, ##__VA_ARGS__)
#define	RPL_TOPIC(p, ...)				generate(p, RPL_CODE_TOPIC, ##__VA_ARGS__)		
#define	RPL_NAMREPLY(p, ...)			generate(p, RPL_CODE_NAMREPLY, ##__VA_ARGS__)	
#define	RPL_ENDOFNAMES(p, ...)			generate(p, RPL_CODE_ENDOFNAMES, ##__VA_ARGS__)

#define RPL_NOTOPIC(p, ...)				generate(p, RPL_CODE_NOTOPIC, ##__VA_ARGS__)
#define RPL_TOPICWHOTIME(p, ...)		generate(p, RPL_CODE_TOPICWHOTIME, ##__VA_ARGS__)
#define RPL_INVITING(p, ...)			generate(p, RPL_CODE_INVITING, ##__VA_ARGS__)
#define ERR_NOSUCHNICK(p, ...)			generate(p, ERR_CODE_NOSUCHNICK, ##__VA_ARGS__)
#define ERR_NOSUCHCHANNEL(p, ...)		generate(p, ERR_CODE_NOSUCHCHANNEL, ##__VA_ARGS__)
#define ERR_NONICKNAMEGIVEN(p, ...)		generate(p, ERR_CODE_NONICKNAMEGIVEN, ##__VA_ARGS__)
#define ERR_ERRONEUSNICKNAME(p, ...)	generate(p, ERR_CODE_ERRONEUSNICKNAME, ##__VA_ARGS__)
#define ERR_NICKNAMEINUSE(p, ...)		generate(p, ERR_CODE_NICKNAMEINUSE, ##__VA_ARGS__)
#define ERR_USERNOTINCHANNEL(p, ...)	generate(p, ERR_CODE_USERNOTINCHANNEL, ##__VA_ARGS__)
#define ERR_NOTONCHANNEL(p, ...)		generate(p, ERR_CODE_NOTONCHANNEL, ##__VA_ARGS__)
#define ERR_USERONCHANNEL(p, ...)		generate(p, ERR_CODE_USERONCHANNEL, ##__VA_ARGS__)
#define ERR_NEEDMOREPARAMS(p, ...)		generate(p, ERR_CODE_NEEDMOREPARAMS, ##__VA_ARGS__)
#define ERR_ALREADYREGISTERED(p, ...)	generate(p, ERR_CODE_ALREADYREGISTERED, ##__VA_ARGS__)
#define ERR_PASSWDMISMATCH(p, ...)		generate(p, ERR_CODE_PASSWDMISMATCH, ##__VA_ARGS__)
#define ERR_CHANNELISFULL(p, ...)		generate(p, ERR_CODE_CHANNELISFULL, ##__VA_ARGS__)
#define ERR_UNKNOWNMODE(p, ...)			generate(p, ERR_CODE_UNKNOWNMODE, ##__VA_ARGS__)
#define ERR_INVITEONLYCHAN(p, ...)		generate(p, ERR_CODE_INVITEONLYCHAN, ##__VA_ARGS__)
#define ERR_BADCHANNELKEY(p, ...)		generate(p, ERR_CODE_BADCHANNELKEY, ##__VA_ARGS__)
#define ERR_CHANOPRIVSNEEDED(p, ...)	generate(p, ERR_CODE_CHANOPRIVSNEEDED, ##__VA_ARGS__)
#define ERR_INVALIDMODEPARAM(p, ...)	generate(p, ERR_CODE_INVALIDMODEPARAM, ##__VA_ARGS__)

#include <IRCSeeker.h>

class IRCArchitect
{
	private:
		IRCSeeker	_seeker;

	public:
		IRCArchitect(void) {}
		~IRCArchitect(void) {}

		const str	generate(const str &prefix, IRCReplyCode code, uint32_t paramCount, ...)
		{
			std::stringstream	stream;
			str					reply = "";
			va_list				list;

			reply += prefix;
			stream << std::setfill('0') << std::setw(3) << code;
			reply += stream.str();
			va_start(list, paramCount);
			for (size_t i = 0; i < paramCount; ++i)
			{
				const str	param = va_arg(list, char *);

				if (param.empty())
					continue ;
				reply += " ";

				if (i != paramCount - 1)
				{
					_seeker.feedString(param);
					_seeker.rebuild(R_CAPTURE(R_CHAR_INV_GROUP(" ") "*"));
					if (!_seeker.match())
						throw InvalidReplyParameterException();
				}
				else
					reply += ":";

				reply += param;
			}
			va_end(list);
			return (reply);
		}
	
	EXCEPTION(InvalidReplyParameterException, "Invalid reply parameter.");
};

#endif	//	IRCARCHITECT_H
