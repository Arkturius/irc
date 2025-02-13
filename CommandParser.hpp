/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:42:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 16:58:20 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDPARSER_HPP
# define COMMANDPARSER_HPP

# include <irc.h>
# include <ircRegex.h>
# include <regex.h>
# include <vector>

class CommandParser
{
	private:
		uint32_t			_flag;
		regex_t				_state;
		std::vector<str>	_matches;

	public:
		void	parseCommand(const str &command, size_t nmatch);
		str		nextMatch();

		CommandParser(void);
		CommandParser(const str &pattern);
		~CommandParser(void);

		EXCEPTION(CPInvalidPatternException,	"Invalid regex pattern.");
};

#endif // COMMANDPARSER_HPP
