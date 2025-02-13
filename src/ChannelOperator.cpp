/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelOperator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yroussea <yroussea@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 14:41:59 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/13 07:39:28 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Channel.h>
#include <RegexMatch.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

//KICK
//INVITE
//TOPIC
//MODE
//JOIN

#define REGEX_KICK "(KICK) "
#define REGEX_INVITE "(INVITE) "
#define REGEX_TOPIC "(TOPIC) "
#define REGEX_MODE "(MODE) "
#define REGEX_JOIN "(JOIN) "
#define REGEX_CHANNEL "([&#][^\7, ])"
#define REGEX_KEY "([^ ,])" //TODO verif regex >> je sais pas bon

void	kick(str command);
void	invite(str command);
void	topic(str command);
void	mode(str command);

void	join(str command)
{
	char	tmp[1000];
	command += 5;

	regmatch_t	pmatch[2];
	std::vector<std::pair<str, str *> > vec;

	while (command.size() && regex_find(REGEX_CHANNEL, command.c_str(), pmatch))
	{
		str	channel;
		if (pmatch[1].rm_eo - pmatch[1].rm_so >= 201)
			;//TODO PRB
		command.copy(tmp, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[1].rm_so);
		channel = tmp;
		vec.push_back(std::make_pair(channel, NULL));
		command += pmatch[1].rm_eo;
	}
	int i = 0;
	while (command.size() && regex_find(REGEX_KEY, command.c_str(), pmatch) && i < (int)vec.size())
	{
		str	key;
		command.copy(tmp, pmatch[1].rm_eo - pmatch[1].rm_so, pmatch[1].rm_so);
		key = tmp;
		vec[i].second = &key;
		command += pmatch[1].rm_eo;
		i++;
	}
	if (command.size())
		; //TODO trop de key dc?
	for (int j = 0; i < (int)vec.size(); j++)
	{
		//TODO server.join_channel(user, vec[j].first, vec[j].second);
	}
}

void	channelOperatorCommand(str command /* + user*/)
{
	int					i;
	static const str	regexCommand[5] = \
		{REGEX_KICK, REGEX_INVITE, REGEX_TOPIC, REGEX_MODE, REGEX_JOIN};

	for (i = 0; i < 5; i++)
	{
		if (regex_match(regexCommand[i] + str(REGEX_CHANNEL), command.c_str()))
			break ;
	}
	switch (i)
	{
		case 0:
			return kick(command);
		case 1:
			return invite(command);
		case 2:
			return topic(command);
		case 3:
			return mode(command);
		case 4:
			return join(command);
		default:
			return ;
	}
}
