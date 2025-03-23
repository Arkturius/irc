/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCBot.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 13:11:04 by rgramati          #+#    #+#             */
/*   Updated: 2025/03/23 19:17:09 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "irc.h"
#include "ircRegex.h"
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>

#include <Client.h>
#include <IRCSeeker.h>
#include <sys/socket.h>

#define IRC_BOT_LIMIT	10

class IRCBot
{
	private:
		uint32_t	_flag;
		int32_t		_sockfd;
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
			IRC_WARN("BOT sending - <%s>", msg.c_str());
			write(_serverFd, msg.c_str(), msg.length());
		}

		void	_configTable()
		{
			str tablePayload;

			tablePayload += "JOIN #" + _table + "\r\n";
			tablePayload += "MODE #" + _table + " +i" + "\r\n";
			tablePayload += "INVITE " + _summoner + " " + "#" + _table + "\r\n";

			_send(tablePayload);

			return _send("PONG ft_irc_bot_accept");
		}

		void	_execute(const str &command)
		{
			_seeker.feedString(command);
			_seeker.rebuild(R_CAPTURE(":localhost"));
			
			if (!_seeker.consume())
				return ;

			_seeker.rebuild(R_MIDDLE_PARAM);
			_seeker.consumeMany();

			const std::vector<str>	&argv = _seeker.get_matches();
			const str				&mnemo = argv[0];

			IRC_LOG("Mnemonic = %s", mnemo.c_str());

			if (mnemo == "PING" && argv.size() == 2)
			{
				const str	&token = argv[1];
				
				if (token == "ft_irc")				{ return _send("PONG " + argv[1] + "\r\n"); }
				if (token == "ft_irc_bot_accept")	{ return _configTable(); }
			}
		}

		void	_handleMessage()
		{
			IRC_WARN("received message : [%s]", _buffer.c_str());

			str					buffer = get_buffer();
			std::vector<str>	commands;

			while (true)
			{
				const size_t	linefeed = buffer.find("\n");
				if (!linefeed || linefeed == str::npos)
					break ;
				const size_t	crlf = (buffer.at(linefeed - 1) == '\r');
				const str		command = buffer.substr(0, linefeed - crlf);

				commands.push_back(command);
				buffer = buffer.substr(linefeed + 1, buffer.length());
			}
			_buffer.clear();

			for (IRC_AUTO it = commands.begin(); it != commands.end(); ++it)
				_execute(*it);
		}

		int	_connect(const str &port)
		{
			IRC_LOG("Resolving server address...");

			struct addrinfo	hints;
			struct addrinfo	*result;

			IRC_BZERO(hints);
			int err = getaddrinfo("localhost", port.c_str(), &hints, &result);
			if (err)
				throw AddrInfoFailedException();

			_sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (_sockfd == -1)
				throw BotSocketFailedException();

			IRC_LOG("connecting to localhost:%s ...", port.c_str());
			if (connect(_sockfd, result->ai_addr, result->ai_addrlen) != -1)
			{
				freeaddrinfo(result);
				return (_sockfd);
			}
			close(_sockfd);
			freeaddrinfo(result);
			throw ConnectFailedException();
		}

	public:

		IRCBot(const str &nickname, const str &port, const str &pass, const str &summoner): _flag(0), _summoner(summoner)
		{
			IRC_LOG("Bot constructor called.");

			static uint32_t	id = 0;
			if (id > IRC_BOT_LIMIT)
				throw BotLimitExceededException();

			int flags = fcntl(_serverFd, F_GETFL, 0);
			fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK);
			
			_serverFd = _connect(port);

			IRC_LOG("new BlackJack dealer !");

			std::stringstream	ss;
			ss << id;

			str authPayload;
			authPayload += "PASS " + pass + "\r\n";
			authPayload += "NICK " + nickname + "\r\n"; 
			authPayload += "USER " + summoner + " 0 * :real_" + nickname + "\r\n";
			_send(authPayload);

			_table = summoner + "_table";
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
			IRC_WARN("BOT STARTED");
			while (!IRC_FLAG_GET(_flag, IRC_CLIENT_EOF))
			{
				IRC_WARN("READING FROM SERVER...");
				readBytes();
				if (IRC_FLAG_GET(_flag, IRC_CLIENT_EOT))
					_handleMessage();
			}
			IRC_WARN("BOT STOPPED");
			str quitPayload = "QUIT :No more bets !\r\n";
			_send(quitPayload);
		}

		GETTER(str, _buffer);

		GETTER_C(str, _buffer);

	EXCEPTION(BotLimitExceededException,	"bot limit exceeded.");
	EXCEPTION(AddrInfoFailedException,		"addrinfo() failed.");
	EXCEPTION(BotSocketFailedException,		"socket() failed.");
	EXCEPTION(ConnectFailedException,		"connect() failed.");
};
