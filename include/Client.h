/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 11:56:54 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 00:47:03 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <irc.h>
# include <map>

class Channel;

typedef enum:	uint32_t
{
	IRC_CLIENT_CONNECTED	=	1U,
	IRC_CLIENT_EOT			=	1U << 1,
	IRC_CLIENT_EOF			=	1U << 2,
}	clientFlag;

#define COMMA	,

class Client
{
	private:
		uint32_t	_flag;
		str			_username;
		str			_nickname;

		str			_buffer;

		struct pollfd				*_pfd;
		std::map<str, Channel *>	_channelMap;

		size_t	_readToBuffer(void);

	public:
		Client(void);
		Client(struct pollfd *pfd);
		~Client(void);

		void	readBytes(void);
		void	resetBuffer(void);

		void	disconnect(void);
		void	joinChannel(Channel *);
		void	leaveChannel(Channel *);
		
		GETTER(uint32_t, _flag);
		GETTER(str, _username);
		GETTER(str, _nickname);
		GETTER(str, _buffer);
		GETTER(struct pollfd *, _pfd);
		GETTER(std::map<str COMMA Channel *>, _channelMap);
		
		GETTER_C(uint32_t, _flag);
		GETTER_C(str, _username);
		GETTER_C(str, _nickname);
		GETTER_C(str, _buffer);
 		GETTER_C(std::map<str COMMA Channel *>, _channelMap);

		SETTER(uint32_t, _flag);
		SETTER(str, _username);
		SETTER(str, _nickname);
};

#endif // CLIENT_HPP
