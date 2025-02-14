/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RParser.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:47:39 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/14 18:18:11 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RPARSER_HPP
# define RPARSER_HPP

# include <irc.h>

# include <regex.h>
# include <ircRegex.h>
#include <vector>

class RParser
{
	private:
		regex_t				_expr;
		str					_pattern;

		size_t				_matchIndex;
		size_t				_matchCount;
		std::vector<str>	_matches;

		void				_findNext(const str &source);

	public:

		bool	match(const str &source);
		void	findall(const str &source);

		RParser(const str &pattern);
		~RParser(void);

		GETTER(size_t, _matchIndex);
		GETTER(size_t, _matchCount);
		GETTER(std::vector<str>, _matches);

		GETTER_C(size_t, _matchIndex);
		GETTER_C(size_t, _matchCount);
		GETTER_C(std::vector<str>, _matches);

	EXCEPTION(InvalidPatternException, "invalid REGEX pattern.");
};

#endif // RPARSER_HPP
