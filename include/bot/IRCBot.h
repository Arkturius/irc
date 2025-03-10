/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCBot.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 13:11:04 by rgramati          #+#    #+#             */
/*   Updated: 2025/03/06 14:08:01 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ircRegex.h"
#include <sstream>

#include <Client.h>
#include <IRCSeeker.h>

#define IRC_BOT_LIMIT	10

class IRCBot
{
	private:
		uint32_t	_flag;
		int32_t		_serverFd;

		IRCSeeker	_seeker;

		str			_summoner;
		str			_table;

		str			_buffer;

		int	_read(void)
		{
			char	tmp[1024] = {0};
			int 	bytes = 0;

			bytes = read(_serverFd, tmp, sizeof(tmp) - 1);
			if (bytes)
			{
				tmp[bytes] = 0;
				_buffer += str(tmp);
			}
			return (bytes);
		}

		void	_send(const str &msg)
		{
			write(_serverFd, msg.c_str(), msg.length());
		}

		void	_handleMessage()
		{
			_seeker.feedString(_buffer);
			_seeker.rebuild(R_COMMAND_MNEMO);
			
			if (!_seeker.consume())
				return ;

			IRC_ERR("BOT COMMAND RECEIVED: <%s>", _seeker.get_matches()[0].c_str());
		}

	public:

		IRCBot(uint32_t serverFd, const str &pass, const str &summoner): _flag(0), _serverFd(serverFd), _summoner(summoner)
		{
			IRC_LOG("Bot constructor called.");

			static uint32_t	id = 0;
			if (id > IRC_BOT_LIMIT)
				throw BotLimitExceededException();

			IRC_LOG("new BlackJack dealer !");

			std::stringstream	ss;
			ss << id;

			str nickname = "dealer" + ss.str();

			str authPayload;
			authPayload += "PASS " + pass + "\r\n";
			authPayload += "NICK " + nickname + "\r\n"; 
			authPayload += "USER " + nickname + " * 0 :real_" + nickname + "\r\n";
			authPayload += "PONG ft_irc\r\n";
			_send(authPayload);

			_table = summoner + "'s table'";
			id++;
		}

		void	readBytes(void)
		{
			int	bytes;

			bytes = _read();
			if (bytes == 0 || _buffer.length() == 0)
			{
				IRC_FLAG_SET(_flag, IRC_CLIENT_EOF);
				return ;
			}
			if (_buffer.find("\n") != std::string::npos)
			{
				IRC_FLAG_SET(_flag, IRC_CLIENT_EOT);
				return ;
			}
		}

		void	start(void)
		{
			while (!IRC_FLAG_GET(_flag, IRC_CLIENT_EOF))
			{
				readBytes();
				if (IRC_FLAG_GET(_flag, IRC_CLIENT_EOT))
					_handleMessage();
			}
			str quitPayload = "QUIT :No more bets !\r\n";
			_send(quitPayload);
		}

	EXCEPTION(BotLimitExceededException, "bot limit exceeded.");
};
