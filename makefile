CC              = c++
CXXFLAGS        = -Wall -Wextra -Werror -std=c++17

NAME            = MattDeamon
CLIENT_NAME     = MattDeamonctl

SRC_DIR         = src/
OBJ_DIR         = .obj/
INCLUDE         = ./include/

SERVER_SRC_FILES =	main \
					LockFile

# CLIENT_SRC_FILES = taskmasterctl \
#                    Shell \
#                    TcpCommandClient

SERVER_OBJ = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(SERVER_SRC_FILES)))
CLIENT_OBJ = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(CLIENT_SRC_FILES)))
HEADER = $(wildcard $(INCLUDE)*.hpp)

all: $(NAME) $(CLIENT_NAME)

$(NAME): $(SERVER_OBJ)
	$(CC) $(CXXFLAGS) -I $(INCLUDE) -o $@ $(SERVER_OBJ) -lyaml-cpp

$(CLIENT_NAME): $(CLIENT_OBJ)
	$(CC) $(CXXFLAGS) -I $(INCLUDE) -o $@ $(CLIENT_OBJ) -lreadline

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -I $(INCLUDE) -c $< -o $@

$(SERVER_OBJ) $(CLIENT_OBJ): Makefile $(HEADER)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(CLIENT_NAME)

re: fclean all

.PHONY: all clean fclean re