#pragma once

# include <unistd.h>
# include <irc.h>
# include <Server.h>
# include <algorithm>
# include <map>
# include <stdint.h>
# include <vector>
# include <Client.h>
# include <sstream>

# define BLACKJACK 50


typedef enum:	uint32_t
{
	BJ_STAND	=	1,
	BJ_AS_BET	=	1U << 1,

	BJ_WAITING	=	1U << 2,
	BJ_OPERATOR	=	1U << 3,
	BJ_INVITEED	=	1U << 4,

	BJ_BETTING	=	1U << 6,
	BJ_PLAYING	=	1U << 7,
	BJ_INGAME	=	(1U << 8) - 1,
}	blackJackFlag;

typedef struct
{
	uint8_t color:2;
	uint8_t value:6;
}	Card;

#define CARD_SPADE		0
#define CARD_HEARTH		1
#define CARD_CLOVER		2
#define CARD_DIAMOND	3

str	cardIcon(int reverse, const Card &card)
{
	const str	icons[4] = { "♤ ", "♡ ", "♧ ", "♢ " }	;
	str			value = "";

	switch (card.value)
	{
		case 10:
			value = "X"; break ;
		case 11:
			value = "J"; break ;
		case 12:
			value = "Q"; break ;
		case 13:
			value = "K"; break ;
		default:
			value += card.value + 48;
	}
	if (reverse)
		return (icons[card.color] + value);
	return (value + icons[card.color]);
}

class Deck
{
	private:
		std::vector<Card>	_allCard;

		template<typename I, typename URNG>
		void	_shuffleDeck(I first, I last, URNG& g)
		{
			if (first == last)
				return;

			typedef typename std::iterator_traits<I>::difference_type diff_t;
			diff_t n = last - first;
			for (diff_t i = n - 1; i > 0; --i)
			{
				diff_t j = static_cast<diff_t>(g() % (i + 1));
				if (j != i)
					std::swap(*(first + i), *(first + j));
			}
		}

	public:
		Deck()
		{
			for (uint8_t i = 1; i < 14; i++)
			{
				for (uint8_t j = 0; j < 4; j++)
					_allCard.push_back((Card){.color = j, .value = i});
			}
			_shuffleDeck(_allCard.begin(), _allCard.end(), std::rand);
		};

		~Deck() {};

		Card	&drawCard()
		{
			static uint8_t i;
			if (++i == 52)
			{
				i = 0;
				_shuffleDeck(_allCard.begin(), _allCard.end(), std::rand);
			}
			return _allCard[i];
		}
};

class	Hand
{
	private:
		std::vector<Card>	_hand;

		uint32_t			_flag;
		int					money;
		int					betting;

		int					_fd;
		str					_name;

	public:
		void	sendToPlayer(const str &msg)
		{
			std::stringstream	stream;
			stream << money;
			str string = str(":localhost PRIVMSG #") + _name + str(" :") + msg + + " ($" + stream.str() + str(")\r\n");
			write(_fd, string.c_str(), string.size());
		}

		int		handValue()
		{
			int	value = 0;
			int	aces = 0;
			for (size_t i = 0; i < _hand.size(); i++)
			{
				value += std::min((int)_hand[i].value, 10);
				if (_hand[i].value == 1)
					aces++;
			}
			while (aces--)
			{
				if (value <= 11)
					value += 10;
			}
			if (value == 21 && _hand.size() == 2)
				return BLACKJACK;
			return value;
		}

		void	addCard(Card &newCard)
		{
			_hand.push_back(newCard);
		}

		void	redraw(Card &firstCard, Card &secondCard)
		{
			_hand.clear();
			_hand.push_back(firstCard);
			_hand.push_back(secondCard);
		}

		Hand() {};
		Hand(int fd, const str &name, Card &firstCard, Card &secondCard):money(1000), _fd(fd), _name(name)
		{
			_hand.push_back(firstCard);
			_hand.push_back(secondCard);
		}
		~Hand() {}

		GETTER(std::vector<Card>, _hand);
		GETTER(uint32_t, _flag);
		GETTER(int, money);
		GETTER(int, betting);
		GETTER(str, _name);

		GETTER_C(std::vector<Card>, _hand);
		GETTER_C(uint32_t, _flag);
		GETTER_C(int, money);
		GETTER_C(int, betting);
		GETTER_C(str, _name);

		SETTER(int, money);
		SETTER(int, betting);
		SETTER(str, _name);
};

# define CARD_TOP		"╭─────╮"
# define CARD_ICON		"│\x02" + cardIcon(0, card) + "\x02  │"
# define CARD_MID		"│ \x02\x1d\x03" "15irc" "\x03\x1d\x02 │"
# define CARD_RICON		"│  \x02" + cardIcon(1, card) + "\x02│"
# define CARD_BTM		"╰─────╯"

# define CARD_TOP_T		"╭──┈"
# define CARD_ICON_T	"│\x02" + cardIcon(0, card) + "\x02"
# define CARD_MID_T		"│ \x02\x1d\x03" "15ir" "\x03\x1d\x02"
# define CARD_RICON_T	"│   "
# define CARD_BTM_T		"╰──┈"

# define CARD_SPACE		"       "

class BlackJack
{
	private:
		Deck					_deck;
		Hand					_dealer;
		std::map<str, Hand>		_players;
		size_t					_standingPlayers;
		uint32_t				_flag;

		void	_startRound()
		{
			IRC_FLAG_SET(_flag, BJ_PLAYING);
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				if (!IRC_FLAG_GET(it->second.get_flag(), BJ_WAITING))
					it->second.sendToPlayer("every player as bet; you can now [stand|hit|double]");
			}
		}

		void	_endRound()
		{
			while (_dealer.handValue() < 17)
				_dealer.addCard(_deck.drawCard());

			int	value = _dealer.handValue();
			if (value > 21 && value != BLACKJACK)
				value = -1;

			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				int newmoney = 0;
				if (it->second.handValue() == BLACKJACK && value != BLACKJACK)
					newmoney = 2.5 * it->second.getbetting();
				else if (it->second.handValue() == value)
					newmoney = it->second.getbetting();
				else if (it->second.handValue() >= value)
					newmoney = 2 * it->second.getbetting();
				it->second.getmoney() += newmoney;
				std::stringstream	stream;
				stream << newmoney;
				it->second.sendToPlayer(str("you have won ") + stream.str());
				it->second.sendToPlayer(displayGame());
			}
			_restart();
		}

		void	_restart()
		{
			_dealer.redraw(_deck.drawCard(), _deck.drawCard());
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				it->second.get_flag() = 0;
				it->second.redraw(_deck.drawCard(), _deck.drawCard());
				it->second.sendToPlayer("waiting for the host to restart the game");
			}
			_flag = 0;
			_standingPlayers = 0;
		}

		size_t	_getInGamePlayer() const
		{
			int	i = 0;
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				if (!IRC_FLAG_GET(it->second.get_flag(), BJ_WAITING))
					i++;
			}
			return i;
		}


	public:
		void	setDealerName(str &name)
		{
			_dealer.set_name(name);
		}

		~BlackJack() {
			const str	msg = "Your leaving table of " + _dealer.get_name() + " with";
			for (IRC_AUTO it = _players.begin(); it != _players.begin(); ++it)
				it->second.sendToPlayer(msg);

		}

		BlackJack(): _deck(Deck())
		{
			_dealer = Hand(-1, "dealer", _deck.drawCard(), _deck.drawCard());
		}

		void addPlayer(const str &name, int fd)
		{
			_players[name] = Hand(fd, name, _deck.drawCard(), _deck.drawCard());
			_players[name].sendToPlayer("starting the game with $1000");
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
			{
				_players[name].get_flag() = BJ_WAITING;
				_players[name].sendToPlayer("waiting the game end");
			}
		}

		void	start()
		{
			IRC_FLAG_SET(_flag, BJ_BETTING);
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				it->second.get_flag() = 0;
				it->second.redraw(_deck.drawCard(), _deck.drawCard());
				it->second.sendToPlayer(displayGame());
				it->second.sendToPlayer("you can now bet");
			}
			_standingPlayers = 0;
		}
		void	bet(const str &name, int money)
		{
			money = std::max(1, std::min(money, 1000));
			if (IRC_FLAG_GET(_players[name].get_flag(), BJ_AS_BET))
				return ;
			_players[name].setbetting(money);
			_players[name].getmoney() -= money;
			IRC_FLAG_SET(_players[name].get_flag(), BJ_AS_BET);
			std::stringstream	stream;
			stream << money;
			_players[name].sendToPlayer(str("you bet $") + stream.str());
			if (_standingPlayers++ == _players.size())
				_startRound();
		}

		void	hit(const str &name)
		{
			if (IRC_FLAG_GET(_players[name].get_flag(), BJ_STAND))
				return ;
			_players[name].addCard(_deck.drawCard());
			if (_players[name].handValue() > 21 || _players[name].handValue() == BLACKJACK)
				stand(name);
		}

		void	stand(const str &name)
		{
			IRC_FLAG_SET(_players[name].get_flag(), BJ_STAND);
			_standingPlayers++;
			if (_standingPlayers == _players.size())
				_endRound();
		}
		void	doubleDown(const str &name)
		{
			if (IRC_FLAG_GET(_players[name].get_flag(), BJ_STAND))
				return ;
			hit(name);
			_players[name].getbetting() *= 2;
			stand(name);
		}

		str	displayGame()
		{
			std::vector<Card>	all;

			all.insert(all.end(), _dealer.get_hand().begin(), _dealer.get_hand().end());
			all.push_back((Card){.color = 0, .value = 0});

			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				all.insert(all.end(), (*it).second.get_hand().begin(), (*it).second.get_hand().end());
				all.push_back((Card){.color = 0, .value = 0});
			}

			str cardbanner = "\n";
			for (int i = 0; i < 5; ++i)
			{
				for (size_t j = 0; j < all.size() - 1; ++j)
				{
					Card	card = all[j];
					Card	nextCard = all[j + 1];

					if (card.value == 0)
					{
						cardbanner += CARD_SPACE;
						continue ;
					}

					str	parts[10] = {CARD_TOP_T, CARD_ICON_T, CARD_MID_T, CARD_RICON_T, CARD_BTM_T, \
						CARD_TOP, CARD_ICON, CARD_MID, CARD_RICON, CARD_BTM};

					cardbanner += parts[i + 5 * (nextCard.value == 0)];
				}
				cardbanner += "\n";
			}
			return cardbanner;
		}

		void	quit(const str &name)
		{
			IRC_AUTO	it = _players.find(name);
			if (IRC_FLAG_GET(it->second.get_flag(), BJ_WAITING))
				goto quiting;
			IRC_FLAG_SET(it->second.get_flag(), BJ_WAITING);
			if (IRC_FLAG_GET(_flag, BJ_PLAYING))
			{
				if (!IRC_FLAG_GET(_players[name].get_flag(), BJ_STAND))
					_standingPlayers++;
				if (_standingPlayers == _getInGamePlayer())
					_endRound();
			}
			else if (IRC_FLAG_GET(_flag, BJ_BETTING))
			{	
				if (!IRC_FLAG_GET(_players[name].get_flag(), BJ_AS_BET))
					_standingPlayers++;
				if (_standingPlayers == _getInGamePlayer())
					_startRound();
			}
quiting:
			it->second.sendToPlayer("your quitting the game");
			_players.erase(it);
		}

		void	stop()
		{
			const str	msg = "Your leaving table of " + _dealer.get_name() + " with";
			for (IRC_AUTO it = _players.begin(); it != _players.begin(); ++it)
				it->second.sendToPlayer(msg);
			_players.clear();
		}
};

IRC_COMMAND_DEF(BJ)
{
	const std::vector<str>	&param = _parsingParam(command);
	BlackJack				Target;

	IRC_LOG("PARAMS SIZE = %lu", param.size());
	if (param.size() == 0)
		goto needMoreParam;

	else if (param[0] == "START")
	{
	}
	else if (param[0] == "STOP")
	{
	}
	else if (param[0] == "INVITE")
	{
	}
	if (param[0] == "JOIN")
	{
	}
	else if (param[0] == "HIT")
	{

	}
	else if (param[0] == "STAND")
	{
	}
	else if (param[0] == "DOUBLE")
	{
	}
	else if (param[0] == "QUIT")
	{
	}

	needMoreParam:
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "BJ"));
}
