RESOURCE_NAMES = (
    "food",
    "linemate",
    "deraumere",
    "sibur",
    "mendiane",
    "phiras",
    "thystame",
)


def parse_inventory(payload: str) -> dict[str, int]:
    text = payload.strip()
    if not text.startswith("[") or not text.endswith("]"):
        raise ValueError(f"invalid inventory payload: {payload!r}")
    inner = text[1:-1].strip()
    inventory = {name: 0 for name in RESOURCE_NAMES}
    if not inner:
        return inventory
    for chunk in inner.split(","):
        parts = chunk.strip().split()
        if len(parts) != 2:
            raise ValueError(f"invalid inventory item: {chunk!r}")
        name, count = parts
        if name not in inventory:
            raise ValueError(f"unknown resource: {name!r}")
        inventory[name] = int(count)
    return inventory
