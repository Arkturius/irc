#pragma once

# include <algorithm>
# include <map>
# include <stdint.h>
# include <vector>
# include <irc.h>

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

		GETTER(uint32_t, _flag);
		GETTER_C(uint32_t, _flag);

		SETTER(int, money);
		GETTER(int, money);
		GETTER_C(int, money);

		SETTER(int, betting);
		GETTER(int, betting);
		GETTER_C(int, betting);
};

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
			IRC_FLAG_SET(_players[fd].get_flag(), BJ_STAND);
			_standingPlayers++;
			if (_standingPlayers == _players.size())
				_end();
		}
		void	quit(int fd)
		{
			IRC_AUTO	it = _players.find(fd);
			int	money = it->second.getmoney();
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
