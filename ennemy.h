#ifndef EFFECT_H
#define EFFECT_H

#include "defs.h" // Pour GameState et Effect ça va être le fichier avec toute les constante et structure de donnée 

// --- Prototypes des Fonctions Effets ---

// Charge les sprites modèles pour les effets DANS gameState
void charger_sprites_effects(GameState gameState);

// Initialise le tableau des effets DANS gameState
void initialiser_effects(GameStategameState);

// Crée un nouvel effet d'impact DANS gameState
void spawn_impact_effect(GameState gameState, int x, int y);

// Met à jour l'animation et la durée de vie des effets DANS gameState
void mettre_a_jour_effects(GameStategameState);

// Dessine les effets actifs (depuis gameState) sur le buffer (gameState->buffer)
void dessiner_effects(GameState gameState);

// Nettoie les ressources effets (sprites modèles dans gameState)
void nettoyer_ressources_effects(GameStategameState);

#endif // EFFECT_H
