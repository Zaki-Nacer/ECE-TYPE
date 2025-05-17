#ifndef PROJECTILE_ENNEMI_H
#define PROJECTILE_ENNEMI_H

#include "defs.h" // Pour GameState, Ennemi et BITMAP

// --- Prototypes des Fonctions ---


void charger_sprites_projectiles_ennemis(GameState *gameState); // Nom modifié pour refléter plusieurs sprites


void initialiser_projectiles_ennemi(GameState *gameState);


void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter, int y_offset, int speed, BITMAP *sprite_to_use, int damage_value); // MODIFIÉ

/**
 * @brief Met à jour la position et l'état des projectiles ennemis.
 * Gère leur déplacement et leur désactivation s'ils sortent de l'écran.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void mettre_a_jour_projectiles_ennemi(GameState *gameState);


void dessiner_projectiles_ennemi(GameState *gameState);


void nettoyer_ressources_projectiles_ennemis(GameState *gameState); // Nom modifié

#endif // PROJECTILE_ENNEMI_H
