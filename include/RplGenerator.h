#include <irc.h>

typedef enum
{
	RPL_CODE_WELCOME			=	1,
	RPL_CODE_YOUHOST			=	2,
	RPL_CODE_CREATED			=	3,
	RPL_CODE_MYINFO				=	4,
	RPL_CODE_ISUPPORT			=	5,
	RPL_CODE_NOTOPIC			=	331,
	RPL_CODE_TOPIC				=	332,
	RPL_CODE_TOPICWHOTIME		=	333,
	RPL_CODE_NAMREPLY			=	353,
	RPL_CODE_ENDOFNAMES			=	366,
	ERR_CODE_NOSUCHCHANNEL		=	403,
	ERR_CODE_USERNOTINCHANNEL	=	441,
	ERR_CODE_NOTONCHANNEL		=	442,
	ERR_CODE_USERONCHANNEL		=	443,
	ERR_CODE_NEEDMOREPARAMS		=	461,
	ERR_CODE_ALREADYREGISTERED	=	462,
	ERR_CODE_PASSWDMISMATCH		=	463,
	ERR_CODE_INVITEONLYCHAN		=	473,
	ERR_CODE_BADCHANNELKEY		=	475,
	ERR_CODE_CHANOPRIVSNEEDED	=	482,

	RPL_CODE_LAST				=	909
}	IRCReplyCode;

#define	RPL_WELCOME(p, ...)				generate(p, RPL_CODE_WELCOME, ##__VA_ARGS__)		
#define	RPL_YOUHOST(p, ...)				generate(p, RPL_CODE_YOUHOST, ##__VA_ARGS__)		
#define	RPL_CREATED(p, ...)				generate(p, RPL_CODE_CREATED, ##__VA_ARGS__)		
#define	RPL_MYINFO(p, ...)				generate(p, RPL_CODE_MYINFO, ##__VA_ARGS__)		
#define	RPL_ISUPPORT(p, ...)			generate(p, RPL_CODE_ISUPPORT, ##__VA_ARGS__)	
#define	RPL_TOPIC(p, ...)				generate(p, RPL_CODE_TOPIC, ##__VA_ARGS__)		
#define	RPL_NAMREPLY(p, ...)			generate(p, RPL_CODE_NAMREPLY, ##__VA_ARGS__)	
#define	RPL_ENDOFNAMES(p, ...)			generate(p, RPL_CODE_ENDOFNAMES, ##__VA_ARGS__)

#define RPL_NOTOPIC(p, ...)				generate(p, RPL_CODE_NOTOPIC, ##__VA_ARGS__)
#define RPL_TOPICWHOTIME(p, ...)		generate(p, RPL_CODE_TOPICWHOTIME, ##__VA_ARGS__)
#define ERR_NOSUCHCHANNEL(p, ...)		generate(p, ERR_CODE_NOSUCHCHANNEL, ##__VA_ARGS__)
#define ERR_USERNOTINCHANNEL(p, ...)	generate(p, ERR_CODE_USERNOTINCHANNEL, ##__VA_ARGS__)
#define ERR_NOTONCHANNEL(p, ...)		generate(p, ERR_CODE_NOTONCHANNEL, ##__VA_ARGS__)
#define ERR_USERONCHANNEL(p, ...)		generate(p, ERR_CODE_USERONCHANNEL, ##__VA_ARGS__)
#define ERR_INVITEONLYCHAN(p, ...)		generate(p, ERR_CODE_INVITEONLYCHAN, ##__VA_ARGS__)
#define ERR_BADCHANNELKEY(p, ...)		generate(p, ERR_BADCHANNELKEY, ##__VA_ARGS__)

#define ERR_NEEDMOREPARAMS(p, ...)		generate(p, ERR_CODE_NEEDMOREPARAMS, ##__VA_ARGS__)
#define ERR_ALREADYREGISTERED(p, ...)	generate(p, ERR_CODE_ALREADYREGISTERED, ##__VA_ARGS__)
#define ERR_PASSWDMISMATCH(p, ...)		generate(p, ERR_CODE_PASSWDMISMATCH, ##__VA_ARGS__)

class RplGenerator
{
	private:

	public:
		RplGenerator(void);
		str	generate(str prefix, IRCReplyCode code, uint32_t paramCount, ...);
};
