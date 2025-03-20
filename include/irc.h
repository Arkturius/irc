#pragma once

# include <assert.h>
# include <stdio.h>
# include <stdint.h>
# include <cstring>
# include <iostream>
# include <exception>
# include <poll.h>

typedef std::string str;

# define	UNUSED(X)			(void)(X)
# define	UNIMPLEMENTED(X)	assert(0 && "Unimplemented : " X)

# define	BOLD(X)				"\033[1m" X "\033[22m"
# define 	ITALIC(X)			"\033[3m" X "\033[23m"
# define 	RED					"\033[31m"
# define 	GREEN				"\033[32m"
# define 	YELLOW				"\033[33m"
# define 	CYAN				"\033[34m"
# define 	MAGENTA				"\033[35m"
# define 	GRAY				"\033[90m"
# define 	RESET				"\033[0m"
# define 	COLOR(C, X)			C X RESET

# define _IRC_LOG(c, t, msg, ...)	printf(BOLD(COLOR(c,"%8s")) " > " msg "\n", t, ##__VA_ARGS__)

# ifdef IRC_VERBOSE
#  define IRC_LOG(msg, ...)		_IRC_LOG(CYAN,       "log:", msg, ##__VA_ARGS__)
#  define IRC_WARN(msg, ...)	_IRC_LOG(YELLOW, "warning:", msg, ##__VA_ARGS__)
#  define IRC_OK(msg, ...)		_IRC_LOG(GREEN,	    "done:", msg, ##__VA_ARGS__)
# else
#  define IRC_LOG(msg, ...)
#  define IRC_WARN(msg, ...)
#  define IRC_OK(msg, ...)
#endif

# define IRC_ERR(msg, ...)		_IRC_LOG(RED,	   "error:", msg, ##__VA_ARGS__)

# define IRC_SEEKER		BOLD(ITALIC(COLOR(RED,"<Seeker> ")))
# define IRC_ARCHITECT	BOLD(ITALIC(COLOR(GREEN,"<Architect> ")))
# define IRC_ANALYST	BOLD(ITALIC(COLOR(CYAN,"<Analyst> ")))

# define IRC_FLAG_SET(w, f)	(w) = ((w) | (f))
# define IRC_FLAG_DEL(w, f)	(w) = ((w) & ~(f))
# define IRC_FLAG_GET(w, f)	((w) & (f))

# define EXCEPTION(n, m)	class n:public std::exception { inline const char *what() const throw() { return (m) ; } ; }

# define IRC_CATCH			catch (std::exception &e) { IRC_ERR("%s", e.what()); }

# define GETTER(t, x)		inline t &get##x() { return this->x ; }
# define GETTER_C(t, x)		inline const t &get##x() const { return this->x ; }

# define SETTER(t, x)		inline void set##x(t other) { this->x = other ; }

# define IRC_BZERO(X)		std::memset(&(X), 0, sizeof(X))

# define IRC_AUTO __auto_type

# define IRC_CLIENT_CAP		9

# define	IRC_ERRNO_NOT_FATAL(e)	(e == ENETDOWN || e == EPROTO || e == ENOPROTOOPT || e == EHOSTDOWN || e == ENONET || e == EHOSTUNREACH || e == EOPNOTSUPP || e == ENETUNREACH)
