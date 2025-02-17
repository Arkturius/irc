/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/17 16:28:40 by yroussea         ###   ########.fr       */
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

# define	R_CHANNEL_KEY_CHAR	R_CHAR_INV_GROUP(" \x09\x10\x11\x12\x13")

# define	R_CHANNEL_KEY		R_1_TO_Y(R_CHANNEL_KEY_CHAR, 23)

# define	R_CAPTURE_CHANNEL_NAME	R_CAPTURE(R_CHANNEL_NAME)
# define	R_CAPTURE_CHANNEL_KEY	R_CAPTURE(R_CHANNEL_KEY)

class Client;

class Channel
{
	//TODO pour l instant tous est public =
	private:
		str						_name;
		bool					_inviteOnlyChannel;

		bool					_activePassword;
		str						_password;

		int						_userLimit;
		str						_topic;
		//topicSetterNickName
		//topicSetTime
		bool					_topicIsSet;
		bool					_topicPermNeeded;

		std::vector<int>		_fdClient;
		std::vector<int>		_fdAdminClient;

		void	_addClient(int fdClient, int perm);
	
	public:
		Channel(str name, int);
		~Channel(void);

		int		get_size() {return _fdAdminClient.size() + _fdClient.size();}
		void	addClient(int fdClient, str *password);
		int		removeClient(int fdClient); //TODO remove call by server, need to check if size == 0 to delete it

		bool	havePerm(int fdClient);
		void	givePerm(int userClient, int targetClient);
		void	removePerm(int targetClient);

		void	_send(const str &str);

		GETTER(str, _name);
		GETTER(str, _topic);
		GETTER(bool, _topicPermNeeded);
		GETTER(bool, _topicIsSet);
		GETTER(str, _password);
		GETTER(bool, _activePassword);
		GETTER(bool, _inviteOnlyChannel);
		GETTER(std::vector<int>, _fdClient);
		GETTER(std::vector<int>, _fdAdminClient);

		GETTER_C(int, _userLimit);
		GETTER_C(str, _name);
		GETTER_C(str, _topic);
		GETTER_C(bool, _topicPermNeeded);
		GETTER_C(bool, _topicIsSet);
		GETTER_C(str, _password);
		GETTER_C(bool, _activePassword);
		GETTER_C(bool, _inviteOnlyChannel);
		GETTER_C(std::vector<int>, _fdClient);
		GETTER_C(std::vector<int>, _fdAdminClient);

		SETTER(int, _userLimit);
		SETTER(str, _topic);
		SETTER(bool, _topicPermNeeded);
		SETTER(bool, _topicIsSet);
		SETTER(str, _password);
		SETTER(bool, _activePassword);
		SETTER(bool, _inviteOnlyChannel);

		EXCEPTION(InvalidChannelNameException,	"Invalid channel name.");
		EXCEPTION(ClientNotInChannelException,	"Client not in channel.");
		EXCEPTION(InvalidChannelKeyException,	"The Channel Key is incorrect");
		EXCEPTION(InviteOnlyChannelException,	"Its a invite Only channel");
};

#endif // CHANNEL_H
