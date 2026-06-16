##
## EPITECH PROJECT, 2026
## Zappy/gui/Makefile
##

NAME		= zappy_gui
CXX		= clang++
CXXFLAGS	= -Wall -Wextra -std=c++20
CLANG_FORMAT	= clang-format
SOURCES		= main.cpp

all: ../$(NAME)
.PHONY: all

$(NAME): main.cpp
	@$(CXX) $(CXXFLAGS) -o $(NAME) main.cpp

../$(NAME): $(NAME)
	@cp $(NAME) ../$(NAME)

check: main.cpp
	@if ! $(CXX) $(CXXFLAGS) -o $(NAME) main.cpp 2>/dev/null; then \
		echo "[zappy_gui] check: failed (build error)" >&2; \
		exit 1; \
	fi
	@if ! ./$(NAME) >/dev/null 2>&1; then \
		echo "[zappy_gui] check: failed (run error)" >&2; \
		exit 1; \
	fi
	@echo "[zappy_gui] check: passed"
.PHONY: check

format: $(SOURCES)
	@$(CLANG_FORMAT) --dry-run -Werror $(SOURCES) || { \
		echo "[zappy_gui] format: failed" >&2; \
		exit 1; \
	}
	@echo "[zappy_gui] format: passed"
.PHONY: format

tests_run: all
	@../$(NAME)
.PHONY: tests_run

clean:
	@rm -f $(NAME)
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

re: fclean all
.PHONY: re
