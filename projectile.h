#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "defs.h" // Pour GameState et Projectile

// --- Prototypes des Fonctions Projectiles Joueur ---

// Charge et redimensionne le sprite modèle pour les projectiles joueur DANS gameState
void charger_et_redimensionner_sprite_projectile_joueur(GameState *gameState);

// Initialise le tableau des projectiles joueur DANS gameState
void initialiser_projectiles_joueur(GameState *gameState);

// Crée un nouveau projectile joueur DANS gameState
void spawn_projectile_joueur(GameState *gameState, int start_x, int start_y);

// Met à jour la position et l'état des projectiles joueur DANS gameState
void mettre_a_jour_projectiles_joueur(GameState *gameState);

// Dessine les projectiles joueur actifs (depuis gameState) sur le buffer (gameState->buffer)
void dessiner_projectiles_joueur(GameState *gameState);

// Nettoie les ressources projectiles joueur (sprite modèle dans gameState)
void nettoyer_ressources_projectiles_joueur(GameState *gameState);

// --- Prototypes pour Projectiles Ennemis (A AJOUTER PLUS TARD) ---

#endif // PROJECTILE_H
