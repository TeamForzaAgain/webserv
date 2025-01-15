# Makefile

NAME = webserv

SRCS = main.cpp ServerManager.cpp Server.cpp ListeningSocket.cpp ClientSocket.cpp

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror #-std=c++98

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