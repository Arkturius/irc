#ifndef CHANNEL_MODE_H
# define CHANNEL_MODE_H

# include <Server.h>
# include <Channel.h>
# include <Client.h>
# include <cstdlib>
# include <poll.h>
# include <errno.h>
# include <climits>

IRC_COMMAND_DEF(MODE)
{
	UNUSED(command);
	UNUSED(client);
}

bool	Server::modePassword(bool plus, const str &modeArguments, Channel *target, Client *client)
{
	if (!plus && target->get_password() == modeArguments)
		return target->set_activePassword(0), 0;
	else if (plus)
	{
		_seeker.rebuild(R_CHANNEL_KEY);
		_seeker.feedString(modeArguments);
		if (!_seeker.match())
			goto invalidNewPassword;
		target->set_password(modeArguments);
		target->set_activePassword(1);
		return 0;
	}
	goto invalidPassword;

invalidPassword:
	_send(client, _architect.ERR_INVALIDMODEPARAM(client->getTargetName(), target->get_name().c_str(), "k", modeArguments.c_str(), "To disable the password you need to enter the correct password"));
	return 1;
invalidNewPassword:
	_send(client, _architect.ERR_INVALIDMODEPARAM(client->getTargetName(), target->get_name().c_str(), "k", modeArguments.c_str(), "invalid input password"));
	return 1;
}

bool	Server::modePermition(bool plus, const str &modeArguments, Channel *target, Client *client)
{
	//TODO
	// _seeker.rebuild(R_); // client
	_seeker.feedString(modeArguments);
	//match => userTarget
	Client	*userTarget = NULL;
	try
	{
		if (!userTarget)
			throw UnexpectedErrorException();
		if (plus)
			target->givePerm(client->get_pfd()->fd, userTarget->get_pfd()->fd);
		else
			target->removePerm(userTarget->get_pfd()->fd);
		return 0;
	}
	catch (std::exception &e)
	{
		goto userNotInChannel;
	}

userNotInChannel:
	_send(client, _architect.ERR_USERNOTINCHANNEL(client->getTargetName(), modeArguments.c_str(), target->get_name().c_str()));
	return 1;

}

bool	Server::_individualMode(bool plus, char mode, const str &modeArguments, Channel *target, Client *client) //ref
{
	switch (mode)
	{
		case ('o'):
		{
			return modePermition(plus, modeArguments, target, client);
		}
		case ('i'):
			return target->set_inviteOnlyChannel(plus), 0;
		case ('t'):
			return target->set_topicPermNeeded(plus), 0;
		case ('k'):
			return modePassword(plus, modeArguments, target, client);
		case ('l'):
		{
			long	ele;
			char	*tmp;

			ele = strtol(modeArguments.c_str(), &tmp, 10);
			if (errno == ERANGE || ele < 0 || ele > INT_MAX || *tmp)
				goto invalidIntParam;
			if (plus)
				target->set_userLimit(ele);
			else
				target->set_userLimit(INT_MAX);
			return 0;
		}
	}
	_send(client, _architect.ERR_UNKNOWNMODE(client->getTargetName(), mode));
	return 1;

invalidIntParam:
	_send(client, _architect.ERR_INVALIDMODEPARAM(client->getTargetName(), target->get_name().c_str(), "l", modeArguments.c_str(), "The limite sould be a possitiv integer"));
	return 1;
}

void	Server::modeCmdReturn(bool plus, const char &individualModeChar, Channel *target, Client *client)
{
	const str	removeSet[2] = {"remove", "set"};
	str	typeCharMode;
	switch (individualModeChar)
	{
		case ('k'):
			typeCharMode = "channel password"; break ;
		case ('l'):
			typeCharMode = "channel user limite"; break ;
		case ('i'):
			typeCharMode = "channel inviteOnly"; break ;
		case ('t'):
			typeCharMode = "channel TopicRestriction"; break ;
	}
	target->_broadcast(_architect.CMD_MODE(client->get_nickname(), 3, target->get_name().c_str(), removeSet[plus].c_str(), typeCharMode.c_str()));
}

void	Server::_mode(const str command, Client *client)
{
	//<target> [<modestring> [<mode arguments>...]]
	(void)command;(void)client;
	str					targetName;
	str					modeString;
	std::vector<str>	modeArgs;
	Channel				*target;
	//TODO parsing

	target = _getChannelByName(targetName);
	if (!target)
		goto invalidNickChannel;
	if (modeString.size() == 0)
		goto getModeOfChannel;
	try
	{
		if (target->havePerm(client->get_pfd()->fd) == 0)
			goto invalidPermition;
	}
	catch (std::exception &e)
	{
		goto invalidChannel;
	}

	//  +ab argsA argsB
	//  on ne melange pas les - et les + !
	for (size_t i = 1; i < modeString.size(); i++)
	{
		const str	&modeArguments = modeArgs[i - 1];
		const bool	&plus = modeString.c_str()[0] == '+';
		const char	&individualModeChar = modeString.c_str()[i];
		if (_individualMode(plus, individualModeChar, modeArguments, target, client))
			return ;
		modeCmdReturn(plus, individualModeChar, target, client);
	}


getModeOfChannel:
	_sendModeIs(client, target);
	return ;
invalidNickChannel:
	_send(client, _architect.ERR_NOSUCHNICK(client->getTargetName(), targetName.c_str()));
	return ;
invalidPermition:
	_send(client, _architect.ERR_CHANOPRIVSNEEDED(client->getTargetName(), targetName.c_str()));
	return ;
invalidChannel:
	_send(client, _architect.ERR_NOTONCHANNEL(client->getTargetName(), targetName.c_str()));
	return ;
}
#endif
