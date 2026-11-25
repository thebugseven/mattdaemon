CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17

NAME = Matt_daemon

SRC_DIR = src/
OBJ_DIR = .obj/
INCLUDE = ./include/

SRC_FILES = main \
            LockFile \
            Tintin_reporter \
			TcpServer \
			SignalManager

OBJ_FILES = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(SRC_FILES)))
HEADERS = $(wildcard $(INCLUDE)*.hpp)

all: $(NAME)

$(NAME): $(OBJ_FILES)
	$(CC) $(CXXFLAGS) -I $(INCLUDE) -o $@ $(OBJ_FILES)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -I $(INCLUDE) -c $< -o $@

$(OBJ_FILES): Makefile $(HEADERS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re