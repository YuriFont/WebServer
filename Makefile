NAME = bin/webserv
FILES = main.cpp
SRC = $(addprefix src/, $(FILES))
OBJ = $(addprefix obj/, ${FILES:%.cpp=%.o})
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj/*

fclean: clean
	rm -rf $(NAME)

re: fclean all