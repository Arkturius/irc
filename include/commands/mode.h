#pragma once

# include <Server.h>
# include <cstdlib>
# include <errno.h>
# include <climits>

bool	Server::_individualMode(bool plus, char mode, const str &modeArguments, Channel *target, Client &client)
{
	switch (mode)
	{
		case ('o'):
		{
			str TargetName;

			_seeker.rebuild(R_NICKNAME);
			_seeker.feedString(modeArguments);
			_seeker.consumeMany();
			if (_seeker.get_matchCount() == 1)
			{
				TargetName = _seeker.get_matches()[0];
				Client	*userTarget = _getClientByName(TargetName);
				try
				{
					if (!userTarget)
						goto noSuchNick;
					if (plus)
						target->givePerm(client.get_fd(), userTarget->get_fd());
					else
						target->removePerm(userTarget->get_fd());
					return 0;
				}
				catch (std::exception &e)
				{
					goto userNotInChannel;
				}
			}
			return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "MODE (o)")), 1;
		noSuchNick:
			return _send(client, _architect.ERR_USERNOTINCHANNEL(client.getTargetName(), modeArguments.c_str(), TargetName.c_str())), 1;
		userNotInChannel:
			return _send(client, _architect.ERR_USERNOTINCHANNEL(client.getTargetName(), modeArguments.c_str(), TargetName.c_str())), 1;
		}
		case ('i'):
		{
			if (plus)
				return IRC_FLAG_SET(target->get_flag(), IRC_CHANNEL_INVITE_ONLY), 0;
			else
				return IRC_FLAG_DEL(target->get_flag(), IRC_CHANNEL_INVITE_ONLY), 0;
		}
		case ('t'):
		{
			if (plus)
				return IRC_FLAG_SET(target->get_flag(), IRC_CHANNEL_TOPIC_PERM), 0;
			else
				return IRC_FLAG_DEL(target->get_flag(), IRC_CHANNEL_TOPIC_PERM), 0;
		}
		case ('k'):
		{	
			if (!plus && (!IRC_FLAG_GET(target->get_flag(), IRC_CHANNEL_ACTIVE_PASSWORD) || target->get_password() == modeArguments))
				return IRC_FLAG_DEL(target->get_flag(), IRC_CHANNEL_ACTIVE_PASSWORD), 0;
			else if (plus)
			{
				_seeker.rebuild(R_CAPTURE_CHANNEL_KEY);
				_seeker.feedString(modeArguments);
				if (!_seeker.match())
					goto invalidNewPassword;
				target->set_password(modeArguments);
				return IRC_FLAG_SET(target->get_flag(), IRC_CHANNEL_ACTIVE_PASSWORD), 0;
			}
			goto invalidPassword;

invalidPassword:
			return _send(client, _architect.ERR_INVALIDMODEPARAM(client.getTargetName(), target->getTargetName(),
						"k", modeArguments.c_str(), "To disable the password you need to enter the correct password")), 1;
invalidNewPassword:
			return _send(client, _architect.ERR_INVALIDMODEPARAM(client.getTargetName(), target->getTargetName(),
						"k", modeArguments.c_str(), "invalid input password")), 1;
		}
		case ('l'):
		{
			long	ele;
			char	*tmp;

			if (modeArguments.size() == 0)
				goto needMoreParam;
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
	_send(client, _architect.ERR_UNKNOWNMODE(client.getTargetName(), mode));
	return 1;

needMoreParam:
	_send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "MODE (l)"));
	return 1;
invalidIntParam:
	_send(client, _architect.ERR_INVALIDMODEPARAM(client.getTargetName(), target->getTargetName(), "l", modeArguments.c_str(), "The limite sould be a possitiv integer"));
	return 1;
}

void	Server::_sendModeIs(Client &client, Channel *channel)
{
	str					modeArgs = "";
	std::stringstream	ss;
	const int			userLimit = channel->get_userLimit();
	str					modeis = "+";

	if (IRC_FLAG_GET(channel->get_flag(), IRC_CHANNEL_INVITE_ONLY))
		modeis += "i";
	if (IRC_FLAG_GET(channel->get_flag(), IRC_CHANNEL_TOPIC_PERM))
		modeis += "t";
	if (IRC_FLAG_GET(channel->get_flag(), IRC_CHANNEL_ACTIVE_PASSWORD))
		modeis += "k";
	if (userLimit != INT_MAX)
	{
		ss << userLimit;
		modeis += "l";
		modeArgs += ss.str();
	}
	_send(client, _architect.RPL_CHANNELMODEIS(client.getTargetName(), channel->getTargetName(), modeis.c_str(), modeArgs.c_str()));
}

IRC_COMMAND_DEF(MODE)
{
	str					targetName;
	str					modeString;
	Channel				*target;
	std::vector<str>	argv;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();
	argv = _seeker.get_matches();
	if (!argv.size())
		goto needMoreParam;

	targetName = argv[0];
	target = _getChannelByName(targetName);
	if (!target)
		goto invalidNickChannel;

	if (argv.size() == 1)
		goto getModeOfChannel;
	modeString = argv[1];
	try
	{
		if (target->havePerm(client.get_fd()) == 0)
			goto invalidPermition;
	}
	catch (std::exception &e)
	{
		goto invalidChannel;
	}

	for (size_t i = 1; i < modeString.size(); i++)
	{
		const bool	&plus = modeString.c_str()[0] == '+';
		const str	&modeArguments = argv[i + 1];
		const char	&individualModeChar = modeString.c_str()[i];
		if (_individualMode(plus, individualModeChar, modeArguments, target, client))
			return ;
		const str	removeSet = str(plus ? "+" : "-") + individualModeChar;
		switch (individualModeChar)
		{
			case ('k'):
				return target->sendMsg(_architect.CMD_MODE(client.get_nickname(), target->getTargetName(), removeSet.c_str(), modeArguments.c_str()));
			default:
				return target->sendMsg(_architect.CMD_MODE(client.get_nickname(), target->getTargetName(), removeSet.c_str()));
		}
	}
	return ;

needMoreParam:
	_send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "MODE"));
	return ;
getModeOfChannel:
	_sendModeIs(client, target);
	return ;
invalidNickChannel:
	_send(client, _architect.ERR_NOSUCHNICK(client.getTargetName(), targetName.c_str()));
	return ;
invalidPermition:
	_send(client, _architect.ERR_CHANOPRIVSNEEDED(client.getTargetName(), targetName.c_str()));
	return ;
invalidChannel:
	_send(client, _architect.ERR_NOTONCHANNEL(client.getTargetName(), targetName.c_str()));
	return ;
}

