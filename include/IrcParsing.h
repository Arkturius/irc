#pragma once

# include <irc.h>
# include <Server.h>

std::vector<str>	Server::_parsingParam(const str &command)
{
	std::vector<str>	result;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();
	std::vector<str>	&midlleParam = _seeker.get_matches();

	result.insert(result.begin(), midlleParam.begin(), midlleParam.end());

	_seeker.rebuild(R_TRAILING_PARAM);
	_seeker.consumeMany();
	std::vector<str>	&trailingParam = _seeker.get_matches();

	result.insert(result.end(), trailingParam.begin(), trailingParam.end());
	return result;
}

