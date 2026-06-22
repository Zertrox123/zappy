##
## EPITECH PROJECT, 2026
## Zappy/gui/Makefile
##

NAME		= zappy_gui
CXX		= clang++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++20 -I.
LDFLAGS		=
CLANG_FORMAT	= clang-format

SFML_CFLAGS	:=
SFML_LIBS	:=
UNAME_S		:= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SFML_PREFIX	:= $(shell brew --prefix sfml@2 2>/dev/null)
ifneq ($(SFML_PREFIX),)
SFML_CFLAGS	= -I$(SFML_PREFIX)/include
SFML_LIBS	= -L$(SFML_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system
endif
else
SFML_CFLAGS	:= $(shell pkg-config --cflags sfml-graphics 2>/dev/null)
SFML_LIBS	:= $(shell pkg-config --libs sfml-graphics 2>/dev/null)
endif
ifeq ($(SFML_LIBS),)
SFML_PREFIX	:= $(shell brew --prefix sfml@2 2>/dev/null)
ifneq ($(SFML_PREFIX),)
SFML_CFLAGS	= -I$(SFML_PREFIX)/include
SFML_LIBS	= -L$(SFML_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system
endif
endif
ifneq ($(SFML_LIBS),)
CXXFLAGS	+= $(SFML_CFLAGS) -Wno-deprecated-declarations
LDFLAGS		+= $(SFML_LIBS)
endif

SRCS		= main.cpp \
		  config/CliConfig.cpp \
		  net/NetworkClient.cpp \
		  net/ReceiveBuffer.cpp \
		  protocol/GraphicHandshake.cpp \
		  protocol/ProtocolParser.cpp \
		  model/GameState.cpp \
		  app/GuiApp.cpp \
		  render/GuiWindow.cpp

HEADERS		= config/CliConfig.hpp \
		  config/Headless.hpp \
		  net/NetworkClient.hpp \
		  net/ReceiveBuffer.hpp \
		  protocol/GraphicHandshake.hpp \
		  protocol/ProtocolParser.hpp \
		  model/Tile.hpp \
		  model/GameState.hpp \
		  app/GuiApp.hpp \
		  render/GuiWindow.hpp

OBJS		= $(SRCS:.cpp=.o)

CHECK_HOST	= 127.0.0.1
CHECK_PORT	= 4242

TEST_BIN		= tests/test_receive_buffer
PROTOCOL_TEST_BIN	= tests/test_protocol_parser

all: ../$(NAME)
.PHONY: all

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

../$(NAME): $(NAME)
	@cp $(NAME) ../$(NAME)

check: $(NAME)
	@./$(NAME) >/dev/null 2>&1; rc=$$?; \
	if [ $$rc -ne 84 ]; then \
		echo "[zappy_gui] check: failed (expected exit 84 without args, got $$rc)" >&2; \
		exit 1; \
	fi
	@ZAPPY_GUI_HEADLESS=1 ./$(NAME) -p $(CHECK_PORT) -h $(CHECK_HOST) >/dev/null 2>&1; rc=$$?; \
	if [ $$rc -ne 0 ] && [ $$rc -ne 84 ]; then \
		echo "[zappy_gui] check: failed (expected exit 0 or 84 with valid args, got $$rc)" >&2; \
		exit 1; \
	fi
	@echo "[zappy_gui] check: passed"
.PHONY: check

format: $(SRCS) $(HEADERS)
	@$(CLANG_FORMAT) --dry-run -Werror $(SRCS) $(HEADERS) || { \
		echo "[zappy_gui] format: failed" >&2; \
		exit 1; \
	}
	@echo "[zappy_gui] format: passed"
.PHONY: format

format-fix: $(SRCS) $(HEADERS)
	@$(CLANG_FORMAT) -i $(SRCS) $(HEADERS)
	@echo "[zappy_gui] format-fix: applied"
.PHONY: format-fix

$(TEST_BIN): tests/test_receive_buffer.cpp net/ReceiveBuffer.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_receive_buffer.cpp net/ReceiveBuffer.cpp

$(PROTOCOL_TEST_BIN): tests/test_protocol_parser.cpp model/GameState.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_protocol_parser.cpp model/GameState.cpp

tests_run: all $(TEST_BIN) $(PROTOCOL_TEST_BIN)
	@./$(TEST_BIN) || { echo "[zappy_gui] tests_run: receive buffer failed" >&2; exit 1; }
	@./$(PROTOCOL_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: protocol parser failed" >&2; exit 1; \
	}
	@ZAPPY_GUI_HEADLESS=1 ./tests/run_handshake_test.sh ../$(NAME) || { \
		echo "[zappy_gui] tests_run: handshake failed" >&2; \
		exit 1; \
	}
	@echo "[zappy_gui] tests_run: passed"
.PHONY: tests_run

clean:
	@rm -f $(OBJS) $(NAME) $(TEST_BIN) $(PROTOCOL_TEST_BIN)
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

re: fclean all
.PHONY: re
