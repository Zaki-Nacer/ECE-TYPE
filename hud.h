#ifndef HUD_H
#define HUD_H

#include "defs.h" // Pour GameState

// --- Prototypes des Fonctions HUD ---

// Charge les ressources HUD DANS gameState
void charger_ressources_hud(GameState *gameState);

// Met à jour l'état du HUD DANS gameState
void mettre_a_jour_hud(GameState *gameState);

// Dessine une barre de progression (utilise les sprites de gameState)
void dessiner_barre_progression(GameState *gameState, BITMAP *dest, int x, int y, int value, int max_value);

// Dessine l'ensemble du HUD (depuis gameState) sur le buffer (gameState->buffer)
void dessiner_hud(GameState *gameState);

// Nettoie les ressources HUD (depuis gameState)
void nettoyer_ressources_hud(GameState *gameState);

#endif // HUD_H
