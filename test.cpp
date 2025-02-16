/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:23:00 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/16 17:37:29 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <regex.h>

#include <irc.h>
#include <RParser.h>

int main(void)
{
	const char	*pattern = R_START_STRING R_CAPTURE("CAP|PASS|USER|NICK");
	const char	*string = "CAP LS 302\r\n";


	try
	{
		RParser				noAuth(pattern);
		IRC_LOG("[%s] %s in [%s]", pattern, noAuth.match(string) ? "found" : "not found", string);
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}
}
