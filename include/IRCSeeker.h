#ifndef IRCSEEKER_HPP
# define IRCSEEKER_HPP

# include <irc.h>

# include <regex.h>
# include <ircRegex.h>
# include <vector>

# define COLUMN	;

class IRCSeeker
{
	private:
		regex_t				_expr;
		str					_pattern;

		size_t				_matchIndex;
		size_t				_matchCount;
		std::vector<str>	_matches;

		void	_reset(void)
		{
			_pattern = "";
			_matchCount = 0;
			_matchIndex = -1U;
			_matches.clear();
			regfree(&_expr);
		}

		void	_findNext(const str &source, size_t nmatch)
		{
			if (_pattern == "")
				throw EmptyPatternException();
			if (_matchIndex == -1U)
				return ;

			regmatch_t	*hold = new regmatch_t[nmatch + 2];
			const str	crop = source.substr(_matchIndex, source.length());

			const int	err = regexec(&_expr, crop.c_str(), nmatch + 2, hold, 0);
			int			start = hold[1].rm_so;
			int			end = hold[1].rm_eo;

			if (err)
			{
				delete[] hold;
				_matchIndex = -1U;
				return ;
			}
			for (size_t i = 0; i < nmatch; ++i)
			{
				start = hold[i + 1].rm_so;
				end = hold[i + 1].rm_eo;

				if (start == -1 || end == -1)
				{
					_matchIndex = -1U;
					break ;
				}

				str match = crop.substr(start, end - start);
				//IRC_LOG("Matched group at %d-%d : " BOLD(COLOR(RED,"%s")), start, end, match.c_str());
				_matches.push_back(match);
				_matchIndex += end;
				_matchCount++;
			}

			delete[] hold;
		}

	public:
		IRCSeeker(void): _pattern(""), _matchIndex(0), _matchCount(0) {}
		~IRCSeeker(void) { regfree(&_expr); }
		IRCSeeker(const str &pattern): _pattern(pattern), _matchIndex(0), _matchCount(0)
		{
			const int err = regcomp(&_expr, pattern.c_str(), REG_EXTENDED);
			if (err)
				throw InvalidPatternException();
		}

		void	rebuild(const str &pattern)
		{
			if (_pattern != "")
				_reset();

			const int	err = regcomp(&_expr, pattern.c_str(), REG_EXTENDED);
			if (err)
				throw InvalidPatternException();

			_pattern = pattern;
			_matchIndex = 0;
		}

		bool	match(const str &source)
		{
			try { _findNext(source, 1); }
			IRC_CATCH

			IRC_WARN("After match, mindex = %lu", _matchIndex);
			if (_matchIndex == -1U)
				return (false);
			_matchCount = 0;
			_matchIndex = 0;
			_matches.clear();
			return (true);
		}

		void	capture(const str &source, size_t n)
		{
			try { _findNext(source, n); }
			IRC_CATCH

			_matchIndex = -1U;
		}

		void	findall(const str &source)
		{
			try { while (_matchIndex != -1U) _findNext(source, 1); }
			IRC_CATCH

			_matchIndex = -1U;
		}

		GETTER(size_t, _matchIndex);
		GETTER(size_t, _matchCount);
		GETTER(std::vector<str>, _matches);

		GETTER_C(size_t, _matchIndex);
		GETTER_C(size_t, _matchCount);
		GETTER_C(std::vector<str>, _matches);

	EXCEPTION(InvalidPatternException,	"invalid REGEX pattern.");
	EXCEPTION(EmptyPatternException,	"empty pattern.");
};

#endif // IRCSEEKER_HPP
