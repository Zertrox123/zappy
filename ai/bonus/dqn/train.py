import sys
from pathlib import Path

dqn_dir = Path(__file__).resolve().parent
ai_dir = dqn_dir.parent.parent
sys.path.insert(0, str(ai_dir))
sys.path.insert(0, str(dqn_dir))

from agent import DQNAgent
from config import EXIT_USAGE, ConfigParseError, parse_args
from defaults import DEFAULT_EPISODES
from env import ZappyEnv

USAGE = (
    "USAGE: python3 train.py -p port -n name [-h machine] [--episodes N]\n"
    "\n"
    "Entraînement DQN (bonus). -p/-n/-h identiques à zappy_ai.\n"
)


def train_ai(config, episodes):
    print(
        f"[*] Initialisation de l'IA pour l'équipe {config.team_name} "
        f"sur {config.hostname}:{config.port}..."
    )
    env = ZappyEnv(config.hostname, config.port, config.team_name)
    agent = DQNAgent(state_dim=env.state_dim, action_dim=env.action_space_n)
    for episode in range(episodes):
        try:
            state = env.reset()
            score = 0
            done = False
            while not done:
                action = agent.select_action(state)
                next_state, reward, done = env.step(action)
                agent.remember(state, action, reward, next_state, done)
                agent.replay()
                state = next_state
                score += reward
            print(
                f"Épisode {episode + 1}/{episodes} | "
                f"Score: {score} | Epsilon: {agent.epsilon:.2f}"
            )
        except ConnectionError as err:
            print(f"[!] Erreur réseau fatale : {err}")
            break
        except OSError as err:
            print(f"[!] Erreur réseau fatale : {err}")
            break
        except Exception as err:
            print(f"[!] Le Trantorien a subi une erreur inattendue : {err}")


def main(argv=None):
    if argv is None:
        argv = sys.argv
    if "--help" in argv or "-help" in argv:
        sys.stdout.write(USAGE)
        return 0

    episodes = DEFAULT_EPISODES
    cleaned = [argv[0]]
    i = 1
    while i < len(argv):
        if argv[i] == "--episodes":
            if i + 1 >= len(argv):
                print("missing value for --episodes", file=sys.stderr)
                return EXIT_USAGE
            try:
                episodes = int(argv[i + 1])
            except ValueError:
                print(f"invalid value for --episodes: {argv[i + 1]}", file=sys.stderr)
                return EXIT_USAGE
            if episodes <= 0:
                print(f"invalid value for --episodes: {argv[i + 1]}", file=sys.stderr)
                return EXIT_USAGE
            i += 2
            continue
        cleaned.append(argv[i])
        i += 1

    try:
        config = parse_args(cleaned)
    except ConfigParseError as err:
        sys.stderr.write(f"{err}\n")
        return EXIT_USAGE

    train_ai(config, episodes)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
