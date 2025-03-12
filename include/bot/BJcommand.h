#pragma once

#include <Server.h>
#include <bot/blackjack.h>

void	Server::_blackJackCommands(Client &user, BlackJack *table, const std::vector<str> &param)
{
	if (param[0] == "START")
	{
		return table->start(user);
	}
	if (param[0] == "STOP")
	{
		//TODO unsummon?
		return table->stop(user);
	}
	if (param[0] == "HIT")
		return table->hit(user);
	if (param[0] == "STAND")
		return table->stand(user);
	if (param[0] == "QUIT")
		return table->quit(user);
	if (param[0] == "DOUBLE")
		return table->doubleDown(user);
	if (param[0] == "BET")
	{
		if (param.size() == 0)
			throw _architect.ERR_NEEDMOREPARAMS(user.getTargetName(), "BJ");
		long	ele;
		char	*tmp;
		ele = strtol(param[1].c_str(), &tmp, 10);
		if (errno == ERANGE || ele < 0 || ele > INT_MAX || *tmp)
			throw "TODO parsing bet";
		return table->bet(user, ele);
	}
	throw "invalid cmd bj";
}

IRC_COMMAND_DEF(BJ)
{
	const std::vector<str>	&param = _parsingParam(command);
	std::stringstream		ss;
	BlackJack				*table = client.get_bjTable();

	IRC_LOG("PARAMS SIZE = %lu", param.size());
	if (param.size() == 0)
		goto needMoreParam;
	else if (param[0] == "SUMMON")
	{
		if (table)
			; //already on a table
		ss << "./IRCBot " << _port << " " << _password << " " << client.get_nickname() + " &";
		system(ss.str().c_str());	
		return ;
	}
	try
	{
		_blackJackCommands(client, table, param);
		return ;
	}
	catch (const str e)
	{
		return _send(client, e);
	}
	catch (const char *e)
	{
		IRC_LOG("%s", e);
	}

	needMoreParam:
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "BJ"));
}

