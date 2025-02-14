/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelOperator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yroussea <yroussea@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 14:41:59 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/14 19:05:03 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <poll.h>
#include <vector>

#include <Server.h>
#include <Client.h>
#include <Channel.h>
#include <RParser.h>

void	Server::_join(const str command, Client *client)
{
	UNUSED(command);
	UNUSED(client);
// 	//JOIN channel,channel key,key
// 	str		cmd = command;
// 	char	tmp[50]; //TODO max(channel size, key size)
// 
// 	cmd += 5; //TODO ptet donner apres le JOIN? //TODO j ai pas le droit xd
// 	std::vector<str> vecChannel;
// 	std::vector<str> vecPassword;
// 
// 	while (cmd.size() && regex_find(R_CHANNEL_NAME, cmd.c_str(), pmatch))
// 	{
// 		str	channel;
// 		cmd.copy(tmp, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[1].rm_so);
// 		channel = tmp;
// 		vecChannel.push_back(channel);
// 		cmd += pmatch[1].rm_eo; //TODO j ai pas le droit xd
// 		//TODO if *cmd != "," => prb sauf si *cmd = " " => break
// 	}
// 	while (cmd.size() && regex_find(R_CHANNEL_NAME, cmd.c_str(), pmatch)) //TODO key not channel
// 	{
// 		str	key;
// 		cmd.copy(tmp, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[1].rm_so);
// 		key = tmp;
// 		vecPassword.push_back(key);
// 		cmd += pmatch[1].rm_eo; //TODO j ai pas le droit xd
// 		//TODO if *cmd != "," => prb sauf si *cmd = 0 = break;
// 	}
// 	if (vecPassword.size() > vecChannel.size())
// 		; //TODO trop de key => throw()
// 	int j;
// 	for (j = 0; j < (int)vecPassword.size(); j++)
// 	{
// 		str	*tmp = NULL;
// 		*tmp = vecPassword[j]; //TODO verif, je suis trop fatiguer pour essaye de penser;
// 							   //y a un monde on ca marche pas dutout xd
// 		_addChannel(vecChannel[j], tmp, client);
// 	}
// 	for (; j < (int)vecChannel.size(); j++)
// 	{
// 		_addChannel(vecChannel[j], NULL, client);
// 	}
}

void	Server::_kick(const str command, Client *client)
{
	 //Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
	//TODO soit 1channel et X user ; soit Nchannel et N user 
	//_kickChannel(channelName, client, clientTmp, comment)
	(void)command;(void)client;
}

void	Server::_topic(const str command, Client *client)
{
	//TOPIC chennel => print (can be not set (erreur))
	//TOPIC channel : => clear 
	//TOPIC channel : topicContent => rempli
	(void)command;(void)client;
}
void	Server::_mode(const str command, Client *client)
{
	(void)command;(void)client;
}
void	Server::_invite(const str command, Client *client)
{
	(void)command;(void)client;
}

void	Server::_addChannel(str channelName, str *channelKey, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = NULL;
	IRC_AUTO		s = _channelMap.find(channelName);

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

void	Server::_removeChannel(str channelName, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = NULL;
	IRC_AUTO		s = _channelMap.find(channelName);

	if (s != _channelMap.end())
	{
		c = s->second;
		int size = c->removeClient(pfd->fd);
		if (size == 0)
			_channelMap.erase(s);
	}
	client->leaveChannel(c);
}

void	Server::_kickChannel(str channelName, Client *admin, Client *kicked, str *comment)
{
	struct pollfd	*pfdAdmin = admin->get_pfd();
	struct pollfd	*pfdKicked = kicked->get_pfd();
	Channel			*c = NULL;
	IRC_AUTO		s = _channelMap.find(channelName);
	int				size;

	if (s != _channelMap.end())
	{
		c = s->second;
		if (!c->havePerm(pfdAdmin->fd))
			goto clientDontHaveThePerm;
		size = c->removeClient(pfdKicked->fd);
		if (size == 0)
			_channelMap.erase(s);
		if (comment)
			kicked->leaveChannel(c, *comment);
		else
			kicked->leaveChannel(c);
	}
	else
	{
		//user not in channel
	}
clientDontHaveThePerm:
	; //TODO "admin" est pas admin
}
