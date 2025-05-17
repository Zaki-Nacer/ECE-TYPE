#include "item.h"
#include "defs.h"
#include "graphics.h"
#include <stdio.h>

// --- Implémentation des Fonctions ---

void charger_sprites_items(GameState *gameState) {
    if (!gameState) return;
    printf("Chargement des sprites d'items...\n"); fflush(stdout);

    gameState->sprite_item_screen_clear = NULL;
    gameState->sprite_item_health_pack = NULL; // NOUVEAU

    // il y'aura des sprites mais pas pour l'instant il faudra decommenter ça : (même logique que le reste)
    /*
    gameState->sprite_item_screen_clear = charger_bitmap_safe(ITEM_SCREEN_CLEAR_SPRITE_FILENAME);
    if (gameState->sprite_item_screen_clear) {
        make_white_transparent(gameState, gameState->sprite_item_screen_clear);
    } else {
        printf("ERREUR: Impossible de charger %s.\n", ITEM_SCREEN_CLEAR_SPRITE_FILENAME);
    }

    gameState->sprite_item_health_pack = charger_bitmap_safe(ITEM_HEALTH_PACK_SPRITE_FILENAME);
    if (gameState->sprite_item_health_pack) {
        make_white_transparent(gameState, gameState->sprite_item_health_pack);
    } else {
        printf("ERREUR: Impossible de charger %s.\n", ITEM_HEALTH_PACK_SPRITE_FILENAME);
    }
    */
   printf("Aucun sprite d'item chargé (utilisation de rectangles pour l'instant).\n"); fflush(stdout);
}

void initialiser_items(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_ITEMS; i++) {
        gameState->items[i].active = 0;
        gameState->items[i].type = ITEM_TYPE_NONE;
        gameState->items[i].sprite = NULL;
    }
}

void spawn_item(GameState *gameState, ItemType type, int x_world, int y_world) {
    if (!gameState) return;

    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!gameState->items[i].active) {
            Item *item = &gameState->items[i];
            item->active = 1;
            item->type = type;
            item->x_world = x_world;
            item->y_world = y_world;
            item->w = ITEM_SIZE;
            item->h = ITEM_SIZE;
            item->sprite = NULL;

            if (type == ITEM_TYPE_SCREEN_CLEAR && gameState->sprite_item_screen_clear) {
                item->sprite = gameState->sprite_item_screen_clear;
            } else if (type == ITEM_TYPE_HEALTH_PACK && gameState->sprite_item_health_pack) { // NOUVEAU
                item->sprite = gameState->sprite_item_health_pack;
            }

            if(item->sprite){
                item->w = item->sprite->w;
                item->h = item->sprite->h;
            }

            printf("Item de type %d apparu en (%d, %d) monde.\n", type, x_world, y_world); fflush(stdout);
            return;
        }
    }
    printf("Attention: Impossible de faire apparaître l'item, tableau plein !\n"); fflush(stdout);
}

void mettre_a_jour_items(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (gameState->items[i].active) {
            int screen_x = gameState->items[i].x_world - gameState->scroll_x;
            if (screen_x + gameState->items[i].w < 0) {
                // gameState->items[i].active = 0; // Optionnel: faire disparaître
            }
        }
    }
}

void dessiner_items(GameState *gameState) {
    if (!gameState || !gameState->buffer) return;
    BITMAP *dest = gameState->buffer;

    for (int i = 0; i < MAX_ITEMS; i++) {
        Item *item = &gameState->items[i];
        if (item->active) {
            int screen_x = item->x_world - gameState->scroll_x;
            int screen_y = item->y_world - gameState->scroll_y_dummy;

            if (screen_x + item->w > 0 && screen_x < gameState->screen_width_allegro &&
                screen_y + item->h > 0 && screen_y < gameState->screen_height_allegro) {

                if (item->sprite) {
                    draw_sprite(dest, item->sprite, screen_x, screen_y);
                } else {
                    int color = makecol(255,255,255);
                    if (item->type == ITEM_TYPE_SCREEN_CLEAR) {
                        color = makecol(0, 255, 255); // Cyan
                    } else if (item->type == ITEM_TYPE_HEALTH_PACK) { // NOUVEAU
                        color = makecol(255, 0, 0);   // Rouge pour le pack de santé
                    }
                    rectfill(dest, screen_x, screen_y, screen_x + item->w -1, screen_y + item->h -1, color);
                }
            }
        }
    }
}

void nettoyer_ressources_items(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage des ressources d'items...\n"); fflush(stdout);
    if (gameState->sprite_item_screen_clear) {
        destroy_bitmap(gameState->sprite_item_screen_clear);
        gameState->sprite_item_screen_clear = NULL;
    }
    if (gameState->sprite_item_health_pack) { // NOUVEAU
        destroy_bitmap(gameState->sprite_item_health_pack);
        gameState->sprite_item_health_pack = NULL;
    }
    printf("Sprites d'items détruits.\n"); fflush(stdout);
}
