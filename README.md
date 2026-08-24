# Zappy

Jeu multijoueur en réseau où des IA s'affrontent sur une carte pour farmer des ressources et monter de niveau.

Le projet contient :
- Un serveur TCP en C (gestion non-bloquante avec `select`)
- Un client IA en Python/C++ qui communique par broadcast audio pour se regrouper et faire des rituels
- Un visualiseur graphique pour voir la partie en direct

## Build et lancement

```bash
make

# Lancer le serveur
./zappy_server -p 4242 -x 20 -y 20 -n Team1 Team2 -c 6 -t 100

# Lancer une IA
./zappy_ai -p 4242 -n Team1 -h 127.0.0.1
```
