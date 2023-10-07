NAME=webserv

SRCS=$(shell find . -type f -name '*.cpp')

OBJS=$(patsubst %.cpp,%.o,$(SRCS)) ./srcs/main.o

TOOL_OBJS=$(patsubst %.cpp,%.o,$(SRCS)) ./tools/main.o

CFLAGS=-Wall -Wextra -Werror -std=c++20
COBJFLAGS=

.PHONY: all re fclean clean debug re-tool

all: $(NAME)

$(NAME): $(OBJS)
	c++ $(CFLAGS) $^ -o $@

%.o:%.cpp
	c++ $(CFLAGS) $(COBJFLAGS) -c $< -o $@

re: fclean all

fclean: clean
	$(RM) $(NAME) tool

clean:
	$(RM) $(OBJS)

debug: re

ifeq ($(MAKECMDGOALS), debug)
CFLAGS += -O0 -g3 -DDEBUG=1
endif
