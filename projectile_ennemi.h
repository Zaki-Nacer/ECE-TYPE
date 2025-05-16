#ifndef PROJECTILE_ENNEMI_H
#define PROJECTILE_ENNEMI_H

#include "defs.h" // Pour GameState, Ennemi, ProjectileEnnemi

// --- Prototypes des Fonctions Projectiles Ennemis ---

// Charge le sprite modèle pour les projectiles ennemis DANS gameState
// (Doit être appelée une fois au démarrage)
void charger_sprite_projectile_ennemi(GameState *gameState);

// Initialise le tableau des projectiles ennemis DANS gameState
// (Doit être appelée au début de chaque niveau)
void initialiser_projectiles_ennemi(GameState *gameState);

// Crée un nouveau projectile ennemi tiré par 'shooter' DANS gameState
// (Appelée depuis mettre_a_jour_ennemis)
void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter);

// Met à jour la position des projectiles ennemis actifs DANS gameState
void mettre_a_jour_projectiles_ennemi(GameState *gameState);

// Dessine les projectiles ennemis actifs (depuis gameState) sur le buffer (gameState->buffer)
void dessiner_projectiles_ennemi(GameState *gameState);

// Nettoie les ressources spécifiques aux projectiles ennemis (sprite modèle dans gameState)
void nettoyer_ressources_projectiles_ennemi(GameState *gameState);


#endif // PROJECTILE_ENNEMI_H
