#ifndef ENEMY_H
#define ENEMY_H

#include "defs.h" // Pour GameState et Ennemi

// --- Prototypes des Fonctions Ennemis ---

// Charge les sprites modèles pour chaque type d'ennemi DANS gameState
void charger_sprites_ennemis(GameState *gameState);

// Initialise le tableau global d'ennemis DANS gameState
void initialiser_ennemis_array(GameState *gameState);

// Cherche un slot inactif et y crée un ennemi DANS gameState
void spawn_ennemi(GameState *gameState, int type, int pos_x_monde, int pos_y_monde);

// Met à jour la position et l'état de tous les ennemis actifs DANS gameState
void mettre_a_jour_ennemis(GameState *gameState);


// Dessine tous les ennemis actifs (depuis gameState) sur le buffer de destination (gameState->buffer)
void dessiner_ennemis(GameState *gameState);

// Nettoie les ressources spécifiques aux ennemis (sprites modèles dans gameState)
void nettoyer_ressources_ennemis(GameState *gameState);

// Définit l'état d'un ennemi spécifique (utilisé en interne et par les collisions)
void set_enemy_state(GameState *gameState, Ennemi *e, EnemyState new_state);


#endif // ENEMY_H
