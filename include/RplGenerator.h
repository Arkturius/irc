#include <irc.h>

typedef enum
{
	RPL_CODE_WELCOME			=	1,
	RPL_CODE_YOUHOST			=	2,
	RPL_CODE_CREATED			=	3,
	RPL_CODE_MYINFO				=	4,
	RPL_CODE_ISUPPORT			=	5,
	RPL_CODE_TOPIC				=	332,
	RPL_CODE_NAMREPLY			=	353,
	RPL_CODE_ENDOFNAMES			=	366,
	ERR_CODE_NEEDMOREPARAMS		=	461,
	ERR_CODE_ALREADYREGISTERED	=	462,
	ERR_CODE_PASSWDMISMATCH		=	463,
	
	RPL_CODE_LAST				=	909
}	IRCReplyCode;

#define	RPL_WELCOME(pre, ...)			generate(p, RPL_CODE_WELCOME, ##__VA_ARGS__)		
#define	RPL_YOUHOST(pre, ...)			generate(p, RPL_CODE_YOUHOST, ##__VA_ARGS__)		
#define	RPL_CREATED(pre, ...)			generate(p, RPL_CODE_CREATED, ##__VA_ARGS__)		
#define	RPL_MYINFO(pre, ...)			generate(p, RPL_CODE_MYINFO, ##__VA_ARGS__)		
#define	RPL_ISUPPORT(pre, ...)			generate(p, RPL_CODE_ISUPPORT, ##__VA_ARGS__)	
#define	RPL_TOPIC(pre, ...)				generate(p, RPL_CODE_TOPIC, ##__VA_ARGS__)		
#define	RPL_NAMREPLY(pre, ...)			generate(p, RPL_CODE_NAMREPLY, ##__VA_ARGS__)	
#define	RPL_ENDOFNAMES(pre, ...)		generate(p, RPL_CODE_ENDOFNAMES, ##__VA_ARGS__)

#define ERR_NEEDMOREPARAMS(pre, ...)	generate(p, ERR_NEEDMOREPARAMS, ##__VA_ARGS__)
#define ERR_ALREADYREGISTERED(pre, ...)	generate(p, ERR_ALREADYREGISTERED, ##__VA_ARGS__)
#define ERR_PASSWDMISMATCH(pre, ...)	generate(p, ERR_PASSWDMISMATCH, ##__VA_ARGS__)

class RplGenerator
{
	private:
		str _client;
		str _channel;
		str _topic;
		str _clientList;

	public:
		RplGenerator(str client, str channel, str topic, str clientList);
		str	generate(str prefix, IRCReplyCode code, uint32_t paramCount, ...);
		str rpl_join();
		str	rpl_topic();
		str	rpl_namreply();
		str	rpl_endofnames();
};
