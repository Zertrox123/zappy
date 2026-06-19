# Server
- comment je distribue les resources sur la map
  > **Réponse :** La distribution est gérée dans [data.rs](file:///Users/karma/code/epitech/2_year/G-YEP-400-LYN-4-1-zappy-7/server/game/src/data.rs).
  > 1. **Densité par Ressource (`Resource::get_density`)** : Chaque ressource possède une densité cible par rapport à la surface de la map (`width * height`) :
  >    - Nourriture (`Food`) : 50% (`0.50`)
  >    - Linemate : 30% (`0.30`)
  >    - Deraumere : 15% (`0.15`)
  >    - Sibur / Mendiane : 10% (`0.10`)
  >    - Phiras : 8% (`0.08`)
  >    - Thystame : 5% (`0.05`)
  > 2. **Refill et Comptage (`Map::refill`)** : Le serveur compte les ressources existantes. Si `actuel < max_cible` (où `max_cible = surface * densite`), il génère la différence (`max - actuel`).
  > 3. **Spawning Aléatoire (`Map::spawn`)** : Des coordonnées `(x, y)` sont tirées au hasard sur la grille, et la ressource est insérée dans la case correspondante (`self.tiles[y][x].stone.push(resource)`).

# Gui
# Ai
- 
