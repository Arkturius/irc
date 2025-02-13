/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:42:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 17:02:35 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandParser.hpp"
#include "irc.h"

CommandParser::CommandParser(void)
{
	IRC_LOG("CommandParser constructor called.");
}

CommandParser::CommandParser(const str &pattern)
{
	int	err;

	err = regcomp(&_state, pattern.c_str(), REG_EXTENDED);
	if (err)
		throw CPInvalidPatternException();
}

CommandParser::~CommandParser(void)
{
	regfree(&_state);
	IRC_LOG("CommandParser destructor called.");
}

#define IRC_CP_MATCH_FAIL	1 << 2

void	CommandParser::parseCommand(const str &command, size_t nmatch)
{
	int			err;
	regmatch_t	match[1024];

	err = regexec(&_state, command.c_str(), nmatch, match, 0);
	if (err)
	{
		IRC_FLAG_SET(_flag, IRC_CP_MATCH_FAIL);
		return ;
	}
}
