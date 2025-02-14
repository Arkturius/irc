/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/14 20:18:41 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <exception>
#include <vector>

#include <RParser.h>
#include <Channel.h>

Channel::Channel(str channelName, int firstClient): _name(channelName), _inviteOnlyChannel(true), _activePassword(0), _userLimit(100)
{
	IRC_LOG("Channel constructor called.");

	RParser	parser(R_CHANNEL_NAME);

	if (parser.match(channelName.c_str()))
		throw InvalidChannelNameException();
	_fdAdminClient.push_back(firstClient);

	IRC_LOG("Channel Succesfully created");
}

Channel::~Channel(void)
{
	IRC_LOG("Channel destructor called.");
}

void	Channel::_addClient(int fdClient, int perm)
{
	if (perm)
		_fdAdminClient.push_back(fdClient);
	else
		_fdClient.push_back(fdClient);
}
void	Channel::addClient(int fdClient, str *password)
{
	//TODO if password given but not needed? et l inverse
	//TODO la ca va segfault dans ce cas xd
	if (_activePassword && *password != _password)
		throw InvalidChannelKeyException();

	std::vector<int>::iterator	it;
	for (it = _fdClient.begin(); it != _fdClient.end(); ++it)
	{
		if (*it == fdClient)
			return ;
	}
	for (it = _fdAdminClient.begin(); it != _fdAdminClient.end(); ++it)
	{
		if (*it == fdClient)
			return ;
	}
	_fdClient.push_back(fdClient);
}

int	Channel::removeClient(int fdClient)
{
	std::vector<int>::iterator	it;

	for (it = _fdClient.begin(); it != _fdClient.end(); ++it)
	{
		if (*it == fdClient)
		{
			_fdClient.erase(it);
			goto end;
		}
	}
	for (it = _fdAdminClient.begin(); it != _fdClient.end(); ++it)
	{
		if (*it == fdClient)
		{
			_fdAdminClient.erase(it);
			goto end;
		}
	}
	throw ClientNotInChannelException();
end:
	return (get_size());
}

bool	Channel::havePerm(int fdClient)
{
	IRC_AUTO it = std::find(_fdAdminClient.begin(), _fdAdminClient.end(), fdClient);
	if (it != _fdAdminClient.end())
		return true;

	it = std::find(_fdClient.begin(), _fdClient.end(), fdClient);
	if (it != _fdClient.end())
		return false;
	throw ClientNotInChannelException();
}

void	Channel::givePerm(int userClient, int targetClient)
{
	try
	{
		if (havePerm(userClient) && !havePerm(targetClient))
		{
			removeClient(targetClient);
			_addClient(targetClient, 1);
		}
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}
}
void	Channel::removePerm(int targetClient)
{
	try
	{
		if (havePerm(targetClient))
		{
			removeClient(targetClient);
			_addClient(targetClient, 0);
		}
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}
}
