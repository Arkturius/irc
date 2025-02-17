/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:23:00 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/17 21:17:15 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <exception>
#include <regex.h>

#include <IRCSeeker.h>
#include <ircRegex.h>

#define R_NOAUTH_COMMANDS		R_START_STRING R_CAPTURE("CAP|PASS|USER|NICK|QUIT")
#define R_CHANNEL_COMMANDS		R_START_STRING R_CAPTURE("JOIN|MODE")
#define	R_IRC_ACCEPTED_COMMANDS	R_NOAUTH_COMMANDS "|" R_CHANNEL_COMMANDS


//	( [^\r\n :](:|[^\r\n :])*){0,14}
//	( :([^\r\n]*))?
//	|
//	(
//		( [^\r\n :](:|[^\r\n :])*)
//	){14}
//	( :?([^\r\n]*))?


void	print_pattern(const char *p)
{
	std::cout << RED;
	while (*p)
	{
		if (std::isprint(*p))
			std::cout << *p;
		else
		{
			switch (*p)
			{
				case '\n':
					std::cout << "\\n";
					break;

				case '\r':
					std::cout << "\\r";
					break;
			}
		}
		++p;
	}
	std::cout << RESET << std::endl;
}

#define	R_SPACE				" "
#define R_NOCRLF			R_CHAR_INV_GROUP("\r\n")
#define R_NOSPCRLFCL		R_CHAR_INV_GROUP("\r\n :")

#define R_MIDDLE			R_NOSPCRLFCL R_CAPTURE(R_0_OR_MORE(":|" R_NOSPCRLFCL))
#define R_TRAILING			R_0_OR_MORE(R_NOCRLF)

#define R_PARAMS_LESS14		R_X_TO_Y(R_CAPTURE(R_SPACE R_MIDDLE),0,14) R_0_OR_1(R_CAPTURE(R_SPACE ":" R_TRAILING))
#define R_PARAMS_14OR15		R_X_EXACT(R_CAPTURE(R_SPACE R_MIDDLE),14) R_0_OR_1(R_CAPTURE(R_SPACE R_0_OR_1(":") R_TRAILING))

#define R_STRING(t)			R_START_STRING t R_END_STRING

#define R_PARAMS			R_START_STRING R_CAPTURE(R_PARAMS_LESS14) R_END_STRING "|" R_START_STRING R_CAPTURE(R_PARAMS_14OR15) R_END_STRING

int main(void)
{
	const char	*pattern = R_CAPTURE(R_SPACE R_MIDDLE);
	const char	*string = " rgramati * 0";

	try
	{
		IRCSeeker	seeker(pattern);

		std::cout << "testing string [" << RED << string << RESET << "] with pattern ";
		print_pattern(pattern);
#if 0
		std::cout << seeker.match(string) << std::endl;
#else
		seeker.findall(string);

#endif

		std::vector<str>	&argv = seeker.get_matches();
		IRC_LOG("size of matches vector = %ld", argv.size());
		for (IRC_AUTO it = argv.begin(); it != argv.end(); ++it)
		{
			IRC_WARN("match = %s", (*it).c_str());
		}

		pattern = R_CAPTURE(R_SPACE R_MIDDLE);
		pattern = R_SPACE R_CAPTURE_WORD;
		seeker.rebuild(pattern);
		
		std::cout << "testing string [" << RED << string << RESET << "] with pattern ";
		print_pattern(pattern);

		seeker.capture(string, argv.size());

		argv = seeker.get_matches();
		IRC_LOG("size of matches vector = %ld", argv.size());
		for (IRC_AUTO it = argv.begin(); it != argv.end(); ++it)
		{
			IRC_WARN("match = %s", (*it).c_str());
		}
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
	}
}
