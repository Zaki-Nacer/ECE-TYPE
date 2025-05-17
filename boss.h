#ifndef BOSS_H
#define BOSS_H

#include "defs.h" // Pour GameState et la structure Boss

// --- Prototypes des Fonctions du Boss ---

// Charge les sprites du boss (phases 1 et 2)
void charger_sprites_boss(GameState *gameState);

// Initialise le boss (avant son apparition)
void initialiser_boss(GameState *gameState);

// Fait apparaître le boss
void spawn_boss(GameState *gameState);

// Met à jour la logique du boss (attaques, phases, etc.)
void mettre_a_jour_boss(GameState *gameState);

// Dessine le boss à l'écran
void dessiner_boss(GameState *gameState);

// Libère les sprites du boss
void nettoyer_ressources_boss(GameState *gameState);

#endif // BOSS_H
