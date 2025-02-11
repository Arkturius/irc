/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegexMatch.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by yroussea          #+#    #+#             */
/*   Updated: 2025/02/11 16:02:02 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REGEX_MATCH_H
# define REGEX_MATCH_H

# include <string>
# include <regex.h>

typedef std::string str;

bool	regex_match(const str regex, const char *string);
bool	regex_find(const str regex, const char *string, regmatch_t *pmatch);

#endif // REGEX_MATCH_H
