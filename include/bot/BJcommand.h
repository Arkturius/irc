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
	if (!table)
		throw "bro faut summon dabord";
	if (param.size() == 0)
	{
		if (user.get_nickname() == table->get_dealer().get_client().get_username())
		{
			table->stop(user);
			std::map<int, Hand *>	&players = table->get_players();
			std::map<int, Hand *>::iterator next;
			for (IRC_AUTO it = players.begin(); it != players.end(); it = next)
			{
				next = it;
				next++;
				Client	&client = it->second->get_client();
				if (client.get_fd() != user.get_fd())
					_commandKICK(client, str(" #") + user.get_nickname() + "_table");
				client.set_bjTable(0);
				delete it->second;
				players.erase(it);
			}
			players.clear();
			_disconnectClient(table->get_dealer().get_client());
		}
		else if (user.get_nickname() == table->get_dealer().get_client().get_nickname())
			delete table;
		else
		{
			table->quit(user);
			std::map<int, Hand *>	&players = table->get_players();
			int fd = user.get_fd();
			IRC_AUTO	it = players.find(fd);
			user.set_bjTable(0);
			delete it->second;
			players.erase(it);
		}
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
	static					int dealerId = 0;
	const std::vector<str>	&param = _parsingParam(command);
	std::stringstream		ss;
	BlackJack				*table = client.get_bjTable();

	IRC_LOG("PARAMS SIZE = %lu", param.size());
	if (param.size() == 0)
		goto needMoreParam;
	else if (param[0] == "SUMMON")
	{
		if (table)
			return ;
		IRC_LOG("Summoning blackjack dealer.");
		ss << "./IRCBot " << "dealer" << dealerId++ << " " << _port << " " << _password << " " << client.get_nickname() + " &";
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

