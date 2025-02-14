/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/14 17:16:26 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_H
# define CHANNEL_H

# include <vector>
# include <ircRegex.h>
# include <irc.h>

# define	R_CHANNEL_PREFIX	R_CHAR_GROUP("&#+")
# define	R_CHANNEL_ID		"!" R_X_EXACT(R_CHAR_GROUP(R_DIGIT_RANGE R_UPPER_RANGE),5)
# define	R_CHANNEL_CHAR		R_CHAR_INV_GROUP(" \x07,")

# define	R_CHANNEL_NAME		R_ALTERNATION											\
								(														\
									R_CHANNEL_PREFIX	R_1_TO_Y(R_CHANNEL_CHAR, 49),	\
									R_CHANNEL_ID		R_1_TO_Y(R_CHANNEL_CHAR, 45)	\
								)

# define	R_CAPTURE_CHANNEL_NAME	R_CAPTURE(R_CHANNEL_NAME)

class Client;

class Channel
{
	private:
		str						_name;
		bool					_inviteOnlyChannel;

		bool					_activePassword;
		str						_password;

		int						_userLimit;
		str						_topic;
		bool					_topicPermNeeded;

		std::vector<int>		_fdClient;
		std::vector<int>		_fdAdminClient;

		int		get_size() {return _fdAdminClient.size() + _fdClient.size();}
		void	_addClient(int fdClient, int perm);
	
	public:
		Channel(str name, int);
		~Channel(void);

		void	addClient(int fdClient, str *password);
		int		removeClient(int fdClient); //TODO remove call by server, need to check if size == 0 to delete it

		bool	havePerm(int fdClient);
		void	givePerm(int userClient, int targetClient);
		void	removePerm(int targetClient);

		SETTER(int, _userLimit);

		SETTER(str, _topic);
		SETTER(bool, _topicPermNeeded);
		GETTER_C(bool, _topicPermNeeded);

		SETTER(str, _password);
		SETTER(bool, _activePassword); //TODO set when set password
		GETTER_C(bool, _activePassword); //TODO Check when join

		GETTER_C(bool, _inviteOnlyChannel);
		SETTER(bool, _inviteOnlyChannel);

		GETTER_C(str, _topic);
		GETTER_C(str, _name);
		GETTER_C(std::vector<int>, _fdClient);

		EXCEPTION(InvalidChannelNameException,	"Invalid channel name.");
		EXCEPTION(ClientNotInChannelException,	"Client not in channel.");
		EXCEPTION(InvalidChannelKeyException,	"The Channel Key is incorrect");
};

#endif // CHANNEL_H
