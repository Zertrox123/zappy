# Zappy

Jeu multijoueur distribué en réseau : Serveur TCP en C, client IA autonome en Python/C++ et visualiseur graphique.

## Présentation

Projet de synthèse de 2ème année (G-YEP-400) simulant un monde virtuel temps réel où des équipes d'agents IA s'affrontent pour collecter des ressources (nourriture, minéraux) et accomplir des rituels d'élévation.

Composants :
- **Serveur réseau (C)** : Serveur TCP asynchrone non-bloquant avec `select`, cadence de jeu configurable par unité de temps (`freq`).
- **Client IA** : Agent autonome explorant la grille, communiquant par broadcast audio et se coordonnant avec ses pairs.
- **Interface graphique** : Rendu visuel 2D/3D temps réel du monde et des actions.

## Prérequis

- GCC / G++
- Python 3.10+
- Make

## Compilation et Lancement

```bash
# Compiler l'ensemble des modules
make

# 1. Lancer le serveur
./zappy_server -p 4242 -x 20 -y 20 -n Equipe1 Equipe2 -c 6 -t 100

# 2. Lancer un drone IA
./zappy_ai -p 4242 -n Equipe1 -h 127.0.0.1
```
