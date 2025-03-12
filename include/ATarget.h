#pragma once

# include <irc.h>

class ATarget
{
	protected:
		str		_targetName;
		bool	_targetIsChannel;

	public:
		ATarget(void): _targetIsChannel(0) {};
		ATarget(str name): _targetName(name), _targetIsChannel(1) {};

		ATarget &operator=(ATarget const &other) 
		{
			if (this != &other)
			{
				this->_targetName = other.get_targetName();
				this->_targetIsChannel = other.get_targetIsChannel();
			}
			return *this;
		}
		virtual ~ATarget(void) {};

		virtual void	sendMsg(const str &) const = 0;
		virtual void	ignoredFlag(int fd, int32_t ignored) = 0;
		const char		*getTargetName() { return _targetName.c_str(); }

		SETTER(str, _targetName);
		GETTER_C(str, _targetName);
		GETTER_C(bool, _targetIsChannel);
};
