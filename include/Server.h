/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/10 18:35:47 by rgramati         ###   ########.fr       */
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

# define IRC_FLAG_SET(w, f)	(w) = (ServerFlag)((w) | (f))
# define IRC_FLAG_DEL(w, f)	(w) = (ServerFlag)((w) & ~(f))
# define IRC_FLAG_GET(w, f)	((w) & (f))

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

		EXCEPTION(ServerSocketFailedException,	"socket() failed.");
		EXCEPTION(ServerBindFailedException,	"bind() failed.");
		EXCEPTION(ServerListenFailedException,	"listen() failed.");
		EXCEPTION(ServerPollFailedException,	"poll() failed.");
		EXCEPTION(ServerAcceptFailedException,	"accept() failed.");
};

#endif // SERVER_HPP
