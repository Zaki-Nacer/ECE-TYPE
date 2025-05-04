#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h" // Pour GameState et Vaisseau

// --- Prototypes des Fonctions Joueur ---

// Charge toutes les animations du joueur, les redimensionne, traite transparence et initialise la structure DANS gameState
void load_player_animations(GameState *gameState);

// Initialise l'état du joueur au début d'un niveau ou après une mort
void init_player_state(GameState *gameState);

// Met à jour la position, l'état et l'animation du joueur DANS gameState
void mettre_a_jour_joueur(GameState *gameState);

// Dessine le joueur (depuis gameState) sur le buffer de destination (gameState->buffer)
void dessiner_joueur(GameState *gameState);

// Nettoie les ressources spécifiques au joueur (tous les sprites dans gameState)
void nettoyer_ressources_joueur(GameState *gameState);

// Fonction pour changer l'état du joueur (et son animation)
void set_player_state(GameState *gameState, PlayerState new_state);

// Fonction pour infliger des dégâts au joueur
void damage_player(GameState *gameState, int amount);

#endif // PLAYER_H
