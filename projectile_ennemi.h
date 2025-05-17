#ifndef PROJECTILE_ENNEMI_H
#define PROJECTILE_ENNEMI_H

#include "defs.h" // Pour GameState, Ennemi et BITMAP

// --- Prototypes des Fonctions ---

/**
 * @brief Charge et prépare les sprites pour les projectiles ennemis.
 * Les sprites sont stockés dans gameState->sprite_projectile_ennemi1 et gameState->sprite_projectile_ennemi2.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void charger_sprites_projectiles_ennemis(GameState *gameState); // Nom modifié pour refléter plusieurs sprites

/**
 * @brief Initialise le tableau des projectiles ennemis dans gameState.
 * Marque tous les projectiles comme inactifs.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void initialiser_projectiles_ennemi(GameState *gameState);

/**
 * @brief Crée un nouveau projectile ennemi.
 * @param gameState Pointeur vers l'état global du jeu.
 * @param shooter Pointeur vers l'ennemi qui tire.
 * @param y_offset Décalage vertical optionnel pour le projectile.
 * @param speed Vitesse spécifique pour ce projectile.
 * @param sprite_to_use Pointeur vers le BITMAP à utiliser pour ce projectile.
 * @param damage_value Dégâts que ce projectile infligera.
 */
void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter, int y_offset, int speed, BITMAP *sprite_to_use, int damage_value); // MODIFIÉ

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
void nettoyer_ressources_projectiles_ennemis(GameState *gameState); // Nom modifié

#endif // PROJECTILE_ENNEMI_H
