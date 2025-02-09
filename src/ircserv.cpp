/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:02:04 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/09 21:18:19 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.h>

#include <sstream>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		IRC_ERR("usage: ./ircserv <port> <password>");
		return (1);
	}

	int					port = 0;
	std::stringstream	args(argv[1]);
	args >> port;

	try
	{
		Server	server(port, argv[2]);

		server.initSocket();
		server.start();
		return (0);
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
		return (1);
	}
}
