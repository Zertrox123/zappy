"""XOR cipher for team broadcast payloads."""

_HEX_CHARS = frozenset("0123456789abcdefABCDEF")


def _xor(data: bytes, key: bytes) -> bytes:
    if not key:
        return data
    return bytes(b ^ key[i % len(key)] for i, b in enumerate(data))


def encrypt_broadcast(plaintext: str, key: str) -> str:
    return _xor(plaintext.encode("utf-8"), key.encode("utf-8")).hex()


def decrypt_broadcast(payload: str, key: str) -> str | None:
    if not payload or len(payload) % 2 != 0:
        return None
    if not all(c in _HEX_CHARS for c in payload):
        return None
    try:
        data = bytes.fromhex(payload)
    except ValueError:
        return None
    try:
        return _xor(data, key.encode("utf-8")).decode("utf-8")
    except UnicodeDecodeError:
        return None
