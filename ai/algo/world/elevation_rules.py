"""Stone and player requirements for each elevation level."""

ELEVATION_REQUIREMENTS: dict[int, tuple[int, dict[str, int]]] = {
    2: (1, {"linemate": 1}),
    3: (2, {"linemate": 1, "deraumere": 1, "sibur": 1}),
    4: (2, {"linemate": 2, "sibur": 1, "phiras": 2}),
    5: (4, {"linemate": 1, "deraumere": 1, "sibur": 2, "phiras": 1}),
    6: (4, {"linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3}),
    7: (6, {"linemate": 1, "deraumere": 2, "sibur": 3, "phiras": 1}),
    8: (
        6,
        {
            "linemate": 2,
            "deraumere": 2,
            "sibur": 2,
            "mendiane": 2,
            "phiras": 2,
            "thystame": 1,
        },
    ),
}

STONE_NAMES = [
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame",
]
