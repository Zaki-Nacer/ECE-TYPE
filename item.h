#ifndef ITEM_H
#define ITEM_H

#include "defs.h" // Pour GameState et ItemType

// --- Prototypes des Fonctions ---

/**
 * @brief Charge les sprites pour les items (si nécessaire).
 * Pour l'instant, cette fonction peut être vide si on utilise des rectangles.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void charger_sprites_items(GameState *gameState);

/**
 * @brief Initialise le tableau des items dans gameState.
 * Marque tous les items comme inactifs.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void initialiser_items(GameState *gameState);

/**
 * @brief Fait apparaître un item d'un type donné à une position donnée dans le monde.
 * @param gameState Pointeur vers l'état global du jeu.
 * @param type Type de l'item à faire apparaître (ex: ITEM_TYPE_SCREEN_CLEAR).
 * @param x_world Coordonnée X dans le monde où l'item doit apparaître.
 * @param y_world Coordonnée Y dans le monde où l'item doit apparaître.
 */
void spawn_item(GameState *gameState, ItemType type, int x_world, int y_world);

/**
 * @brief Met à jour la position et l'état des items.
 * Les items ne bougent pas d'eux-mêmes par rapport au monde, ils défilent avec l'écran.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void mettre_a_jour_items(GameState *gameState);

/**
 * @brief Dessine les items actifs sur le buffer de jeu.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void dessiner_items(GameState *gameState);

/**
 * @brief Libère les ressources allouées pour les sprites des items.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void nettoyer_ressources_items(GameState *gameState);

#endif // ITEM_H
