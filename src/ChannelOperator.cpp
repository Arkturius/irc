/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelOperator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yroussea <yroussea@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 14:41:59 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/13 23:37:39 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <poll.h>
#include <RegexMatch.h>
#include <Client.h>
#include <Server.h>
#include <vector>

void	Server::_join(const str command, Client *client)
{
	str		cmd = command;
	char	tmp[1000]; //TODO max(channel size, key size)

	cmd += 5;
	regmatch_t	pmatch[2];
	std::vector<str> vecChannel;
	std::vector<str> vecPassword;

	while (cmd.size() && regex_find(R_CHANNEL_NAME, cmd.c_str(), pmatch))
	{
		str	channel;
		cmd.copy(tmp, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[1].rm_so);
		channel = tmp;
		vecChannel.push_back(channel);
		cmd += pmatch[1].rm_eo;
	}
	while (cmd.size() && regex_find(R_CHANNEL_NAME, cmd.c_str(), pmatch))
	{
		str	key;
		cmd.copy(tmp, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[1].rm_so);
		key = tmp;
		vecPassword.push_back(key);
		cmd += pmatch[1].rm_eo;
	}
	if (vecPassword.size() > vecChannel.size())
		; //TODO trop de key
	int j;
	for (j = 0; j < (int)vecPassword.size(); j++)
	{
		str	*tmp = NULL;
		*tmp = vecPassword[j]; //TODO verif, je suis trop fatiguer pour essaye de penser;
							   //y a un monde on ca marche pas dutout xd
		_addChannel(vecChannel[j], tmp, client);
	}
	for (; j < (int)vecChannel.size(); j++)
	{
		_addChannel(vecChannel[j], NULL, client);
	}
}


void	Server::_addChannel(str channelName, str *channelKey, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = NULL;
	auto			s = _channelMap.find(channelName);

	if (s != _channelMap.end())
	{
		c = s->second;
		c->addClient(pfd->fd, channelKey);
	}
	else
	{
		*c = Channel(channelName, pfd->fd);
		_channelMap[channelName] = c;
		c->addClient(pfd->fd, channelKey);
	}
	client->joinChannel(c);
}
