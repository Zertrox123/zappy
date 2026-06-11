##
## EPITECH PROJECT, 2026
## Zappy/ai/Makefile
##

NAME	= zappy_ai
PYTHON	= python3

all: ../$(NAME)
.PHONY: all

../$(NAME): main.py
	@cp main.py ../$(NAME)
	@chmod +x ../$(NAME)

check: main.py
	@if ! $(PYTHON) -m py_compile main.py 2>/dev/null; then \
		echo "[zappy_ai] check: failed (build error)" >&2; \
		exit 1; \
	fi
	@if ! $(PYTHON) main.py >/dev/null 2>&1; then \
		echo "[zappy_ai] check: failed (run error)" >&2; \
		exit 1; \
	fi
	@echo "[zappy_ai] check: passed"
.PHONY: check

tests_run: all
	@../$(NAME)
.PHONY: tests_run

clean:
	@rm -rf __pycache__
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

re: fclean all
.PHONY: re
