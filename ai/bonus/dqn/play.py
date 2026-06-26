import sys
from pathlib import Path

dqn_dir = Path(__file__).resolve().parent
ai_dir = dqn_dir.parent.parent
sys.path.insert(0, str(ai_dir))
sys.path.insert(0, str(dqn_dir))

from agent import DQNAgent
from config import EXIT_USAGE, ConfigParseError, parse_args
from env import ZappyEnv

USAGE = (
    "USAGE: python3 play.py -p port -n name [-h machine] [--model fichier.pt]\n"
    "\n"
    "Jeu autonome DQN (bonus). -p/-n/-h identiques à zappy_ai.\n"
)


def play_ai(config, model_path=None):
    print(
        f"[*] Connexion autonome pour l'équipe {config.team_name} "
        f"sur {config.hostname}:{config.port}..."
    )
    env = ZappyEnv(config.hostname, config.port, config.team_name)
    agent = DQNAgent(state_dim=env.state_dim, action_dim=env.action_space_n)
    agent.epsilon = 0.0
    if model_path is not None:
        agent.load(model_path)
        print(f"[*] Modèle chargé depuis {model_path}")

    while True:
        try:
            state = env.reset()
            done = False
            while not done:
                action = agent.select_greedy_action(state)
                state, reward, done = env.step(action)
        except ValueError as err:
            print(f"[!] Connexion refusée : {err}")
            return
        except (ConnectionError, OSError) as err:
            print(f"[!] Connexion perdue : {err}")
            return
        except Exception as err:
            print(f"[!] Erreur inattendue : {err}")
            return


def main(argv=None):
    if argv is None:
        argv = sys.argv
    if "--help" in argv or "-help" in argv:
        sys.stdout.write(USAGE)
        return 0

    model_path = None
    cleaned = [argv[0]]
    i = 1
    while i < len(argv):
        if argv[i] == "--model":
            if i + 1 >= len(argv):
                print("missing value for --model", file=sys.stderr)
                return EXIT_USAGE
            model_path = argv[i + 1]
            i += 2
            continue
        cleaned.append(argv[i])
        i += 1

    try:
        config = parse_args(cleaned)
    except ConfigParseError as err:
        sys.stderr.write(f"{err}\n")
        return EXIT_USAGE

    play_ai(config, model_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
