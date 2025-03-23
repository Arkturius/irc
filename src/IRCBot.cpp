/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCBot.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 14:01:26 by rgramati          #+#    #+#             */
/*   Updated: 2025/03/20 14:41:28 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <bot/IRCBot.h>

int main(int argc, char **argv)
{
	if (argc != 5)
		return (1);

	try
	{
		IRCBot	bot(argv[1], argv[2], argv[3], argv[4]);

		bot.start();
	}
	IRC_CATCH

	return (0);
}
