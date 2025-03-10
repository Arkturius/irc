/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCBot.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 14:01:26 by rgramati          #+#    #+#             */
/*   Updated: 2025/03/06 17:27:40 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <bot/IRCBot.h>
#include <sstream>

int main(int argc, char **argv)
{
	if (argc != 4)
		return (1);

	int serverFd;
	std::stringstream	ss(argv[1]);

	ss >> serverFd;

	try
	{
		IRCBot	bot(serverFd, argv[2], argv[3]);

		bot.start();
	}
	IRC_CATCH

	return (0);
}
