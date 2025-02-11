/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelOperator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yroussea <yroussea@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 14:41:59 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/11 16:31:20 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Channel.h>
#include <RegexMatch.h>
#include <cstring>
#include <utility>
#include <vector>

//KICK
//INVITE
//TOPIC
//MODE
//JOIN

#define REGEX_KICK "(KICK)"
#define REGEX_INVITE "(INVITE)"
#define REGEX_TOPIC "(TOPIC)"
#define REGEX_MODE "(MODE)"
#define REGEX_JOIN "(JOIN)"
#define REGEX_CHANNEL "([&#][^\7, ])"
#define REGEX_KEY "([^ ,])" //TODO verif regex

void	kick(str command);
void	invite(str command);
void	topic(str command);
void	mode(str command);

str	strldup(char *c, int len) //TODO verif car g pas la fois de ca xd
{
	char d = c[len];
	c[len] = 0;
	str	result(c);
	c[len] = d;
	return result;
}

void	join(str command)
{
	char	*save = strstr((char *)command.c_str(), "JOIN");
	char	*c = save + 5;
	//TODO verif que c est bien plasser

	regmatch_t	pmatch[2];
	std::vector<std::pair<str, str *> > vec;

	while (c && *c && regex_find(REGEX_CHANNEL, c, pmatch))
	{
		str channel = strldup(c + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so); //TODO verif len +-1
		vec.push_back(std::make_pair(channel, NULL));
	}
	c = strchr(save + 5, ' ');
	int i = 0;
	while (c && *c && regex_find(REGEX_KEY, c, pmatch))
	{
		str key = strldup(c + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so); //TODO verif len +-1
		vec[i].second = &key;
		i++;
	}
	//TODO faire les joins
	//TODO ne pas oublier ce qu il y a possiblement avant le JOIN
}

void	channelOperatorCommand(str command)
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
