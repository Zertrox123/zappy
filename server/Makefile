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

tests_run:
	@$(CARGO) test
.PHONY: tests_run

re: fclean all
.PHONY: re
