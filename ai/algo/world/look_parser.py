from dataclasses import dataclass


@dataclass(frozen=True)
class TileView:
    index: int
    objects: tuple[str, ...]


def parse_look(payload: str) -> list[TileView]:
    text = payload.strip()
    if not text.startswith("[") or not text.endswith("]"):
        raise ValueError(f"invalid look payload: {payload!r}")
    inner = text[1:-1].strip()
    if not inner:
        return []
    tiles: list[TileView] = []
    for index, chunk in enumerate(inner.split(",")):
        objects = tuple(obj.strip() for obj in chunk.split() if obj.strip())
        tiles.append(TileView(index, objects))
    return tiles
