/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/11 12:46:46 by yroussea         ###   ########.fr       */
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
		int						userLimit;
		str						_name;
		str						_topic;
		std::vector<int>		_fdClient;
	
	public:
		Channel(str name);
		~Channel(void);

		void	addClient(int fdClient);
		void	removeClient(int fdClient);


		GETTER(str, _name);
		GETTER(str, _topic);
		SETTER(str, _topic);
		GETTER(std::vector<int>, _fdClient);

		EXCEPTION(invalideChannelNameException,	"The Channel Name is Invalide");
};

std::ostream	&operator<<(std::ostream &, Channel const &);

#endif // CHANNEL_H
