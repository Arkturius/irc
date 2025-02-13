/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 11:56:54 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 00:46:23 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/poll.h>
#include <Client.h>
#include <Channel.h>

Client::Client(void) {}

Client::~Client(void) {}

Client::Client(struct pollfd *pfd): _flag(IRC_CLIENT_CONNECTED), _pfd(pfd) {}

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
// 	for (auto it = _channelMap.begin(); it < _channelMap; ++it)
// 		(*it).second.channelInfo();
// }


void	Client::joinChannel(Channel *channel)
{
	str	channelName = channel->get_name();

	auto s = _channelMap.find(channelName);
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
	str	channelName = channel->get_name();

	auto s = _channelMap.find(channelName);
	if (s != _channelMap.end())
	{
		_channelMap.erase(s);
		//TODO sucefully leave the channel;
	}
	else
	{
		//TODO tu etais pas dedant ...
	}
}
