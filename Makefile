NAME = bin/webserv

CONFIG =	Config.cpp \
			Location.cpp

CORE = 		Client.cpp \
			Server.cpp \
			ServerConfig.cpp

HANDLERS =	DeleteHandler.cpp \
			GetHandler.cpp \
			PostHandler.cpp \
			RequestHandler.cpp\
			CgiHandler.cpp \
			RedirectHandler.cpp \
			NotImplementedHandler.cpp

HTTP =		HttpRequest.cpp \
			HttpResponse.cpp \
			HttpStatus.cpp

UTILS = 	Utils.cpp

FOLDERS = 	config/ \
			core/ \
			handlers/ \
			http/ \
			utils/

FILES = $(addprefix config/, $(CONFIG)) \
		$(addprefix core/, $(CORE)) \
		$(addprefix handlers/, $(HANDLERS)) \
		$(addprefix http/, $(HTTP)) \
		$(addprefix utils/, $(UTILS))
		
SRC = $(addprefix src/, $(FILES))
OBJ = $(addprefix obj/, ${FILES:%.cpp=%.o})
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) src/main.cpp -o $@ $^

obj/%.o: src/%.cpp
	@mkdir -p $(addprefix obj/, $(FOLDERS))
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj/*

fclean: clean
	rm -rf $(NAME)

test:
	@cd tests && bash get_test.sh \
	&& bash delete_test.sh \
	&& bash post_test.sh
	@make fclean

re: fclean all