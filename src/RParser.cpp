/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RParser.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:47:39 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 18:41:06 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <RParser.h>

RParser::RParser(const str &pattern): _pattern(pattern), _matchIndex(0), _matchCount(0)
{
	int err;

	err = regcomp(&_expr, pattern.c_str(), REG_EXTENDED);
	if (err)
		throw InvalidPatternException();
}

RParser::~RParser(void)
{
	regfree(&_expr);
}

void	RParser::_findNext(const str &source)
{
	if (_matchIndex == -1U)
		return ;

	int			err;
	regmatch_t	hold[3];
	str			crop = source.substr(_matchIndex, source.length());

	err = regexec(&_expr, crop.c_str(), 3, hold, 0);
	if (err)
	{
		_matchIndex = -1U;
		return ;
	}

	int	start = hold[1].rm_so;
	int	end = hold[1].rm_eo;
	if (start == -1 || end == -1)
	{
		_matchIndex = -1U;
		return ;
	}

	str match = crop.substr(start, end - start);
	_matches.push_back(match);
	_matchIndex += end;
	_matchCount++;
}

bool	RParser::match(const str &source)
{
	_findNext(source);
	if (_matchIndex == -1U)
		return (false);
	_matchIndex = 0;
	return (true);
}

#include <unistd.h>
void	RParser::findall(const str &source)
{
	while (_matchIndex != -1U)
		_findNext(source);
	_matchIndex = 0;
}
