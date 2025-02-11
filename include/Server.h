/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/11 12:42:01 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <irc.h>
#include <vector>

typedef enum
{
	IRC_STATUS_NULL	=	0,
	IRC_STATUS_OK	=	1U << 0,
}	ServerFlag;

class Server
{
	private:
		ServerFlag	_flag;

		int		_port;
		int		_sockfd;
		str		_password;

		std::vector<struct pollfd>	_pollSet;

		void	_bindSocket() const;
		void	_listenSocket() const;

		bool	_updatePollSet();

		void	_acceptClient();
		void	_disconnectClient(size_t id);

	public:
		Server(int port, str password);
		~Server(void);

		void	initSocket();
		void	start();

		void	serverInfo() const;

		GETTER(int, _port);
		GETTER(int, _sockfd);
		GETTER_C(int, _port);
		GETTER_C(int, _sockfd);

		SETTER(int, _port);
		SETTER(int, _sockfd);

		EXCEPTION(ServerSocketFailedException,	"socket() failed.");
		EXCEPTION(ServerBindFailedException,	"bind() failed.");
		EXCEPTION(ServerListenFailedException,	"listen() failed.");
		EXCEPTION(ServerPollFailedException,	"poll() failed.");
		EXCEPTION(ServerAcceptFailedException,	"accept() failed.");
};

std::ostream &operator<<(std::ostream &os, const Server &client);

#endif // SERVER_HPP
