/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircRegex.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 15:48:43 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/11 16:16:31 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_REGEX_H
# define IRC_REGEX_H

# define	R_CHAR_RANGE(s, e)	(s)"-"(e)
# define	R_CHAR_GROUP(g)		"["(g)"]"
# define	R_CAPTURE(c)		"("(c)")"

# define	R_1_OR_MORE(x)		(x)"+"
# define	R_0_OR_MORE(x)		(x)"*"
# define	R_0_OR_1(x)			(x)"?"

# define	R_DIGIT_RANGE		R_CHAR_RANGE("0","9")
# define	R_LOWER_RANGE		R_CHAR_RANGE("a","z")
# define	R_UPPER_RANGE		R_CHAR_RANGE("A","Z")
# define	R_ALPHA_RANGE		R_LOWER_RANGE""R_UPPER_RANGE
# define	R_ALNUM_RANGE		R_DIGIT_RANGE""R_ALPHA_RANGE

# define	R_DIGIT				R_CHAR_GROUP(R_DIGIT_RANGE)
# define	R_LOWER				R_CHAR_GROUP(R_LOWER_RANGE)
# define	R_UPPER				R_CHAR_GROUP(R_UPPER_RANGE)
# define	R_ALPHA				R_CHAR_GROUP(R_ALPHA_RANGE)
# define	R_ALNUM				R_CHAR_GROUP(R_ALNUM_RANGE)

# define	R_CAPTURE_INT		R_CAPTURE(R_DIGIT""R_1_OR_MORE)
# define	R_CAPTURE_WORD		R_CAPTURE(R_ALPHA""R_1_OR_MORE)

#endif
