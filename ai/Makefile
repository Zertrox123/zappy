##
## EPITECH PROJECT, 2026
## Zappy/ai/Makefile
##

NAME	= zappy_ai
PYTHON	= python3

SOURCES	= main.py config.py

all: ../$(NAME)
.PHONY: all

../$(NAME): $(SOURCES)
	@{ \
		echo '#!/bin/sh'; \
		echo 'cd "$$(dirname "$$0")/ai" && exec python3 main.py "$$@"'; \
	} > ../$(NAME)
	@chmod +x ../$(NAME)

check: $(SOURCES)
	@$(PYTHON) -m py_compile $(SOURCES) 2>/dev/null || { \
		echo "[zappy_ai] check: failed (build error)" >&2; \
		exit 1; \
	}
	@$(PYTHON) main.py --help >/dev/null 2>&1 || { \
		echo "[zappy_ai] check: failed (run error)" >&2; \
		exit 1; \
	}
	@echo "[zappy_ai] check: passed"
.PHONY: check

tests_run: all
	@$(PYTHON) -m unittest discover -s tests -p 'test_*.py' -q
	@../$(NAME) --help >/dev/null
.PHONY: tests_run

clean:
	@rm -rf __pycache__ tests/__pycache__
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

re: fclean all
.PHONY: re
