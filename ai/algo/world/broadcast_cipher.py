def _xor(data: bytes, key: bytes) -> bytes:
    if not key:
        return data
    return bytes(b ^ key[i % len(key)] for i, b in enumerate(data))


def encrypt_broadcast(plaintext: str, key: str) -> str:
    return _xor(plaintext.encode("utf-8"), key.encode("utf-8")).hex()


def decrypt_broadcast(payload: str, key: str) -> str | None:
    if not payload or len(payload) % 2 != 0:
        return None
    try:
        data = bytes.fromhex(payload)
        return _xor(data, key.encode("utf-8")).decode("utf-8")
    except (ValueError, UnicodeDecodeError):
        return None
