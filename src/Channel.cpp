/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/11 14:00:12 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Channel.h>
#include <algorithm>
#include <exception>
#include <regex.h>
#include <vector>

bool	regex_match(str regex, str string)
{
	regex_t preg;
	int err = regcomp (&preg, regex.c_str(), REG_NOSUB | REG_EXTENDED);
	if (err == 0)
	{
		int match = regexec (&preg, string.c_str(), 0, NULL, 0);
		regfree (&preg);
		if (match == 0)
			return true;
	}
	return false;
}

Channel::Channel(str channelName, int firstClient): _name(channelName), _inviteOnlyChannel(true), _userLimit(100)
{
	IRC_LOG("Channel constructor called.");

	if (!regex_match(REGEX_APPROVE_CHANNEL_NAME, channelName))
		throw invalideChannelNameException();

	_fdAdminClient.push_back(firstClient);
}

Channel::~Channel(void)
{
	IRC_LOG("Channel destructor called.");
}

void	Channel::addClient(int fdClient, int perm)
{
	//TODO password here?
	//TODO check if alraidy in?
	if (perm)
		_fdAdminClient.push_back(fdClient);
	else
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
	throw clientNotInChannelException();
end:
	return (get_size());
}

bool	Channel::havePerm(int fdClient)
{
	auto it = std::find(_fdAdminClient.begin(), _fdAdminClient.end(), fdClient);
	if (it != _fdAdminClient.end())
		return true;

	it = std::find(_fdClient.begin(), _fdClient.end(), fdClient);
	if (it != _fdClient.end())
		return false;
	throw clientNotInChannelException();
}

void	Channel::givePerm(int userClient, int targetClient)
{
	try
	{
		if (havePerm(userClient) && !havePerm(targetClient))
		{
			removeClient(targetClient);
			addClient(targetClient, 1);
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}
void	Channel::removePerm(int targetClient)
{
	try
	{
		if (havePerm(targetClient))
		{
			removeClient(targetClient);
			addClient(targetClient, 0);
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}
