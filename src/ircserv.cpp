/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:02:04 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 00:17:49 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sstream>

#include <Server.h>
#include <unistd.h>

#define IRC_START	0
#define IRC_STOP	1

void	ircSigHandler(int sig);

void	ircIO(bool action)
{
	struct termios		term;

	tcgetattr(STDIN_FILENO, &term);

	if (action == IRC_START)
	{
		IRC_LOG("Setting up I/O.");

		struct sigaction	handler;
		
		handler.sa_handler = ircSigHandler;
		sigaction(SIGINT, &handler, NULL);

		IRC_FLAG_DEL(term.c_lflag, ECHOCTL);
		IRC_OK("I/O Setup.");
	}
	else
		IRC_FLAG_SET(term.c_lflag, ECHOCTL);

	tcsetattr(STDIN_FILENO, 0, &term);
	
}

void	ircSigHandler(int sig)
{
	if (sig == SIGINT)
	{
		// TODO :	clear channels, disconnect clients properly etc ...
		//			maybe have a function to do al of it cause we could exit from another place
		//
		IRC_OK("Server has been shut down by ^C.");
		ircIO(IRC_STOP);
		exit(0);
	}
}

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
		ircIO(IRC_START);
		
		Server	server(port, argv[2]);

		server.init();
		server.start();

		ircIO(IRC_STOP);
	}
	catch (std::exception &e)
	{
		IRC_ERR("%s", e.what());
		return (1);
	}
	return (0);
}
