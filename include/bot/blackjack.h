#pragma once

# include <cstdlib>
#include <ctime>
# include <unistd.h>
# include <irc.h>
# include <Server.h>
# include <algorithm>
# include <map>
# include <stdint.h>
#include <utility>
# include <vector>
# include <Client.h>
# include <sstream>

# define BLACKJACK 50


typedef enum:	uint32_t
{
	BJ_STAND	=	1,
	BJ_AS_BET	=	1U << 1,

	BJ_WAITING	=	1U << 2,

	BJ_BETTING	=	1U << 6,
	BJ_PLAYING	=	1U << 7,
	BJ_INGAME	=	(1U << 6) + (1U << 7),
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

	if (card.value == 63)
		return "???";
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
		}

		~Deck() {}

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
		void	sendMsg(const str &msg) const
		{
			write(_client.get_fd(), msg.c_str(), msg.size());
		}
		void	sendToPlayer(const str &msg, const str &dealer, const str &channel) const
		{
			std::stringstream	stream;
			stream << money;
			str string = str(":") + dealer + str(" PRIVMSG #") + channel;
			string += str(" :") + msg + str(" ($");
			string += stream.str() + str(")\r\n");
			sendMsg(string);
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

		Hand(Client &client, Card &firstCard, Card &secondCard): money(1000), _client(client)
		{
			_hand.push_back(firstCard);
			_hand.push_back(secondCard);
		}

		~Hand() {}

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
		size_t	size() const
		{
			return _hand.size();
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
# define CARD_ICON		"│" + cardIcon(0, card) + "  │"
# define CARD_MID		"│ irc │"
# define CARD_RICON		"│  " + cardIcon(1, card) + "│"
# define CARD_BTM		"╰─────╯"

# define CARD_TOP_T		"╭──┈"
# define CARD_ICON_T	"│" + cardIcon(0, card) 
# define CARD_MID_T		"│ ir"
# define CARD_RICON_T	"│   "
# define CARD_BTM_T		"╰──┈"

# define CARD_SPACE		"		"

class BlackJack
{
	private:
		Deck					_deck;
		Hand					_dealer;
		std::map<int, Hand *>	_players;
		size_t					_standingPlayers;
		uint32_t				_flag;

		void	_sendToPlayer(const Hand *hand, const str &msg)
		{
			const str	&dealer = _dealer.get_client().get_nickname();
			const str	&channel = _dealer.get_client().get_username() + str("_table");

			hand->sendToPlayer(msg, dealer, channel);
		}
		void	_sendDisplay(const Hand *hand)
		{
			const str	&msg = displayGame();

			hand->sendMsg(msg);
		}


		void	_startRound()
		{
			IRC_FLAG_DEL(_flag, BJ_BETTING);
			IRC_FLAG_SET(_flag, BJ_PLAYING);
			IRC_FLAG_DEL(_flag, BJ_BETTING);
			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				if (!IRC_FLAG_GET(it->second->get_flag(), BJ_WAITING))
					_sendToPlayer(it->second, "every player as bet; you can now [stand|hit|double]");
			}
			_standingPlayers = 0;
		}

		void	_endRound()
		{
			IRC_FLAG_DEL(_flag, BJ_PLAYING);
			while (_dealer.handValue() < 17)
				_dealer.addCard(_deck.drawCard());

			int	value = _dealer.handValue();
			if (value > 21 && value != BLACKJACK)
				value = -1;
			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				if (IRC_FLAG_GET(it->second->get_flag(), BJ_WAITING))
					continue ;
				int newmoney = 0;
				if (it->second->handValue() == BLACKJACK && value != BLACKJACK)
					newmoney = 2.5 * it->second->getbetting();
				else if (it->second->handValue() == value)
					newmoney = it->second->getbetting();
				else if (it->second->handValue() >= value)
					newmoney = 2 * it->second->getbetting();
				if (it->second->handValue() > 21 && it->second->handValue() != BLACKJACK)
					newmoney = 0;
				it->second->getmoney() += newmoney;
				std::stringstream	stream;
				stream << newmoney;
				_sendDisplay(it->second);
				_sendToPlayer(it->second,str("you have won ") + stream.str());
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
				_sendToPlayer(it->second,"waiting for the host to restart the game");
			}
			_flag = 0;
			_standingPlayers = 0;
		}

	public:
		~BlackJack() {
			IRC_LOG("bj destructor");
		}

		BlackJack(Client &dealer): _deck(Deck()), _dealer(dealer, _deck.drawCard(), _deck.drawCard()), _flag(0) {}

		void addPlayer(Client &client)
		{
			int	fd = client.get_fd();
			_players[fd] = new Hand(client, _deck.drawCard(), _deck.drawCard());
			_sendToPlayer(_players[fd], "starting the game with $1000");
			_players[fd]->get_flag() = 0;
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
			{
				_players[fd]->get_flag() = BJ_WAITING;
				_sendToPlayer(_players[fd], "waiting the game end");
			}
		}

		void	start(Client &client)
		{
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
				return ;
			str	summoner_name =  _dealer.get_client().get_username();
			if (client.get_nickname() != summoner_name)
				throw "Your not the Operator";
			IRC_LOG("starting the game");

			_flag =  BJ_BETTING;
			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
				it->second->redraw(_deck.drawCard(), _deck.drawCard());
			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				it->second->get_flag() = 0;
				_sendDisplay(it->second);
				_sendToPlayer(it->second,"you can now bet");
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
			_sendToPlayer(_players[fd], str("you bet $") + stream.str());
			if (++_standingPlayers == size())
				_startRound();
			IRC_LOG("BETTING STATS %zu/%zu", _standingPlayers,size());
		}

		void	hit(Client &client)
		{
			if (!IRC_FLAG_GET(_flag, BJ_PLAYING))
				return ;
			int	fd = client.get_fd();
			if (IRC_FLAG_GET(_players[fd]->get_flag(), BJ_STAND))
				return ;
			_players[fd]->addCard(_deck.drawCard());
			_sendDisplay(_players[fd]);
			if (_players[fd]->handValue() > 21 || _players[fd]->handValue() == BLACKJACK)
				stand(client);
		}

		void	stand(Client &client)
		{
			if (!IRC_FLAG_GET(_flag, BJ_PLAYING))
				return ;
			int	fd = client.get_fd();
			if (IRC_FLAG_GET(_players[fd]->get_flag(), BJ_STAND))
				return ;
			IRC_FLAG_SET(_players[fd]->get_flag(), BJ_STAND);
			if (++_standingPlayers == size())
				_endRound();
			IRC_LOG("STANDING STATS %zu/%zu", _standingPlayers,size());
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

		str	nickNamesDisplay(str &name, char nbCard)
		{
			str	result = " ";
			result += name;
			char	spaces = 10 - name.size() + (nbCard - 2) * 4;
			while (spaces--)
				result += " ";
			result += CARD_SPACE;
			return result;
		}

		str	displayGame()
		{	
			const str	&dealer = _dealer.get_client().get_nickname();
			const str	&channel = _dealer.get_client().get_username() + str("_table");
			const str	&start = str(":") + dealer + str(" PRIVMSG #") + channel +  str(" :");

			std::vector<std::pair<str, char> >	nickNames;
			std::vector<Card>	all;

			nickNames.push_back(std::make_pair("dealer", _dealer.size()));
			all.insert(all.end(), _dealer.get_hand().begin(), _dealer.get_hand().end());
			if (IRC_FLAG_GET(_flag, BJ_INGAME))
				all[1].value = 63;

			all.push_back((Card){.color = 0, .value = 0});

			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				Hand	*player = it->second;
				all.insert(all.end(), player->get_hand().begin(), player->get_hand().end());
				all.push_back((Card){.color = 0, .value = 0});
				nickNames.push_back(std::make_pair(player->get_client().get_nickname(), player->size()));
			}

			str cardbanner = start;
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
				cardbanner += "\n" + start;
			}

			for (size_t j = 0; j < nickNames.size(); j++)
			{
				char	size = nickNames[j].second;
				if (j + 1 == nickNames.size())
					cardbanner += " " + nickNames[j].first;
				else
					cardbanner += nickNamesDisplay(nickNames[j].first, size);
			}
			cardbanner += "\n";

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
			if (IRC_FLAG_GET(it->second->get_flag(), BJ_WAITING) || 
				!IRC_FLAG_GET(_flag, BJ_INGAME))
				goto quiting;
			if (IRC_FLAG_GET(_flag, BJ_PLAYING))
			{
				if (!IRC_FLAG_GET(_players[fd]->get_flag(), BJ_STAND))
					_standingPlayers++;
				if (_standingPlayers == size())
					_endRound();
			}
			else if (IRC_FLAG_GET(_flag, BJ_BETTING))
			{	
				if (!IRC_FLAG_GET(_players[fd]->get_flag(), BJ_AS_BET))
					_standingPlayers++;
				if (_standingPlayers == size())
					_startRound();
			}
quiting:
			_sendToPlayer(it->second,str("your quitting the game of ") + summoner_name);
		}

		void	stop(Client &client)
		{
			str	summoner_name =  _dealer.get_client().get_username();
			
			std::map<int, Hand *>::iterator next;
			for (IRC_AUTO it = _players.begin(); it != _players.end(); it = next)
			{
				next = it;
				++next;
				if (it->second->get_client().get_fd() != client.get_fd())
					quit(it->second->get_client());
				else
					_sendToPlayer(it->second,"your stoping your game");
			}
		}

		size_t	size() const
		{
			size_t	i = 0;
			for (IRC_AUTO it = _players.begin(); it != _players.end(); ++it)
			{
				if (it->second->get_flag() != BJ_WAITING)
					i++;
			}
			return i;
		}

		GETTER(Hand, _dealer);
		GETTER(std::map<int COMMA Hand *>, _players);

		GETTER_C(Hand, _dealer);
		GETTER_C(std::map<int COMMA Hand *>, _players);
};
