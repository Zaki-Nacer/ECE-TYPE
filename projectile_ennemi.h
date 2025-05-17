#ifndef PROJECTILE_ENNEMI_H
#define PROJECTILE_ENNEMI_H

#include "defs.h" // Pour GameState et Ennemi

// --- Prototypes des Fonctions ---

/**
 * @brief Charge et prépare le sprite pour les projectiles ennemis.
 * Le sprite est stocké dans gameState->sprite_projectile_ennemi.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void charger_sprite_projectile_ennemi(GameState *gameState);

/**
 * @brief Initialise le tableau des projectiles ennemis dans gameState.
 * Marque tous les projectiles comme inactifs.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void initialiser_projectiles_ennemi(GameState *gameState);

/**
 * @brief Crée un nouveau projectile ennemi.
 * Le projectile est tiré par l'ennemi 'shooter'.
 * @param gameState Pointeur vers l'état global du jeu.
 * @param shooter Pointeur vers l'ennemi qui tire.
 * @param y_offset Décalage vertical optionnel pour le projectile (utilisé pour les tirs en éventail).
 */
void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter, int y_offset, int speed);

/**
 * @brief Met à jour la position et l'état des projectiles ennemis.
 * Gère leur déplacement et leur désactivation s'ils sortent de l'écran.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void mettre_a_jour_projectiles_ennemi(GameState *gameState);

/**
 * @brief Dessine les projectiles ennemis actifs sur le buffer de jeu.
 * Les projectiles sont dessinés avec une inversion horizontale.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void dessiner_projectiles_ennemi(GameState *gameState);

/**
 * @brief Libère les ressources allouées pour les sprites des projectiles ennemis.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void nettoyer_ressources_projectiles_ennemi(GameState *gameState);

#endif // PROJECTILE_ENNEMI_H
