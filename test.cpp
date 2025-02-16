/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:23:00 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/17 00:05:51 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <regex.h>

#include <irc.h>
#include <RParser.h>

#define R_IRC_NOSPCRLFCL	R_CHAR_INV_GROUP(" \r\n:")
#define R_COMMAND_PASS		R_START_STRING R_CAPTURE("PASS") R_1_OR_MORE(" ") R_CAPTURE(R_1_OR_MORE(R_IRC_NOSPCRLFCL))

int main(void)
{
	const char	*pattern = R_COMMAND_PASS;
	const char	*string = "PASS skibidi\n";

	try
	{
		RParser	seeker(pattern);
#if 0
		std::cout << seeker.match(string) << std::endl;
#else
		seeker.capture(string, 2);
#endif

		std::vector<str>	&argv = seeker.get_matches();
		for (IRC_AUTO it = argv.begin(); it != argv.end(); ++it)
		{
			IRC_ERR("match = %s", (*it).c_str());
		}
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}
}
