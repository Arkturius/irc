/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegexMatch.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/13 23:35:40 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REGEX_MATCH_H
# define REGEX_MATCH_H

# include <regex.h>

bool	regex_match(const char *regex, const char *string);
bool	regex_find(const char *regex, const char *string, regmatch_t *pmatch);

#endif // REGEX_MATCH_H
