##
## EPITECH PROJECT, 2026
## Zappy/gui/Makefile
##

NAME	= zappy_gui
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

CHECK_HOST	= 127.0.0.1
CHECK_PORT	= 4242

check: main.cpp
	@if ! $(CXX) $(CXXFLAGS) -o $(NAME) main.cpp 2>/dev/null; then \
		echo "[zappy_gui] check: failed (build error)" >&2; \
		exit 1; \
	fi
	@./$(NAME) >/dev/null 2>&1; rc=$$?; \
	if [ $$rc -ne 84 ]; then \
		echo "[zappy_gui] check: failed (expected exit 84 without args, got $$rc)" >&2; \
		exit 1; \
	fi
	@./$(NAME) -p $(CHECK_PORT) -h $(CHECK_HOST) >/dev/null 2>&1; rc=$$?; \
	if [ $$rc -ne 0 ] && [ $$rc -ne 84 ]; then \
		echo "[zappy_gui] check: failed (expected exit 0 or 84 with valid args, got $$rc)" >&2; \
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

format-fix: $(SOURCES)
	@$(CLANG_FORMAT) -i $(SOURCES)
	@echo "[zappy_gui] format-fix: applied"
.PHONY: format-fix

tests_run: all
	@../$(NAME) -p $(CHECK_PORT) -h $(CHECK_HOST) >/dev/null 2>&1; rc=$$?; \
	if [ $$rc -ne 0 ] && [ $$rc -ne 84 ]; then \
		echo "[zappy_gui] tests_run: failed (expected exit 0 or 84, got $$rc)" >&2; \
		exit 1; \
	fi
.PHONY: tests_run

clean:
	@rm -f $(NAME)
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

re: fclean all
.PHONY: re
