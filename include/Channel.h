/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/11 14:00:10 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_H
# define CHANNEL_H

# include <irc.h>
# include <vector>

# define REGEX_APPROVE_CHANNEL_NAME "[#&].[a-zA-Z]{1,200}"

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

		int	get_size() {return _fdAdminClient.size() + _fdClient.size();}
	
	public:
		Channel(str name, int);
		~Channel(void);

		void	addClient(int fdClient, int perm);
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
										 //
		GETTER_C(bool, _inviteOnlyChannel);
		SETTER(bool, _inviteOnlyChannel);

		GETTER_C(str, _topic);
		GETTER_C(str, _name);
		GETTER_C(std::vector<int>, _fdClient);


		EXCEPTION(invalideChannelNameException,	"The Channel Name is Invalide");
		EXCEPTION(clientNotInChannelException,	"The Client is not in the Channel");
};

#endif // CHANNEL_H
