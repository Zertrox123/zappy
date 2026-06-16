##
## EPITECH PROJECT, 2026
## Zappy/server/Makefile
##

NAME	= zappy_server
CARGO	= cargo

all: ../$(NAME)
.PHONY: all

../$(NAME):
	@$(CARGO) build --release
	@cp target/release/$(NAME) ../$(NAME)

clean:
	@$(CARGO) clean
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

check:
	@if ! $(CARGO) check -q 2>/dev/null; then \
		echo "[zappy_server] check: failed (build error)" >&2; \
		exit 1; \
	fi
	@echo "[zappy_server] check: passed"
.PHONY: check

format:
	@$(CARGO) fmt --check || { \
		echo "[zappy_server] format: failed" >&2; \
		exit 1; \
	}
	@echo "[zappy_server] format: passed"
.PHONY: format

tests_run:
	@$(CARGO) test
.PHONY: tests_run

re: fclean all
.PHONY: re
