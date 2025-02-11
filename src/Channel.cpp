/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/11 12:42:04 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Channel.h>
#include <algorithm>
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

Channel::Channel(str channelName): _name(channelName)
{
	IRC_LOG("Channel constructor called.");

	if (!regex_match(REGEX_APPROVE_CHANNEL_NAME, channelName))
		throw invalideChannelNameException();

}

Channel::~Channel(void)
{
	IRC_LOG("Channel destructor called.");

}

void	Channel::addClient(int fdClient)
{
	_fdClient.push_back(fdClient);
}

void	Channel::removeClient(int fdClient)
{
	__attribute__((unused))std::vector<int>::const_iterator	it = std::remove_if(_fdClient.begin(), _fdClient.end(), fdClient);
}

std::ostream	&operator<<(std::ostream &out, Channel const &channel)
{
	out << "Channel:" << channel.get_name() << " as " << channel.get_fdClient().size() << "Client";
	return out;
}
