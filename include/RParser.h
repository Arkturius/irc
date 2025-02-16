/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RParser.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 17:47:39 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/16 23:46:29 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RPARSER_HPP
# define RPARSER_HPP

# include <irc.h>

# include <regex.h>
# include <ircRegex.h>
# include <vector>

class RParser
{
	private:
		regex_t				_expr;
		str					_pattern;

		size_t				_matchIndex;
		size_t				_matchCount;
		std::vector<str>	_matches;

		void				_reset(void);

		void				_findNext(const str &source, size_t nmatch);

	public:
		void	rebuild(const str &pattern);

		bool	match(const str &source);
		void	capture(const str &source, size_t n);
		void	findall(const str &source);

		RParser(void);
		~RParser(void);
		RParser(const str &pattern);

		GETTER(size_t, _matchIndex);
		GETTER(size_t, _matchCount);
		GETTER(std::vector<str>, _matches);

		GETTER_C(size_t, _matchIndex);
		GETTER_C(size_t, _matchCount);
		GETTER_C(std::vector<str>, _matches);

	EXCEPTION(InvalidPatternException, "invalid REGEX pattern.");
	EXCEPTION(EmptyPatternException, "empty pattern.");
};

#endif // RPARSER_HPP
