#include <irc.h>

class RplGenerator
{
	private:
		str _client;
		str _channel;
		str _topic;
		str _clientList;
	public:
		RplGenerator(str client, str channel, str topic, str clientList);
		str rpl_join();
		str	rpl_topic();
		str	rpl_namreply();
		str	rpl_endofnames();
};
