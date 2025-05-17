#ifndef ITEM_H
#define ITEM_H

#include "defs.h" // Pour GameState et ItemType

// --- Prototypes des Fonctions ---

void charger_sprites_items(GameState *gameState);


void initialiser_items(GameState *gameState);


void spawn_item(GameState *gameState, ItemType type, int x_world, int y_world);


void mettre_a_jour_items(GameState *gameState);

void dessiner_items(GameState *gameState);

yer_ressources_items(GameState *gameState);

#endif // ITEM_H
