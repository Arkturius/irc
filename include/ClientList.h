/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientList.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 16:35:38 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/11 16:52:25 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTLIST_H
# define CLIENTLIST_H

# include <Client.h>
# include <vector>

# define FD_OFFSET	4

/**
 * This dumb class is provided under the rgramati/babonnet™ stupid license 
 * Dont copy unless youre an idiot.
 */

class	ClientList: public std::vector<Client>
{
	private:

	public:

		Client &operator[](int n)
		{
			if (n < FD_OFFSET)
				throw StandardFileException();
			return ClientList::vector::operator[](n - FD_OFFSET);
		}

		const Client &operator[](int n) const
		{
			if (n < FD_OFFSET)
				throw StandardFileException();
			return ClientList::vector::operator[](n - FD_OFFSET);
		}

		EXCEPTION(StandardFileException, "");
};

#endif
