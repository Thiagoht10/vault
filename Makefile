CXX = c++

FLAGS = -Wall -Werror -Wextra

LDFLAGS = -lsodium

INCLUDE = -Iincludes

NAME = vault

SRC = $(wildcard src/*.cpp) main.cpp

OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

OBJ_DIR = obj

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) $(OBJ) $(LDFLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(OBJ)
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
