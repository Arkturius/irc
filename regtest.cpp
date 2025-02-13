/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regtest.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 19:21:59 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 17:05:56 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.h"
#include <irc.h>
#include <ircRegex.h>
#include <stdio.h>

int main(int argc, char **argv)
{
// 	if (argc != 2)
// 		return 1;
// 
// 	str	command = str(argv[1]);
// 
// 	CommandParser	parser;
//
	UNUSED(argc);
	UNUSED(argv);
	std::cout << "Channel pattern = '" << R_CHANNEL_NAME << "'" << std::endl;
}
