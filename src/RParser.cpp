/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RParser.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:47:39 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/17 00:05:04 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <RParser.h>
#include <exception>
#include <regex.h>

RParser::RParser(void): _pattern(""), _matchIndex(0), _matchCount(0) {}

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

void	RParser::_findNext(const str &source, size_t nmatch)
{
	if (_pattern == "")
		throw EmptyPatternException();
	if (_matchIndex == -1U)
		return ;

	regmatch_t	*hold = new regmatch_t[nmatch + 2];
	str			crop = source.substr(_matchIndex, source.length());

	IRC_LOG("_findNext on crop = [%s]", crop.c_str());

	int			err = regexec(&_expr, crop.c_str(), nmatch + 2, hold, 0);
	int			start = hold[1].rm_so;
	int			end = hold[1].rm_eo;

	if (err || start == -1 || end == -1)
	{
		delete[] hold;
		_matchIndex = -1U;
		return ;
	}

	for (size_t i = 0; i < nmatch; ++i)
	{
		start = hold[i + 1].rm_so;
		end = hold[i + 1].rm_eo;

		str match = crop.substr(start, end - start);
		_matches.push_back(match);
		_matchIndex += end;
		_matchCount++;
	}

	delete[] hold;
}

void	RParser::_reset(void)
{
	_pattern = "";
	_matchCount = 0;
	_matchIndex = -1U;
	regfree(&_expr);
	_matches.clear();
}

void	RParser::rebuild(const str &pattern)
{
	int err;

	if (pattern != "")
		_reset();
	err = regcomp(&_expr, pattern.c_str(), REG_EXTENDED);
	if (err)
		throw InvalidPatternException();
	_pattern = pattern;
	_matchIndex = 0;
}

bool	RParser::match(const str &source)
{
	try { _findNext(source, 1); }
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}

	if (_matchIndex == -1U)
		return (false);
	_matchIndex = 0;
	return (true);
}

void	RParser::capture(const str &source, size_t n)
{
	try { _findNext(source, n); }	
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}
	_matchIndex = 0;
}

void	RParser::findall(const str &source)
{
	try { while (_matchIndex != -1U) _findNext(source, 1); }
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}

	_matchIndex = 0;
}
