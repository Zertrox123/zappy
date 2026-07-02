"""Decoy broadcasts: plaintext chatter crafted to mislead rival teams.

Rival AIs commonly coordinate with loose plaintext commands (REGROUP/READY/
COME/INC...) and parse whatever arrives on the shared channel. Sending
plausible fakes makes weakly-filtered enemies chase phantom leaders, walk
toward our camped bots, or mistime their ceremonies. Our own bots are immune:
they only trust messages carrying our cipher tag, so every decoy is free
disinformation. A decoy costs the same 7/f time units as the idle Look it
replaces, so waiting bots jam the channel at zero cost to our own tempo.
"""

import random

_LEVELS = (2, 3, 4, 5, 6, 7)

_TEMPLATES = (
    "REGROUP_L{lvl}",
    "REGROUP_L{lvl}_{tag}",
    "REGROUP L{lvl}",
    "READY_L{lvl}",
    "READY_L{lvl}_{tag}",
    "READY L{lvl}",
    "INC_{lvl}",
    "INC L{lvl}",
    "COME_{lvl}",
    "COME",
    "GO",
    "STOP",
    "WAIT",
    "HELP_{lvl}",
    "FOOD_{x}_{y}",
    "MEET_{x}_{y}",
    "LVL{lvl}_OK",
    "{tag}",
)


def craft_decoy_message(rng: random.Random | None = None) -> str:
    rng = rng or random
    template = rng.choice(_TEMPLATES)
    return template.format(
        lvl=rng.choice(_LEVELS),
        x=rng.randint(0, 20),
        y=rng.randint(0, 20),
        tag="".join(rng.choice("0123456789abcdef") for _ in range(8)),
    )
