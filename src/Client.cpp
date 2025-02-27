#include <map>
#include <unistd.h>
#include <sys/poll.h>
#include <Client.h>
#include <Channel.h>

Client::Client(void): ATarget() {}

Client::~Client(void) {}

Client::Client(uint32_t flag, int32_t fd): _flag(flag), _fd(fd), _username(""), _nickname("") {}

void	Client::resetBuffer(void) { _buffer = ""; IRC_FLAG_DEL(_flag, IRC_CLIENT_EOT); }

int	Client::_readToBuffer(void)
{
	char	tmp[1024] = {0};
	int 	bytes = 0;

	bytes = read(_fd, tmp, sizeof(tmp) - 1);
	if (bytes)
	{
		tmp[bytes] = 0;
		_buffer += str(tmp);
		IRC_LOG("client [%d] - appending [%s]", _fd, tmp);
	}
	return (bytes);
}

void	Client::disconnect()
{
	IRC_LOG("client "BOLD(COLOR(GRAY,"[%d]"))" disconnected.", _fd);
	close(_fd);
}

void	Client::readBytes(void)
{
	int	bytes;

	bytes = _readToBuffer();
	IRC_LOG("client [%d] - readbytes returned %d", _fd, bytes);
	if (bytes == 0 || _buffer.length() == 0)
	{
		IRC_FLAG_SET(_flag, IRC_CLIENT_EOF);
		return ;
	}
	if (_buffer.find("\n") != std::string::npos)
	{
		IRC_FLAG_SET(_flag, IRC_CLIENT_EOT);
		return ;
	}
}

// void	Client::clientInfo(void)
// {
// 	IRC_LOG
// 	(
// 		"Client:\n"
// 		"\tfrom file descriptor      "BOLD(COLOR(GRAY,"[%d]"))"\n"
// 		"\tchannels :"
// 	);
// 	for (IRC_AUTO it = _channelMap.begin(); it < _channelMap; ++it)
// 		(*it).second.channelInfo();
// }


void	Client::joinChannel(Channel *channel)
{
	str	channelName = channel->get_name();

	IRC_AUTO s = _channelMap.find(channelName);
	if (s == _channelMap.end())
		_channelMap[channelName] = channel;
}

void	Client::leaveChannel(Channel *channel)
{
	std::map<str, Channel * >::iterator s;

	s = _channelMap.find(channel->get_name());
	if (s != _channelMap.end())
	{
		delete s->second;
		_channelMap.erase(s);
	}
}
void	Client::leaveChannel(Channel *channel, str comment)
{
	std::map<str, Channel * >::iterator s;
	str									channelName; 

	channelName = channel->get_name();
	s = _channelMap.find(channelName);
	if (s != _channelMap.end())
	{
		delete s->second;
		_channelMap.erase(s);
		(void)comment; //TODO kicked msg
	}
}
