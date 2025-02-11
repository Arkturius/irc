/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Regex.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/11 16:01:30 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <RegexMatch.h>
#include <regex.h>

bool	regex_match(const str regex, const char *string)
{
	__attribute__((cleanup(regfree)))regex_t		preg;

	int err = regcomp(&preg, regex.c_str(), REG_NOSUB | REG_EXTENDED);
	if (err == 0)
	{
		int match = regexec(&preg, string, 0, NULL, 0);
		if (match == 0)
			return true;
	}
	return false;
}

bool	regex_find(const str regex, const char *string, regmatch_t *pmatch)
{
	__attribute__((cleanup(regfree)))regex_t		preg;

	int err = regcomp(&preg, regex.c_str(), REG_EXTENDED);
	if (err == 0)
	{
		int match = regexec(&preg, string, 2, pmatch, 0);
		if (match == 0)
			return true;
	}
	return false;
}
