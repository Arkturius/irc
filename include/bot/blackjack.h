#pragma once

#include <cstdio>
#include <cstdlib>
# include <irc.h>
# include <algorithm>
# include <map>
# include <stdint.h>
# include <vector>

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
			return value;
		}
		void	addCard(Card &newCard)
		{
			_hand.push_back(newCard);
		}

		Hand() {};
		Hand(Card &firstCard, Card &secondCard)
		{
			_hand.push_back(firstCard);
			_hand.push_back(secondCard);
		}
		~Hand() {}

		GETTER(std::vector<Card>, _hand);
		GETTER_C(std::vector<Card>, _hand);

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
	
	public:
		~BlackJack() {}

		BlackJack(): _deck(Deck())
		{
			_dealer = Hand(_deck.drawCard(), _deck.drawCard());
		}

		void AddPlayer(int fd)
		{
			_players[fd] = Hand(_deck.drawCard(), _deck.drawCard());
		}

		void	Start()
		{
		}
		void	hit(int fd)
		{
			_players[fd].addCard(_deck.drawCard());
		}
		void	stand(int fd)
		{
			UNUSED(fd);
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
};
