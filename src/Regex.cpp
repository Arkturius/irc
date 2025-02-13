/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Regex.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 17:29:38 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <RegexMatch.h>
#include <cstring>
#include <regex.h>

bool	regex_match(const str regex, const char *string)
{
	__attribute__((cleanup(regfree)))regex_t		preg;

	int err = regcomp(&preg, regex.c_str(), REG_NOSUB | REG_EXTENDED);
	if (err == 0)
	{
		int match = regexec(&preg, string, 0, NULL, 0);
		return (!match);
	}
	return false;
}

bool	regex_find(const str regex, const char *string, size_t nmatch)
{
	__attribute__((cleanup(regfree)))regex_t		preg;
	regmatch_t										groups[256];

	if (nmatch > 255)
		return (false);

	int err = regcomp(&preg, regex.c_str(), REG_EXTENDED);
	if (err == 0)
	{
		std::memset(groups, 0, sizeof(regmatch_t));
		int match = regexec(&preg, string, nmatch, groups, 0);
		return (!match);
	}
	return (false);
}
