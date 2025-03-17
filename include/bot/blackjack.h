#pragma once

# include <cstdlib>
#include <ctime>
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
			std::srand(std::time(0));
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

		Client				&_client;

	public:
		void	sendToPlayer(const str &msg)
		{
			std::stringstream	stream;
			stream << money;
			str string = str(":localhost PRIVMSG #") + _client.getTargetName();
			string += str(" :") + msg + str(" ($");
			string += stream.str() + str(")\r\n");
			write(_client.get_fd(), string.c_str(), string.size());
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

		Hand(Client &client, Card &firstCard, Card &secondCard):money(1000), _client(client)
		{
			_hand.push_back(firstCard);
			_hand.push_back(secondCard);
		}
		~Hand()
		{
			IRC_LOG("hand destructor");
		}
		Hand	&operator=(Hand const &other)
		{
			if (this != &other)
			{
				money = other.getmoney();
				_flag = other.get_flag();
				betting = other.getbetting();
				_client = other.get_client();
				_hand = other.get_hand();
			}
			return *this;
		}

		GETTER(std::vector<Card>, _hand);
		GETTER(uint32_t, _flag);
		GETTER(int, money);
		GETTER(int, betting);
		GETTER(Client, _client);

		GETTER_C(std::vector<Card>, _hand);
		GETTER_C(uint32_t, _flag);
		GETTER_C(int, money);
		GETTER_C(int, betting);
		GETTER_C(Client, _client);

		SETTER(int, money);
		SETTER(int, betting);
		SETTER(Client, _client);
};

# define CARD_TOP		"╭─────╮"
# define CARD_ICON		"│\x02" + cardIcon(0, card) + "\x02  │"
# define CARD_MID		"│ \x02\x1d\x03" "irc" "\x03\x1d\x02 │"
# define CARD_RICON		"│  \x02" + cardIcon(1, card) + "\x02│"
# define CARD_BTM		"╰─────╯"

# define CARD_TOP_T		"╭──┈"
# define CARD_ICON_T	"│\x02" + cardIcon(0, card) + "\x02"
# define CARD_MID_T		"│ \x02\x1d\x03" "ir" "\x03\x1d\x02"
# define CARD_RICON_T	"│   "
# define CARD_BTM_T		"╰──┈"

# define CARD_SPACE		"       "

class BlackJack
{
	private:
		Deck					_deck;
		Hand					_dealer;
		std::map<int, Hand *>	_players;
		size_t					_standingPlayers;
		uint32_t				_flag;

		void	_startRound()
		{
			IRC_FLAG_SET(_flag, BJ_PLAYING);
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				if (!IRC_FLAG_GET(it->second->get_flag(), BJ_WAITING))
					it->second->sendToPlayer("every player as bet; you can now [stand|hit|double]");
			}
			_standingPlayers = 0;
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
				if (it->second->handValue() == BLACKJACK && value != BLACKJACK)
					newmoney = 2.5 * it->second->getbetting();
				else if (it->second->handValue() == value)
					newmoney = it->second->getbetting();
				else if (it->second->handValue() >= value)
					newmoney = 2 * it->second->getbetting();
				it->second->getmoney() += newmoney;
				std::stringstream	stream;
				stream << newmoney;
				it->second->sendToPlayer(displayGame());
				it->second->sendToPlayer(str("you have won ") + stream.str());
			}
			_restart();
		}

		void	_restart()
		{
			_dealer.redraw(_deck.drawCard(), _deck.drawCard());
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				it->second->get_flag() = 0;
				it->second->redraw(_deck.drawCard(), _deck.drawCard());
				it->second->sendToPlayer("waiting for the host to restart the game");
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
				if (!IRC_FLAG_GET(it->second->get_flag(), BJ_WAITING))
					i++;
			}
			return i;
		}


	public:
		~BlackJack() {
			IRC_LOG("bj destructor");
		}

		BlackJack(Client &dealer): _deck(Deck()), _dealer(dealer, _deck.drawCard(), _deck.drawCard()), _flag(0)
		{
			IRC_LOG("summoning blackjack game");
		}

		void addPlayer(Client &client)
		{
			int	fd = client.get_fd();
			_players[fd] = new Hand(client, _deck.drawCard(), _deck.drawCard());
			_players[fd]->sendToPlayer("starting the game with $1000");
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
			{
				_players[fd]->get_flag() = BJ_WAITING;
				_players[fd]->sendToPlayer("waiting the game end");
			}
		}

		void	start(Client &client)
		{
			IRC_LOG("starting the game");
			str	summoner_name =  _dealer.get_client().get_username();
			if (client.get_nickname() != summoner_name)
				throw "Your not the Operator";

			IRC_FLAG_SET(_flag, BJ_BETTING);
			IRC_AUTO it = _players.begin();
			for (; it != _players.end(); ++it)
			{
				//pk c pas un broadcast? car faut montrer la money
				it->second->get_flag() = 0;
				it->second->redraw(_deck.drawCard(), _deck.drawCard());
				it->second->sendToPlayer("you can now bet"); //WHY?
				it->second->sendToPlayer(displayGame());
			}
			_standingPlayers = 0;
		}
		void	bet(Client &client, int money)
		{
			if (!IRC_FLAG_GET(_flag, BJ_BETTING))
				return ;
			int	fd = client.get_fd();
			money = std::max(1, std::min(money, 1000));
			if (IRC_FLAG_GET(_players[fd]->get_flag(), BJ_AS_BET))
				return ;
			_players[fd]->setbetting(money);
			_players[fd]->getmoney() -= money;
			IRC_FLAG_SET(_players[fd]->get_flag(), BJ_AS_BET);
			std::stringstream	stream;
			stream << money;
			_players[fd]->sendToPlayer(str("you bet $") + stream.str());
			if (++_standingPlayers == _players.size())
				_startRound();
		}

		void	hit(Client &client)
		{
			if (!IRC_FLAG_GET(_flag, BJ_PLAYING))
				return ;
			int	fd = client.get_fd();
			if (IRC_FLAG_GET(_players[fd]->get_flag(), BJ_STAND))
				return ;
			_players[fd]->addCard(_deck.drawCard());
			_players[fd]->sendToPlayer(displayGame());
			if (_players[fd]->handValue() > 21 || _players[fd]->handValue() == BLACKJACK)
				stand(client);
		}

		void	stand(Client &client)
		{
			if (!IRC_FLAG_GET(_flag, BJ_PLAYING))
				return ;
			int	fd = client.get_fd();
			IRC_FLAG_SET(_players[fd]->get_flag(), BJ_STAND);
			if (++_standingPlayers == _players.size())
				_endRound();
			IRC_LOG("%zu %zu", _standingPlayers, _players.size());
		}
		void	doubleDown(Client &client)
		{
			if (!IRC_FLAG_GET(_flag, BJ_PLAYING))
				return ;
			int	fd = client.get_fd();
			if (IRC_FLAG_GET(_players[fd]->get_flag(), BJ_STAND))
				return ;
			hit(client);
			_players[fd]->getbetting() *= 2;
			stand(client);
		}

		str	displayGame()
		{
			std::vector<Card>	all;

			all.insert(all.end(), _dealer.get_hand().begin(), _dealer.get_hand().end());
			all.push_back((Card){.color = 0, .value = 0});

			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				all.insert(all.end(), (*it).second->get_hand().begin(), (*it).second->get_hand().end());
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

		void	quit(Client &client)
		{
			str	summoner_name =  _dealer.get_client().get_username();
			if (client.get_nickname() == summoner_name)
			{
				stop(client);
				return ;
			}


			int fd = client.get_fd();
			IRC_AUTO	it = _players.find(fd);
			if (IRC_FLAG_GET(it->second->get_flag(), BJ_WAITING))
				goto quiting;
			IRC_FLAG_SET(it->second->get_flag(), BJ_WAITING);
			if (IRC_FLAG_GET(_flag, BJ_PLAYING))
			{
				if (!IRC_FLAG_GET(_players[fd]->get_flag(), BJ_STAND))
					_standingPlayers++;
				if (_standingPlayers == _getInGamePlayer())
					_endRound();
			}
			else if (IRC_FLAG_GET(_flag, BJ_BETTING))
			{	
				if (!IRC_FLAG_GET(_players[fd]->get_flag(), BJ_AS_BET))
					_standingPlayers++;
				if (_standingPlayers == _getInGamePlayer())
					_startRound();
			}
quiting:
			it->second->sendToPlayer(str("your quitting the game of ") + summoner_name);
			it->second->get_client().set_bjTable(0);
			delete it->second;
			_players.erase(it);
		}

		void	stop(Client &client)
		{
			str	summoner_name =  _dealer.get_client().get_username();
			
			std::map<int, Hand *>::iterator next;
			for (IRC_AUTO it = _players.begin(); it != _players.end(); it = next)
			{
				next = it;
				++next;
				IRC_LOG("boucle x");
				if (it->second->get_client().get_fd() != client.get_fd())
					quit(it->second->get_client());
				else
				{
					IRC_LOG("Bj stoped");
					it->second->sendToPlayer("your stoping your game");
					client.set_bjTable(0);
					delete it->second;
					_players.erase(it);
				}
			}
			_players.clear();
		}

		GETTER(Hand, _dealer);
		GETTER_C(Hand, _dealer);
};

