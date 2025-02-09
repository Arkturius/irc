/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:04:01 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/09 21:32:59 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_H
# define IRC_H

# include <stdio.h>
# include <string>
# include <iostream>
# include <exception>

typedef std::string str;

# define	UNUSED(X)			(void)(X)
# define	UNIMPLEMENTED(X)	assert(0 && "Unimplemented")

# define	BOLD(X)				"\033[1m"X"\033[22m"
# define 	ITALIC(X)			"\033[3m"X"\033[23m"
# define 	RED					"\033[31m"
# define 	GREEN				"\033[32m"
# define 	YELLOW				"\033[33m"
# define 	CYAN				"\033[34m"
# define 	MAGENTA				"\033[35m"
# define 	GRAY				"\033[90m"
# define 	RESET				"\033[0m"
# define 	COLOR(C, X)			C X RESET

# define _IRC_LOG(c, t, msg, ...)	printf(BOLD(COLOR(c,"%6s"))" > "msg"\n", t, ##__VA_ARGS__)

# define IRC_LOG(msg, ...)	_IRC_LOG(YELLOW, "info:", msg, ##__VA_ARGS__)
# define IRC_OK(msg, ...)	_IRC_LOG(GREEN,	 "done:", msg, ##__VA_ARGS__)
# define IRC_ERR(msg, ...)	_IRC_LOG(RED,	 "error:", msg, ##__VA_ARGS__)

# define EXCEPTION(n, m)	class n:public std::exception { inline const char *what() const throw() { return (m) ; } ; }

# define GETTER(t, x)		inline t &get##x() { return this->x ; }				\
							inline const t &get##x() const { return this->x ; }

# define SETTER(c, t, x)	inline t c::set##x(t other) { this->x = other ; }

# define auto __auto_type

#endif
