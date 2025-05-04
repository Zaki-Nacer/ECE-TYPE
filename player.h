#ifndef PLAYER_H
#define PLAYER_H

#include "defs.h" // Pour GameState et Vaisseau

// --- Prototypes des Fonctions Joueur ---

// Charge toutes les animations du joueur DANS gameState
void load_player_animations(GameState *gameState);

// Initialise l'état du joueur au début d'un niveau ou au démarrage
void init_player_state(GameState *gameState);

// Définit l'état du joueur et met à jour les sprites/animations correspondants
void set_player_state(GameState *gameState, PlayerState new_state);

// Met à jour la logique du joueur (déplacement, état, animation)
void mettre_a_jour_joueur(GameState *gameState);

// Dessine le joueur sur le buffer de destination (gameState->buffer)
void dessiner_joueur(GameState *gameState);

// Inflige des dégâts au joueur et gère les conséquences (état HIT, GAME OVER)
void damage_player(GameState *gameState, int damage_amount);

// Nettoie les ressources spécifiques au joueur (sprites modèles dans gameState)
void nettoyer_ressources_player(GameState *gameState);


#endif // PLAYER_H
