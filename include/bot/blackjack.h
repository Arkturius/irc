#pragma once

# include <algorithm>
#include <map>
# include <stdint.h>
# include <vector>
# include <irc.h>

typedef struct
{
	uint8_t color:2;
	uint8_t value:6;
}	Card;

class Deck
{
	private:
		std::vector<Card>	_allCard;
		void	_shuffleDeck()
		{
			std::random_shuffle(_allCard.begin(), _allCard.end());
		}

	public:
		Deck()
		{
			for (uint8_t i = 1; i < 14; i++)
			{
				for (uint8_t j = 0; j < 4; j++)
					_allCard.push_back((Card){.color = j, .value = i});
			}
		};
		~Deck() {};
		Card	&drawCard()
		{
			static uint8_t i;
			if (++i == 52)
			{
				i = 0;
				_shuffleDeck();
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

};

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

		}
};
