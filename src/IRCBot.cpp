/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCBot.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 14:01:26 by rgramati          #+#    #+#             */
/*   Updated: 2025/03/10 18:05:55 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <bot/IRCBot.h>

int main(int argc, char **argv)
{
	if (argc != 4)
		return (1);

	try
	{
		IRCBot	bot(argv[1], argv[2], argv[3]);

		bot.start();
	}
	IRC_CATCH

	return (0);
}
