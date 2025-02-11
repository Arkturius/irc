# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rgramati <rgramati@student.42angouleme.fr  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/05 17:09:56 by rgramati          #+#    #+#              #
#    Updated: 2025/02/11 12:17:27 by rgramati         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	ircserv

SRC_DIR		:=	src
INC_DIR		:=	include
OBJ_DIR		:=	build

include		sources.mk
SRCS		:=	$(addprefix $(SRC_DIR)/,$(SRCS))
OBJS 		:=	$(addprefix $(OBJ_DIR)/, $(SRCS:%.cpp=%.o))

CC			:=	clang++

CFLAGS		:=	-Wall -Wextra -Werror -std=c++98

ifeq ($(TEMP), 1)
	CFLAGS	+=	-save-temps
endif

ifeq ($(VERBOSE), 1)
	CFLAGS	+=	-DIRC_VERBOSE=1
endif

IFLAGS		:=	-I$(INC_DIR)

RM			:=	rm -rf

#
# Rules
#

all:			$(NAME)

$(NAME):	 	$(OBJS)
	@$(CC) $(CFLAGS) $(IFLAGS) -o $(NAME) $^
	@echo " $(GREEN)$(BOLD)$(ITALIC)■$(RESET)  building	$(GRAY)$(BOLD)$(ITALIC)$(NAME)$(RESET)"

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo " $(CYAN)$(BOLD)$(ITALIC)■$(RESET)  compiling	$(GRAY)$(BOLD)$(ITALIC)$(notdir $@)$(RESET) from $(GRAY)$(BOLD)$(ITALIC)$(notdir $^)$(RESET)"
	@$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $^

clean:
	echo " $(RED)$(BOLD)$(ITALIC)■$(RESET)  deleted	$(GRAY)$(BOLD)$(ITALIC)$(OBJS)$(RESET)"
	$(RM) $(OBJS)
	$(RM) $(OBJ_DIR)

fclean:			clean
	echo " $(RED)$(BOLD)$(ITALIC)■$(RESET)  deleted	$(GRAY)$(BOLD)$(ITALIC)$(NAME)$(RESET)"
	$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean test re
.SILENT:	all clean fclean test re

#
# Ansi
# 

BOLD			=	\033[1m
ITALIC			=	\033[3m

RED				=	\033[31m
GREEN			=	\033[32m
YELLOW			=	\033[33m
CYAN			=	\033[36m

RESET			=	\033[0m

LINE_CLR		=	\33[2K\r
