CC     = c++
FLAGS  = -Wall -Werror -Wextra -g3 -std=c++98

WAY = src/
WAY_H = inc/

SRCS   = $(WAY)main.cpp \
		$(WAY)Server.cpp \
		$(WAY)Client.cpp \
		$(WAY)Channel.cpp
		
HEADER  = $(WAY_H)main.cpp \
		$(WAY_H)Server.cpp \
		$(WAY_H)Client.cpp \
		$(WAY_H)Channel.cpp


OBJS   = $(SRCS:.cpp=.o)

NAME   = ircserv

all: $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
