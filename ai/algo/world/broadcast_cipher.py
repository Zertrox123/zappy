"""Authenticated team broadcasts for a hostile, shared channel.

Tournament threat model: every team hears every broadcast, team names are
public, and rival AIs may spam lookalike commands or replay our own messages.
Messages are XOR-encrypted with a keystream derived from the team name plus a
secret pepper (so knowing the team name is not enough to forge), carry a magic
tag (so garbage or foreign ciphertext never decodes into a valid-looking
command), and carry a per-sender sequence number (so replayed messages are
detected and dropped by the receiver).

Wire format: hex( keystream XOR f"{MAGIC}|{sender}|{seq}|{payload}" ).
"""

import hashlib

_PEPPER = "zappy7-lyn41-pepper"
_MAGIC = "Z7"


def _keystream(key: str, length: int) -> bytes:
    seed = hashlib.sha256((_PEPPER + key + _PEPPER).encode("utf-8")).digest()
    out = b""
    block = seed
    while len(out) < length:
        block = hashlib.sha256(block + seed).digest()
        out += block
    return out[:length]


def _xor(data: bytes, key: str) -> bytes:
    stream = _keystream(key, len(data))
    return bytes(b ^ stream[i] for i, b in enumerate(data))


def encrypt_broadcast(
    plaintext: str, key: str, sender: str = "anon", seq: int = 0
) -> str:
    envelope = f"{_MAGIC}|{sender}|{seq}|{plaintext}"
    return _xor(envelope.encode("utf-8"), key).hex()


def decrypt_broadcast(payload: str, key: str) -> tuple[str, int, str] | None:
    """Return (sender, seq, plaintext) if the message is authentic, else None."""
    if not payload or len(payload) % 2 != 0:
        return None
    try:
        data = bytes.fromhex(payload)
        envelope = _xor(data, key).decode("utf-8")
    except (ValueError, UnicodeDecodeError):
        return None
    parts = envelope.split("|", 3)
    if len(parts) != 4 or parts[0] != _MAGIC:
        return None
    try:
        seq = int(parts[2])
    except ValueError:
        return None
    return parts[1], seq, parts[3]
