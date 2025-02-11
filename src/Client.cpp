/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 11:56:54 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/11 12:14:03 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Client.h>

Client::Client(struct pollfd *pfd): _pfd(pfd)
{
	IRC_LOG("Client constructor called.");
}

Client::~Client(void)
{
	IRC_LOG("Client destructor called.");
}

std::ostream &operator<<(std::ostream &os, const Client &client)
{
	IRC_LOG("Client");
	return (os);
}
