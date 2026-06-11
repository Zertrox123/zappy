##
## EPITECH PROJECT, 2026
## Zappy/Makefile
##

NAME_SERVER	= zappy_server
NAME_GUI	= zappy_gui
NAME_AI		= zappy_ai

all: $(NAME_SERVER) $(NAME_GUI) $(NAME_AI)
.PHONY: all

$(NAME_SERVER):
	@$(MAKE) -C server
.PHONY: $(NAME_SERVER)

$(NAME_GUI):
	@$(MAKE) -C gui
.PHONY: $(NAME_GUI)

$(NAME_AI):
	@$(MAKE) -C ai
.PHONY: $(NAME_AI)

tests_run:
	@$(MAKE) -C server tests_run
	@$(MAKE) -C gui tests_run
	@$(MAKE) -C ai tests_run
.PHONY: tests_run

clean:
	@$(MAKE) -C server clean
	@$(MAKE) -C gui clean
	@$(MAKE) -C ai clean
.PHONY: clean

fclean: clean
	@rm -f $(NAME_SERVER) $(NAME_GUI) $(NAME_AI)
.PHONY: fclean

re: fclean all
.PHONY: re

check:
	@$(MAKE) -C server check
	@$(MAKE) -C gui check
	@$(MAKE) -C ai check
	@echo "[zappy] all checks passed"
.PHONY: check
