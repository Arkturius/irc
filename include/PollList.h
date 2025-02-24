/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PollList.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 14:05:43 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/23 15:45:19 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POLLLIST_H
# define POLLLIST_H

# include <irc.h>

typedef struct
{
	void	*next;
}	FreeList;

class	PollList
{
	private:
		struct pollfd	_array[IRC_CLIENT_CAP];
		uint32_t		_size;
		struct pollfd	*_free;
		uint32_t		_freeSize;

	public:
		PollList(void): _size(0), _free(NULL), _freeSize(0) { IRC_BZERO(_array); };
		~PollList(void) {};

		void	push(struct pollfd client)
		{
			if (size() == IRC_CLIENT_CAP)
				throw OutOfBoundsException();

			IRC_LOG("Pushing into poll list.");

			struct pollfd	*spot = _free;

			if (!spot) { spot = &_array[_size]; }
			else { _freeSize--; }

			_size++;
			*spot = client;

			IRC_OK("Element pushed. NEW _size = %u, NEW _freeSize = %u", _size, _freeSize);
		}

		void	erase(int n)
		{
			if (n < 0 || n >= (int)size())
				throw OutOfBoundsException();
			if (_size == 0)
				return ;

			((FreeList *)&_array[n])->next = _free;
			_free = &_array[n];
			_freeSize++;

			_array[n] = (struct pollfd){.fd = 0, .events = 0, .revents = 0};
			_size--;
		}

		uint32_t	size() const { return (_size + _freeSize); } 

		struct pollfd	&operator[](int n)
		{
			if (n < 0 || n >= IRC_CLIENT_CAP)
				throw	OutOfBoundsException();
			return _array[n];
		}

		const struct pollfd	&operator[](int n) const
		{
			if (n < 0 || n >= IRC_CLIENT_CAP)
				throw	OutOfBoundsException();
			return _array[n];
		}
	
	EXCEPTION(OutOfBoundsException, "Trying to access out of bounds.");
};

#endif
