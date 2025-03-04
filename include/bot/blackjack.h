#pragma once

#include <cstdio>
#include <cstdlib>
# include <irc.h>
# include <algorithm>
# include <map>
# include <stdint.h>
# include <vector>

# define BLACKJACK 50


typedef enum:	uint32_t
{
	BJ_STAND	=	1,
	BJ_INGAME	=	1U << 2
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

	public:
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

		Hand():money(1000) {};
		Hand(Card &firstCard, Card &secondCard):money(1000)
		{
			_hand.push_back(firstCard);
			_hand.push_back(secondCard);
		}
		~Hand() {}

		GETTER(std::vector<Card>, _hand);
		GETTER(uint32_t, _flag);
		GETTER(int, money);
		GETTER(int, betting);

		GETTER_C(std::vector<Card>, _hand);
		GETTER_C(uint32_t, _flag);
		GETTER_C(int, money);
		GETTER_C(int, betting);

		SETTER(int, money);
		SETTER(int, betting);
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
		Deck				_deck;
		Hand				_dealer;
		std::map<int, Hand>	_players;
		size_t				_standingPlayers;
		uint32_t			_flag;

		void	_end()
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
				//you have won : newmoney
			}
			_restart();
		}
		void	_restart()
		{
			_dealer.redraw(_deck.drawCard(), _deck.drawCard());
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				IRC_FLAG_DEL(it->second.get_flag(), BJ_STAND);
				it->second.redraw(_deck.drawCard(), _deck.drawCard());
				//afficher argent et dire que on t attend
			}
			IRC_FLAG_DEL(_flag, BJ_INGAME);
			_standingPlayers = 0;
		}

	public:
		~BlackJack() {}

		BlackJack(): _deck(Deck())
		{
			_dealer = Hand(_deck.drawCard(), _deck.drawCard());
		}

		void addPlayer(int fd)
		{
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
				; //inside game
			_players[fd] = Hand(_deck.drawCard(), _deck.drawCard());
		}

		void	bet(int fd, int money)
		{
			if (IRC_FLAG_GET(_players[fd].get_flag(), BJ_STAND))
				return ;
			_players[fd].setbetting(money);
			_players[fd].getmoney() -= money;
			IRC_FLAG_SET(_players[fd].get_flag(), BJ_STAND);
			if (_standingPlayers++ == _players.size())
				start();
		}
		void	start()
		{
			IRC_FLAG_SET(_flag, BJ_INGAME);
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				IRC_FLAG_DEL(it->second.get_flag(), BJ_STAND);
				it->second.redraw(_deck.drawCard(), _deck.drawCard());
			}
			_standingPlayers = 0;
			//send to all player their card
		}

		void	hit(int fd)
		{
			if (IRC_FLAG_GET(_players[fd].get_flag(), BJ_STAND))
				return ;
			_players[fd].addCard(_deck.drawCard());
			if (_players[fd].handValue() > 21 || _players[fd].handValue() == BLACKJACK)
				stand(fd);
		}

		void	stand(int fd)
		{
			UNUSED(fd);
			IRC_FLAG_SET(_players[fd].get_flag(), BJ_STAND);
			_standingPlayers++;
			if (_standingPlayers == _players.size())
				_end();
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

		void	quit(int fd)
		{
			IRC_AUTO	it = _players.find(fd);
			int	money = it->second.getmoney();
			UNUSED(money);
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
			{
				if (!IRC_FLAG_GET(_players[fd].get_flag(), BJ_STAND))
					_standingPlayers++;
				if (_standingPlayers == _players.size())
					_end();
				_players.erase(it);
			}
			else
			{	
				if (!IRC_FLAG_GET(_players[fd].get_flag(), BJ_STAND))
					_standingPlayers++;
				if (_standingPlayers == _players.size())
					start();
				_players.erase(it);
			}
			//quittting with (money)
		}
		void	doubleDown(int fd)
		{
			hit(fd);
			_players[fd].getbetting() *= 2;
			stand(fd);
		}

};
