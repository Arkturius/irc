#pragma once

# include <irc.h>

class ATarget
{
	protected:
		str	_targetName;

	public:
		ATarget(void) {};
		virtual ~ATarget(void) {};

		virtual void sendMsg(const str &) const = 0;
		const char	*getTargetName() { return _targetName.c_str(); }	

		SETTER(str, _targetName);
};
