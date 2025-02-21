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
	struct termios			term;
	static struct termios	start;

	tcgetattr(STDIN_FILENO, &term);

	if (action == IRC_START)
	{
		IRC_LOG("Setting up I/O.");

		struct sigaction	handler;
		
		tcgetattr(STDIN_FILENO, &start);
		handler.sa_handler = ircSigHandler;
		sigaction(SIGINT, &handler, NULL);

		IRC_FLAG_DEL(term.c_lflag, ECHOCTL);
// 		term.c_cc[VMIN] = 0;
		term.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, 0, &term);

		IRC_OK("I/O Setup.");
	}
	else
		tcsetattr(STDIN_FILENO, 0, &start);
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
