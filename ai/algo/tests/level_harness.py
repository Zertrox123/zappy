import os
import re
import socket
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path

AI_DIR = Path(__file__).resolve().parents[2]
ROOT = AI_DIR.parent

LEVEL_RE = re.compile(r"level=(\d+)\s+alive=True")


@dataclass
class HarnessConfig:
    port: int = 4242
    host: str = "127.0.0.1"
    team: str = "team"
    width: int = 10
    height: int = 10
    clients: int = 6
    freq: int = 50
    target_level: int = 4
    timeout_sec: int = 300
    leader_headstart_sec: int = 120
    client_count: int = 2


@dataclass
class HarnessResult:
    max_level: int
    passed: bool
    logs: dict[str, Path]
    server_cmd: list[str]


def resolve_server() -> Path | None:
    env_path = os.environ.get("ZAPPY_SERVER")
    if env_path:
        candidate = Path(env_path)
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return candidate
    for candidate in (
        ROOT / "zappy_server",
        ROOT / "server" / "target" / "release" / "zappy_server",
        ROOT / "tools" / "reference-server" / "zappy_server",
    ):
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return candidate
    return None


def resolve_python() -> str:
    venv = ROOT / ".venv" / "bin" / "python3"
    if venv.is_file():
        return str(venv)
    return sys.executable


def resolve_ai_entry() -> list[str]:
    zappy_ai = ROOT / "zappy_ai"
    if zappy_ai.is_file() and os.access(zappy_ai, os.X_OK):
        return [str(zappy_ai)]
    return [resolve_python(), str(AI_DIR / "main.py")]


def wait_for_port(host: str, port: int, timeout_sec: float = 10.0) -> bool:
    deadline = time.monotonic() + timeout_sec
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.3):
                return True
        except OSError:
            time.sleep(0.2)
    return False


def parse_max_level_from_log(log_path: Path) -> int:
    max_level = 1
    if not log_path.is_file():
        return max_level
    text = log_path.read_text(errors="replace")
    for match in LEVEL_RE.finditer(text):
        level = int(match.group(1))
        if level > max_level:
            max_level = level
    return max_level


def kill_process(proc: subprocess.Popen | None) -> None:
    if proc is None:
        return
    if proc.poll() is not None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=3)


def free_port(port: int) -> None:
    try:
        result = subprocess.run(
            ["lsof", "-ti", f":{port}"],
            capture_output=True,
            text=True,
            check=False,
        )
    except OSError:
        return
    pids = [part.strip() for part in result.stdout.split() if part.strip()]
    for pid in pids:
        try:
            os.kill(int(pid), 9)
        except OSError:
            pass


def build_server_cmd(server: Path, config: HarnessConfig) -> list[str]:
    return [
        str(server),
        "-p",
        str(config.port),
        "-x",
        str(config.width),
        "-y",
        str(config.height),
        "-n",
        config.team,
        "-c",
        str(config.clients),
        "-f",
        str(config.freq),
    ]


def run_level_harness(config: HarnessConfig | None = None) -> HarnessResult:
    if config is None:
        config = HarnessConfig()

    server = resolve_server()
    if server is None:
        raise FileNotFoundError("no executable zappy_server found")

    free_port(config.port)

    log_dir = Path(tempfile.mkdtemp(prefix="zappy-level-test-"))
    logs: dict[str, Path] = {}
    ai_cmd = resolve_ai_entry()
    server_cmd = build_server_cmd(server, config)

    server_proc = subprocess.Popen(
        [
            "sh",
            "-c",
            f"(printf 'start\\n'; exec tail -f /dev/null) | {server_cmd[0]} {' '.join(server_cmd[1:])}",
        ],
        cwd=ROOT,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    if not wait_for_port(config.host, config.port):
        kill_process(server_proc)
        raise RuntimeError(f"server did not listen on {config.host}:{config.port}")

    time.sleep(0.5)

    client_procs: list[subprocess.Popen] = []
    roles = ["leader"]
    for index in range(1, config.client_count):
        roles.append("follower")

    try:
        for index, role in enumerate(roles):
            if index == 1 and config.leader_headstart_sec > 0:
                time.sleep(config.leader_headstart_sec)

            log_path = log_dir / f"{role}_{index}.log"
            logs[role if index == 0 else f"{role}_{index}"] = log_path
            env = os.environ.copy()
            env["ZAPPY_ROLE"] = role
            env["PYTHONUNBUFFERED"] = "1"
            log_file = open(log_path, "w", encoding="utf-8")
            proc = subprocess.Popen(
                ai_cmd
                + [
                    "-p",
                    str(config.port),
                    "-n",
                    config.team,
                    "-h",
                    config.host,
                ],
                cwd=ROOT,
                env=env,
                stdout=log_file,
                stderr=subprocess.STDOUT,
            )
            client_procs.append(proc)

        deadline = time.monotonic() + config.timeout_sec
        max_level = 1
        passed = False

        while time.monotonic() < deadline:
            alive = False
            for proc in client_procs:
                if proc.poll() is None:
                    alive = True
            for log_path in logs.values():
                level = parse_max_level_from_log(log_path)
                if level > max_level:
                    max_level = level
                if level >= config.target_level:
                    passed = True
                    break
            if passed:
                break
            if not alive:
                break
            time.sleep(2)

        if not passed:
            for log_path in logs.values():
                level = parse_max_level_from_log(log_path)
                if level > max_level:
                    max_level = level

        return HarnessResult(
            max_level=max_level,
            passed=passed,
            logs=logs,
            server_cmd=server_cmd,
        )
    finally:
        for proc in client_procs:
            kill_process(proc)
        kill_process(server_proc)
        free_port(config.port)
