/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Regex.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 00:38:20 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <RegexMatch.h>
#include <cstring>
#include <regex.h>

bool	regex_match(const char *regex, const char *string)
{
	__attribute__((cleanup(regfree)))regex_t		preg;

	int err = regcomp(&preg, regex, REG_NOSUB | REG_EXTENDED);
	if (err == 0)
	{
		int match = regexec(&preg, string, 0, NULL, 0);
		return (!match);
	}
	return false;
}

bool	regex_find(const char *regex, const char *string, regmatch_t *groups)
{
	__attribute__((cleanup(regfree)))regex_t		preg;

	int err = regcomp(&preg, regex, REG_EXTENDED);
	if (err == 0)
	{
		std::memset(groups, 0, sizeof(regmatch_t));
		int match = regexec(&preg, string, 2, groups, 0);
		return (!match);
	}
	return (false);
}
