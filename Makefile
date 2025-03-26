# Makefile

NAME = webserv

SRCS =	srcs/main.cpp \
		srcs/bonus/*.cpp \
		srcs/HTTP_Methods/*.cpp \
		srcs/parsing/*.cpp \
		srcs/serverFiles/*.cpp \
		srcs/socketFiles/*.cpp \
		srcs/utils/*.cpp

CXX = g++

HEADER_FILES = -I./srcs/bonus -I./srcs/parsing -I./srcs/serverFiles -I./srcs/socketFiles -I./srcs/utils

CXXFLAGS = -g -Wall -Wextra -Werror -std=c++98 $(HEADER_FILES)

all: $(NAME)

$(NAME): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(SRCS)
	echo "${BOLD}${GREEN}Compilation successful${NO_COLOR}";

clean:
	rm -f $(NAME)
	echo "${BOLD}${RED}Executable removed${NO_COLOR}";

re: clean all

.PHONY: all clean fclean re

.SILENT: all $(NAME) clean fclean re


NO_COLOR =	\033[0m
BOLD	 =	\033[1m
YELLOW	 =	\033[33m
RED 	 = 	\033[1;31m
GREEN 	 = 	\033[1;32m
