#ifndef IRCSEEKER_HPP
# define IRCSEEKER_HPP

#include <cstring>
# include <irc.h>

# include <regex.h>
# include <ircRegex.h>
# include <vector>

# define COLUMN	;
# define IRC_SEEKER_GROUP_MAX	126

# define IRC_BZERO(X)	std::memset(&(X), 0, sizeof(X))

class IRCSeeker
{
	private:
		regex_t				_expr;
		str					_pattern;
		str					_string;
		bool				_didMatch;

		size_t				_matchIndex;
		size_t				_matchCount;
		std::vector<str>	_matches;
		regmatch_t			_hold[IRC_SEEKER_GROUP_MAX + 2];

		void	_crop(void)			{ _string = _string.substr(_matchIndex, _string.length()); _matchIndex = 0; }
		void	_resetMatches(void)	{ _matchIndex = 0; _matchCount = 0; _matches.clear(); }

		void	_rebuild(const str &pattern)
		{
			if (pattern == "")		{ throw EmptyPatternException(); }
			regfree(&_expr);
 			if (_expr.allocated)	{ regfree(&_expr); }

			if (regcomp(&_expr, pattern.c_str(), REG_EXTENDED))
				throw InvalidPatternException();

			_pattern = pattern;
			_resetMatches();
		}

		void	_find(void)
		{
			_didMatch = !regexec(&_expr, _string.c_str() + _matchIndex, IRC_SEEKER_GROUP_MAX + 2, _hold, 0);

			if (_matchIndex == _string.length())	{_didMatch = false; IRC_WARN("_string fully consumed, aborting."); return; }
			if (!_didMatch)							{ throw RegExecFailedException(); }
			if (_hold[1].rm_so == -1)				{ _didMatch = false; IRC_WARN("0 length match, aborting."); return; }

			const str	match = _string.substr(_hold[1].rm_so + _matchIndex, _hold[1].rm_eo - _hold[1].rm_so);
			IRC_OK("match found at %d - %d = [" BOLD(COLOR(GREEN,"%s")) "]", _hold[1].rm_so, _hold[1].rm_eo, match.c_str());

			_matches.push_back(match);
			_matchIndex += _hold[1].rm_eo;
			_matchCount++;
		}

	public:
		IRCSeeker(void): _pattern(""), _string(""), _matchIndex(0), _matchCount(0) { IRC_BZERO(_expr); }
		~IRCSeeker(void) { regfree(&_expr); }

		IRCSeeker(const str &pattern): _pattern(pattern), _string(""), _matchIndex(0), _matchCount(0)
		{
			const int err = regcomp(&_expr, pattern.c_str(), REG_EXTENDED);
			if (err)
				throw InvalidPatternException();
		}

		void	rebuild(const str &pattern)		{ try { _rebuild(pattern); } IRC_CATCH }

		void	feedString(const str &string)	{ _string = string; }

		void	findall(void)					{ _didMatch = true; try { while(_didMatch) { _find(); } } IRC_CATCH }

		bool	match(void)						{ try { _find(); } IRC_CATCH return (_didMatch); }

		bool	consume(void)					{ try { _find(); _crop(); } IRC_CATCH return (_didMatch); }
		
		void	consumeMany(void)				{ _didMatch = true; try { while(_didMatch) { _find(); _crop(); } } IRC_CATCH }

		GETTER(str, _pattern);
		GETTER(str, _string);
		GETTER(bool, _didMatch);
		GETTER(size_t, _matchIndex);
		GETTER(size_t, _matchCount);
		GETTER(std::vector<str>, _matches);

		GETTER_C(str, _pattern);
		GETTER_C(str, _string);
		GETTER_C(bool, _didMatch);
		GETTER_C(size_t, _matchIndex);
		GETTER_C(size_t, _matchCount);
		GETTER_C(std::vector<str>, _matches);

	EXCEPTION(InvalidPatternException,		"invalid REGEX pattern.");
	EXCEPTION(EmptyPatternException,		"empty pattern.");
	EXCEPTION(RegExecFailedException,		"regexec() failed.");
};

#endif // IRCSEEKER_HPP
