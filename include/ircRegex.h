/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircRegex.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 15:48:43 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 23:18:42 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_REGEX_H
# define IRC_REGEX_H

# define	R_CHAR_RANGE(s, e)	s "-" e
# define	R_CHAR_GROUP(g)		"[" g "]"
# define	R_CHAR_INV_GROUP(g)	"[^" g "]"
# define	R_CAPTURE(c)		"(" c ")"

# define	R_1_OR_MORE(t)		t "+"
# define	R_0_OR_MORE(t)		t "*"
# define	R_0_OR_1(t)			t "?"
# define	R_AS_FEW(t)			R_0_OR_1(t)

# define	R_X_OR_MORE(t, x)	t "{"#x",}"
# define	R_X_EXACT(t, x)		t "{"#x"}"
# define	R_X_TO_Y(t, x, y)	t "{"#x","#y"}"
# define	R_1_TO_Y(t, y)		R_X_TO_Y(t,1,y)
# define	R_ALTERNATION(x, y)	x "|" y

# define	R_DIGIT_RANGE		R_CHAR_RANGE("0","9")
# define	R_LOWER_RANGE		R_CHAR_RANGE("a","z")
# define	R_UPPER_RANGE		R_CHAR_RANGE("A","Z")
# define	R_ALPHA_RANGE		R_LOWER_RANGE R_UPPER_RANGE
# define	R_ALNUM_RANGE		R_DIGIT_RANGE R_ALPHA_RANGE

# define	R_DIGIT				R_CHAR_GROUP(R_DIGIT_RANGE)
# define	R_LOWER				R_CHAR_GROUP(R_LOWER_RANGE)
# define	R_UPPER				R_CHAR_GROUP(R_UPPER_RANGE)
# define	R_ALPHA				R_CHAR_GROUP(R_ALPHA_RANGE)
# define	R_ALNUM				R_CHAR_GROUP(R_ALNUM_RANGE)

# define	R_ANY				"."
# define	R_START_STRING		"^"
# define	R_END_STRING		"$"
# define	R_ESCAPE(c)			"\\" c
# define	R_BACKREF(n)		"\\"#n

# define	R_CAPTURE_INT		R_CAPTURE(R_DIGIT R_1_OR_MORE)
# define	R_CAPTURE_WORD		R_CAPTURE(R_ALPHA R_1_OR_MORE)

//TODO regex join(ect) : JOIN CHANNEL,CHANNEL,CHANNEL KEY,KEY,KEY


#endif
