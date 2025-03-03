#pragma once

# include <irc.h>

class ATarget
{
	protected:
		str	_targetName;
		int	_ignoredFd;

	public:
		ATarget(void) {};
		virtual ~ATarget(void) {};

		virtual void sendMsg(const str &) const = 0;
		const char	*getTargetName() { return _targetName.c_str(); }	

		SETTER(str, _targetName);
		SETTER(int, _ignoredFd);

		GETTER(int, _ignoredFd);
		GETTER_C(int, _ignoredFd);
};
