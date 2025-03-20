#pragma once

#include <Server.h>
#include <bot/blackjack.h>
#include <vector>

void	Server::_clientPartBj(Client &client)
{
	BlackJack				*table = client.get_bjTable();
	if (!table)
		return ;
	const std::vector<str>	vec;
	_blackJackCommands(client, table, vec);
}

void	Server::_blackJackCommands(Client &user, BlackJack *table, const std::vector<str> &param)
{
	//invite other player 2: INVITE work!
	if (!table)
		throw "bro faut summon dabord";
	if (param.size() == 0)
	{
		if (user.get_nickname() != table->get_dealer().get_client().get_username())
			return table->quit(user);
		//TODO unsummon!!! il tourne dans le vide je crois?
		table->stop(user);
		_disconnectClient(table->get_dealer().get_client());
		delete table;
		return ;
	}
	if (param[0] == "START")
		return table->start(user);
	if (param[0] == "HIT")
		return table->hit(user);
	if (param[0] == "STAND")
		return table->stand(user);
	if (param[0] == "DOUBLE")
		return table->doubleDown(user);
	if (param[0] == "BET")
	{
		if (param.size() == 1)
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
			return ; //already on a table
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
		return ;
	}

	needMoreParam:
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "BJ"));
}

