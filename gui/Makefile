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
		  protocol/Commands.cpp \
		  protocol/MapSync.cpp \
		  model/GameState.cpp \
		  app/GuiApp.cpp \
		  render/GuiWindow.cpp \
		  render/PlayerAnimator.cpp \
		  render/PlayerRenderer.cpp \
		  render/MapCamera.cpp \
		  render/MapIcons.cpp \
		  render/PanInput.cpp \
		  render/ZoomInput.cpp \
		  render/Selection.cpp \
		  render/Sidebar.cpp \
		  render/EffectRenderer.cpp \
		  render/UiFont.cpp

HEADERS		= config/CliConfig.hpp \
		  config/Headless.hpp \
		  net/NetworkClient.hpp \
		  net/ReceiveBuffer.hpp \
		  protocol/GraphicHandshake.hpp \
		  protocol/ProtocolParser.hpp \
		  protocol/ICommand.hpp \
		  protocol/Commands.hpp \
		  protocol/ProtocolParse.hpp \
		  protocol/MapSync.hpp \
		  model/Tile.hpp \
		  model/GameState.hpp \
		  app/GuiApp.hpp \
		  render/GuiWindow.hpp \
		  render/PlayerAnimator.hpp \
		  render/PlayerRenderer.hpp \
		  render/MapCamera.hpp \
		  render/MapIcons.hpp \
		  render/PanInput.hpp \
		  render/ZoomInput.hpp \
		  render/Selection.hpp \
		  render/Sidebar.hpp \
		  render/EffectRenderer.hpp \
		  render/UiFont.hpp \
		  model/WorldEffect.hpp

OBJS		= $(SRCS:.cpp=.o)

CHECK_HOST	= 127.0.0.1
CHECK_PORT	= 4242

TEST_BIN		= tests/test_receive_buffer
PROTOCOL_TEST_BIN	= tests/test_protocol_parser
ANIMATOR_TEST_BIN	= tests/test_player_animator
SELECTION_TEST_BIN	= tests/test_selection
EFFECTS_TEST_BIN	= tests/test_world_effects
FONT_TEST_BIN		= tests/test_ui_font
CAMERA_TEST_BIN		= tests/test_map_camera
PAN_TEST_BIN		= tests/test_pan_input
ZOOM_TEST_BIN		= tests/test_zoom_input

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

$(PROTOCOL_TEST_BIN): tests/test_protocol_parser.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_protocol_parser.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp

$(ANIMATOR_TEST_BIN): tests/test_player_animator.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp \
		render/PlayerAnimator.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_player_animator.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp \
		render/PlayerAnimator.cpp

$(SELECTION_TEST_BIN): tests/test_selection.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp \
		render/PlayerAnimator.cpp render/Selection.cpp net/NetworkClient.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_selection.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp \
		render/PlayerAnimator.cpp render/Selection.cpp net/NetworkClient.cpp

$(EFFECTS_TEST_BIN): tests/test_world_effects.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_world_effects.cpp model/GameState.cpp \
		net/ReceiveBuffer.cpp protocol/ProtocolParser.cpp protocol/Commands.cpp

$(FONT_TEST_BIN): tests/test_ui_font.cpp render/UiFont.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_ui_font.cpp render/UiFont.cpp $(LDFLAGS)

$(CAMERA_TEST_BIN): tests/test_map_camera.cpp render/MapCamera.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_map_camera.cpp render/MapCamera.cpp \
		$(LDFLAGS)

$(PAN_TEST_BIN): tests/test_pan_input.cpp render/PanInput.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_pan_input.cpp render/PanInput.cpp

$(ZOOM_TEST_BIN): tests/test_zoom_input.cpp render/ZoomInput.cpp
	@$(CXX) $(CXXFLAGS) -o $@ tests/test_zoom_input.cpp render/ZoomInput.cpp

tests_run: all $(TEST_BIN) $(PROTOCOL_TEST_BIN) $(ANIMATOR_TEST_BIN) \
	$(SELECTION_TEST_BIN) $(EFFECTS_TEST_BIN) $(FONT_TEST_BIN) \
	$(CAMERA_TEST_BIN) $(PAN_TEST_BIN) $(ZOOM_TEST_BIN)
	@./$(TEST_BIN) || { echo "[zappy_gui] tests_run: receive buffer failed" >&2; exit 1; }
	@./$(PROTOCOL_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: protocol parser failed" >&2; exit 1; \
	}
	@./$(ANIMATOR_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: player animator failed" >&2; exit 1; \
	}
	@./$(SELECTION_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: selection failed" >&2; exit 1; \
	}
	@./$(EFFECTS_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: world effects failed" >&2; exit 1; \
	}
	@./$(FONT_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: ui font failed" >&2; exit 1; \
	}
	@./$(CAMERA_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: map camera failed" >&2; exit 1; \
	}
	@./$(PAN_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: pan input failed" >&2; exit 1; \
	}
	@./$(ZOOM_TEST_BIN) || { \
		echo "[zappy_gui] tests_run: zoom input failed" >&2; exit 1; \
	}
	@ZAPPY_GUI_HEADLESS=1 ./tests/run_handshake_test.sh ../$(NAME) || { \
		echo "[zappy_gui] tests_run: handshake failed" >&2; \
		exit 1; \
	}
	@echo "[zappy_gui] tests_run: passed"
.PHONY: tests_run

clean:
	@rm -f $(OBJS) $(NAME) $(TEST_BIN) $(PROTOCOL_TEST_BIN) $(ANIMATOR_TEST_BIN) \
		$(SELECTION_TEST_BIN) $(EFFECTS_TEST_BIN) $(FONT_TEST_BIN) \
		$(CAMERA_TEST_BIN) $(PAN_TEST_BIN) $(ZOOM_TEST_BIN)
.PHONY: clean

fclean: clean
	@rm -f ../$(NAME)
.PHONY: fclean

re: fclean all
.PHONY: re
