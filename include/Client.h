/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 11:56:54 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/11 12:42:48 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <irc.h>
#include <map>
# include <vector>

class Channel;

typedef enum
{
	IRC_CLIENT_CONNECTED	=	1,
	IRC_CLIENT_OPERATOR		=	1 << 1,
}	clientFlag;

#define COMMA	,

class Client
{
	private:
		clientFlag	_flag;
		str			_username;
		str			_nickname;

		struct pollfd				*_pfd;
		std::map<str, Channel &>	_channelMap;

	public:
		Client(struct pollfd *pfd);
		~Client(void);

		void	joinChannel(Channel *);
		void	leaveChannel(Channel *);

		GETTER(struct pollfd *, _pfd);
		GETTER(std::map<str COMMA Channel &>, _channelMap);
		
 		GETTER_C(std::map<str COMMA Channel &>, _channelMap);

		SETTER(str, _username);
		SETTER(str, _nickname);
};

#endif // CLIENT_HPP
