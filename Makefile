NAME=webserv

#find ./srcs -type f -name '*.cpp' -and -not -name 'main.cpp'
SRCS=$(shell find ./srcs -type f -name '*.cpp' -and -not -name 'main.cpp')

OBJS=$(patsubst %.cpp,%.o,$(SRCS)) ./srcs/main.o

TOOL_OBJS=$(patsubst %.cpp,%.o,$(SRCS)) ./tools/main.o

CFLAGS=-Wall -Wextra -Werror -std=c++98
COBJFLAGS=-Isrcs

.PHONY: all re fclean clean debug re-tool

all: $(NAME)

tool: $(TOOL_OBJS)
	c++ $(CFLAGS) $^ -o $@

re-tool: fclean tool
	./tool default.conf
	$(MAKE) re

$(NAME): $(OBJS)
	c++ $(CFLAGS) $^ -o $@

%.o:%.cpp
	c++ $(CFLAGS) $(COBJFLAGS) -c $< -o $@

re: fclean all

fclean: clean
	$(RM) $(NAME) tool

clean:
	$(RM) $(OBJS) $(TOOL_OBJS)

debug-tool: re-tool

debug: re

ifeq ($(MAKECMDGOALS), debug-tool)
CFLAGS += -O0 -g3 -DDEBUG=1
else
ifeq ($(MAKECMDGOALS), debug)
CFLAGS += -O0 -g3 -DDEBUG=1
endif
endif
