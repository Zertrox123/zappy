##
## EPITECH PROJECT, 2026
## Zappy/ai/bonus/dqn/Makefile — bonus RL (PyTorch)
##

PYTHON	= python3
BLACK	= black

SOURCES	= agent.py env.py train.py play.py defaults.py

check: $(SOURCES)
	@$(PYTHON) -m py_compile $(SOURCES) 2>/dev/null || { \
		echo "[dqn] check: failed (build error)" >&2; \
		exit 1; \
	}
	@$(PYTHON) train.py --help >/dev/null 2>&1 || { \
		echo "[dqn] check: failed (train --help)" >&2; \
		exit 1; \
	}
	@$(PYTHON) play.py --help >/dev/null 2>&1 || { \
		echo "[dqn] check: failed (play --help)" >&2; \
		exit 1; \
	}
	@echo "[dqn] check: passed"
.PHONY: check

format:
	@$(BLACK) --check . || { \
		echo "[dqn] format: failed" >&2; \
		exit 1; \
	}
	@echo "[dqn] format: passed"
.PHONY: format

format-fix:
	@$(BLACK) .
	@echo "[dqn] format-fix: applied"
.PHONY: format-fix

tests_run:
	@$(PYTHON) -m unittest discover -s tests -p 'test_*.py' -q
	@echo "[dqn] tests_run: passed"
.PHONY: tests_run

clean:
	@rm -rf __pycache__ */__pycache__
.PHONY: clean
