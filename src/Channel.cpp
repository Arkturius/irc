#include <algorithm>
#include <vector>

#include <IRCSeeker.h>
#include <Channel.h>

bool	intInVector(std::vector<int> &v, int x)
{
	std::vector<int>::iterator	it;
	for (it = v.begin(); it != v.end(); ++it)
	{
		if (*it == x)
			return 1;
	}
	return 0;
}

bool	removeIfVector(std::vector<int> &v, int x)
{
	bool returnValue = 0;
	for (size_t id = 0; id < v.size();)
	{
		if (v[id] == x)
		{
			IRC_WARN("anti crash");
			v.erase(v.begin() + id);
			returnValue = 1;
			IRC_LOG("didnt crash");
			continue ;
		}
		id++;
	}
	IRC_OK("finshed");
	return returnValue;
}

bool	Channel::isInvited(int fdClient) {return intInVector(_invitedClient, fdClient);}

Channel::Channel(str channelName, int firstClient): ATarget(), _name(channelName), _inviteOnlyChannel(false), _activePassword(0), _userLimit(100), _topicIsSet(0), _topicPermNeeded(0)
{
	IRC_LOG("Channel constructor called : |%s|", channelName.c_str());

	set_targetName(_name);
	_fdAdminClient.push_back(firstClient);
}

Channel::~Channel(void)
{
	IRC_LOG("Channel destructor called.");
}

void	Channel::_addClient(int fdClient, int perm)
{
	if (perm)
		_fdAdminClient.push_back(fdClient);
	else
		_fdClient.push_back(fdClient);
}
void	Channel::addClient(int fdClient, const str *password)
{
	if (removeIfVector(_invitedClient, fdClient))
		goto addClientLabel;
	if (_activePassword && (!password || *password != _password))
		throw InvalidChannelKeyException();
	
	if (intInVector(_fdClient, fdClient))
		throw ClientIsInChannelException();
	if (intInVector(_fdAdminClient, fdClient))
		throw ClientIsInChannelException();
addClientLabel:
	_fdClient.push_back(fdClient);
}

int	Channel::removeClient(int fdClient)
{
	std::vector<int>::iterator	it;

	if (removeIfVector(_fdClient, fdClient))
		goto end;
	if (removeIfVector(_fdAdminClient, fdClient))
		goto end;
	throw ClientNotInChannelException();
end:
	IRC_LOG("remove client; nb of client left : %d", get_size());
	return (get_size());
}

bool	Channel::havePerm(int fdClient)
{
	IRC_AUTO it = std::find(_fdAdminClient.begin(), _fdAdminClient.end(), fdClient);
	if (it != _fdAdminClient.end())
		return true;

	it = std::find(_fdClient.begin(), _fdClient.end(), fdClient);
	if (it != _fdClient.end())
		return false;
	throw ClientNotInChannelException();
}

void	Channel::givePerm(int userClient, int targetClient)
{
	if (havePerm(userClient) && !havePerm(targetClient))
	{
		removeClient(targetClient);
		_addClient(targetClient, 1);
	}
}
void	Channel::removePerm(int targetClient)
{
	if (havePerm(targetClient))
	{
		removeClient(targetClient);
		_addClient(targetClient, 0);
	}
}

#include <unistd.h>

void	Channel::_broadcast(const str &string) const
{
	IRC_AUTO	it = _fdClient.begin();

	IRC_LOG("Channel Brodcast " BOLD(COLOR(YELLOW,"%s")) " to %d client", string.c_str(), get_size());

	for (; it != _fdClient.end(); ++it)
		write(*it, (string + "\r\n").c_str(), string.size() + 2);

	it = _fdAdminClient.begin();

	for (; it != _fdAdminClient.end(); ++it)
		write(*it, (string + "\r\n").c_str(), string.size() + 2);

	IRC_LOG("succesfull Broadcast, it suprisely didnt segfault");
}

void	Channel::sendMsg(const str &string) const
{
	_broadcast(string);
}
