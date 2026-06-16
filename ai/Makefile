##
## EPITECH PROJECT, 2026
## Zappy/ai/Makefile
##

NAME	= zappy_ai
PYTHON	= python3
BLACK	= black

SOURCES	= main.py config.py

all: ../$(NAME)
.PHONY: all

../$(NAME): $(SOURCES)
	@{ \
		echo '#!/bin/sh'; \
		echo 'cd "$$(dirname "$$0")/ai" && exec python3 main.py "$$@"'; \
	} > ../$(NAME)
	@chmod +x ../$(NAME)

CHECK_PORT	= 4242
CHECK_TEAM	= team1
CHECK_HOST	= 127.0.0.1

check: $(SOURCES)
	@$(PYTHON) -m py_compile $(SOURCES) 2>/dev/null || { \
		echo "[zappy_ai] check: failed (build error)" >&2; \
		exit 1; \
	}
	@$(PYTHON) main.py --help >/dev/null 2>&1 || { \
		echo "[zappy_ai] check: failed (--help run error)" >&2; \
		exit 1; \
	}
	@$(PYTHON) main.py >/dev/null 2>&1; rc=$$?; \
	if [ $$rc -ne 84 ]; then \
		echo "[zappy_ai] check: failed (expected exit 84 without args, got $$rc)" >&2; \
		exit 1; \
	fi
	@$(PYTHON) main.py -p $(CHECK_PORT) -n $(CHECK_TEAM) -h $(CHECK_HOST) >/dev/null 2>&1 || { \
		echo "[zappy_ai] check: failed (valid args run error)" >&2; \
		exit 1; \
	}
	@echo "[zappy_ai] check: passed"
.PHONY: check

format:
	@$(BLACK) --check . || { \
		echo "[zappy_ai] format: failed" >&2; \
		exit 1; \
	}
	@echo "[zappy_ai] format: passed"
.PHONY: format

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
