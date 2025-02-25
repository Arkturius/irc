#ifndef ATARGET_H
# define ATARGET_H

# include <irc.h>

class ATarget
{
	public:
		ATarget(void) {};
		virtual ~ATarget(void) {};

		virtual void sendMsg(const str &) const = 0;
};

#endif
