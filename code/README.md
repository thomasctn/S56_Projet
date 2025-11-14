# Projet Pac-Man Multijoueur (Client/Serveur)

## Description du projet
Ce projet consiste à réaliser un Pac-Man multijoueur en C++ basé sur une architecture **client/serveur** :

- Le **serveur** gère toute la logique du jeu (déplacements, collisions, pac-gommes, victoire…).
- Le **client** s’occupe uniquement de l’affichage et des entrées utilisateur.
- Le tout utilise le framework **Gamedev (gf)** pour le réseau et le rendu 2D.

L’objectif final est d’obtenir un Pac-Man jouable en réseau local, avec un plateau composé de cases et des joueurs contrôlant Pac-Man ou fantôme.

---

# Architecture du projet

code(src)/
│
├── common/
│ ├── Protocol.hpp
│ ├── Serializable.hpp
│ ├── Types.hpp
│
├── server/
│ ├── Game.hpp / .cpp
│ ├── Player.hpp / .cpp
│ ├── Plateau.hpp / .cpp
│ ├── Case.hpp / .cpp
│ ├── PacGomme.hpp / .cpp
│ ├── GameNetworkServer.hpp / .cpp
│
└── client/
├── GameClient.hpp / .cpp
├── Renderer.hpp / .cpp
├── ClientPlayer.hpp / .cpp
├── ClientNetwork.hpp / .cpp
├── InputHandler.hpp / .cpp


---

# Objectifs par dossier

## common/ — Types et protocoles partagés

Contient tous les éléments nécessaires pour que le client et le serveur utilisent le même protocole de communication.

### À implémenter :
- **Protocol.hpp**  
  - Types de messages réseau (PlayerMove, GameState, PlayerJoin…)
  - Identifiants d’entités
- **Serializable.hpp**  
  - Structures sérialisables envoyées sur le réseau (PlayerState, GameState…)
- **Types.hpp**  
  - Position, Direction, Role, types d'entités…

---

## 2️⃣ server/ — Logique du jeu + réseau serveur

Le serveur est **autoritaire** : il contrôle l’état réel du jeu.

### À implémenter :
- **Game**  
  - Gère la boucle de jeu
  - Met à jour toutes les entités
  - Gère collisions et victoires
  - Applique les commandes du client  
- **Player**  
  - id, nom, rôle, position
  - logique de déplacement côté serveur  
- **Plateau**  
  - Grille de la map
  - Chargement (ou génération procédurale a terme)
- **Case**  
  - Mur, sol, maison
  - Détermine la validité des déplacements  
- **PacGomme**  
  - Position + état "mangée"  
- **GameNetworkServer**  
  - Gestion des connexions
  - Réception des inputs
  - Envoi régulier du GameState


---

## client/ — Affichage + entrée utilisateur

Le client **ne prend aucune décision de gameplay**.  
Il affiche ce que dit le serveur.

### À implémenter :
- **GameClient**  
  - Boucle d'affichage
  - Réception du GameState
  - Envoi des inputs  
- **Renderer**  
  - Affichage du plateau
  - Affichage des joueurs et pac-gommes  
- **ClientPlayer**  
  - Version simplifiée du joueur pour l’affichage  
- **ClientNetwork**  
  - Envoi des inputs
  - Connexion + réception des données du serveur  
- **InputHandler**  
  - Gestion clavier (direction du joueur)

---

# Fonctionnement réseau

## Client ➜ Serveur
- Entrées du joueur (direction)
- Demande de connexion
- Déconnexion
- Activation pouvoir (plus tard)

## Serveur ➜ Client
Envoi régulier du **GameState** :
- Positions de tous les joueurs
- Etat des pac-gommes
- Map
- L'Etat du jeu

---


# Répartition de l’équipe

### Thomas — Serveur
Game, Player, Plateau, PacGomme, collisions.

### Benoit — Réseau
Protocol, Serializable, GameNetworkServer, ClientNetwork.

### Aurianne — Client
Renderer, GameClient, InputHandler, ClientPlayer.

---

