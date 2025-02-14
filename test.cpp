/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:23:00 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 18:50:57 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <regex.h>

#include <irc.h>
#include <RParser.h>

int main(void)
{
	const char	*pattern = R_START_STRING R_CAPTURE(R_1_OR_MORE("a"));
	const char	*string = "aa bbbb aaaa bbb aaa bbbb ab abbba abba bbbb aaa a a";

	RParser	pr(pattern);

	IRC_LOG("String %s", pr.match(string) ? "found" : "not found");
}
