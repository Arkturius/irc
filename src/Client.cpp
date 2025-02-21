#include <map>
#include <unistd.h>
#include <sys/poll.h>
#include <Client.h>
#include <Channel.h>

Client::Client(void) {}

Client::~Client(void) {}

Client::Client(struct pollfd *pfd, uint32_t flag): _flag(flag), _username(""), _nickname(""), _pfd(pfd) {}

void	Client::resetBuffer(void) { _buffer = ""; IRC_FLAG_DEL(_flag, IRC_CLIENT_EOT); }

size_t	Client::_readToBuffer(void)
{
	char	tmp[1024] = {0};
	size_t	bytes = 0;

	bytes = read(_pfd->fd, tmp, sizeof(tmp) - 1);
	if (bytes)
	{
		tmp[bytes] = 0;
		_buffer += str(tmp);
	}
	return (bytes);
}

void	Client::disconnect()
{
	IRC_LOG("client "BOLD(COLOR(GRAY,"[%d]"))" disconnected.", _pfd->fd);
	close(_pfd->fd);
	_pfd->fd = -1;
	_pfd->events = 0;
}

void	Client::readBytes(void)
{
	size_t	bytes;

	bytes = _readToBuffer();
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
	if (s != _channelMap.end())
	{
		//TODO tu est deja dedant..
	}
	else
	{
		_channelMap[channelName] = channel;
		//TODO sucefully join the channel;
	}
}
void	Client::leaveChannel(Channel *channel)
{
	std::map<str, Channel * >::iterator s;
	str									channelName; 

	if (!channel)
		goto channelDoesntExist;
	channelName = channel->get_name();
	s = _channelMap.find(channelName);
	if (s != _channelMap.end())
	{
		delete s->second;
		_channelMap.erase(s);
		//TODO sucefully leave the channel;
	}
	else
	{
channelDoesntExist:
		;
		//TODO tu etais pas dedant ...
	}
}
void	Client::leaveChannel(Channel *channel, str comment)
{
	std::map<str, Channel * >::iterator s;
	str									channelName; 

	if (!channel)
		goto channelDoesntExist;
	channelName = channel->get_name();
	s = _channelMap.find(channelName);
	if (s != _channelMap.end())
	{
		delete s->second;
		_channelMap.erase(s);
		//TODO sucefully leave the channel;
		(void)comment;
	}
	else
	{
channelDoesntExist:
		;
		//TODO tu etais pas dedant ...
	}
}
