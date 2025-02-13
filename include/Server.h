/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 23:20:06 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Channel.h"
#include <map>
# include <sys/socket.h>
# include <irc.h>

# include <vector>

typedef enum:	uint32_t
{
	IRC_STATUS_NULL	=	0U,
	IRC_STATUS_OK	=	1U << 0,
}	serverFlag;

class	Client;

class Server
{
	private:
		uint32_t	_flag;

		int		_port;
		int		_sockfd;
		str		_password;

		std::vector<struct pollfd>	_pollSet;

// 		Client	_clients[IRC_CLIENT_CAP];

		void			_bindSocket() const;
		void			_listenSocket() const;

		bool			_updatePollSet();

		struct pollfd	*_acceptClient();

		void			_handleMessage(Client *client);
		str				_extractCommand(str *source);
		void			_executeCommand(Client *client, const str &command);

	public:
		Server(int port, str password);
		~Server(void);

		void	initSocket();
		void	start();

		void	serverInfo() const;

		GETTER(uint32_t, _flag);
		GETTER(int, _port);
		GETTER(int, _sockfd);

		GETTER_C(uint32_t, _flag);
		GETTER_C(int, _port);
		GETTER_C(int, _sockfd);

		SETTER(uint32_t, _flag);
		SETTER(int, _port);
		SETTER(int, _sockfd);

		EXCEPTION(ServerSocketFailedException,	"socket() failed.");
		EXCEPTION(ServerBindFailedException,	"bind() failed.");
		EXCEPTION(ServerListenFailedException,	"listen() failed.");
		EXCEPTION(ServerPollFailedException,	"poll() failed.");
		EXCEPTION(ServerAcceptFailedException,	"accept() failed.");
	
	private:
		std::map<str, Channel *>	_channelMap;
		void						_addChannel(str channelName, str *key, Client *);

		void						_join(const str, Client *);
		void						_kick(const str, Client *);
		void						_topic(const str, Client *);
		void						_invite(const str, Client *);
		void						_mode(const str, Client *);
};

std::ostream &operator<<(std::ostream &os, const Server &client);

#endif // SERVER_HPP
