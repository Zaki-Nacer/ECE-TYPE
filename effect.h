#ifndef EFFECT_H
#define EFFECT_H

#include "defs.h" // Pour GameState et Effect

// --- Prototypes des Fonctions Effets ---

// Charge les sprites modèles pour les effets DANS gameState
void charger_sprites_effects(GameState *gameState);

// Initialise le tableau des effets DANS gameState
void initialiser_effects(GameState *gameState);

// Crée un nouvel effet d'impact DANS gameState
void spawn_impact_effect(GameState *gameState, int x, int y);

// Met à jour l'animation et la durée de vie des effets DANS gameState
void mettre_a_jour_effects(GameState *gameState);

// Dessine les effets actifs (depuis gameState) sur le buffer (gameState->buffer)
void dessiner_effects(GameState *gameState);

// Nettoie les ressources effets (sprites modèles dans gameState)
void nettoyer_ressources_effects(GameState *gameState);

#endif // EFFECT_H
