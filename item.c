#include "item.h"
#include "defs.h"
#include "graphics.h" // Pour charger_bitmap_safe (si utilisé plus tard)
#include <stdio.h>

// --- Implémentation des Fonctions ---

void charger_sprites_items(GameState *gameState) {
    if (!gameState) return;
    printf("Chargement des sprites d'items...\n"); fflush(stdout);
    // Pour l'instant, nous n'allons pas charger de sprite fichier pour l'item.
    // Si vous créez un "item_screen_clear.bmp", décommentez et adaptez :
    /*
    gameState->sprite_item_screen_clear = charger_bitmap_safe(ITEM_SCREEN_CLEAR_SPRITE_FILENAME);
    if (gameState->sprite_item_screen_clear) {
        make_white_transparent(gameState, gameState->sprite_item_screen_clear);
        printf("Sprite pour ITEM_SCREEN_CLEAR chargé.\n"); fflush(stdout);
    } else {
        printf("ERREUR: Impossible de charger le sprite pour ITEM_SCREEN_CLEAR.\n"); fflush(stdout);
    }
    */
   gameState->sprite_item_screen_clear = NULL; // Indiquer qu'aucun sprite n'est chargé
   printf("Aucun sprite d'item chargé (utilisation de rectangles pour l'instant).\n"); fflush(stdout);
}

void initialiser_items(GameState *gameState) {
    if (!gameState) return;
    // printf("Initialisation du tableau d'items...\n"); fflush(stdout); // Optionnel
    for (int i = 0; i < MAX_ITEMS; i++) {
        gameState->items[i].active = 0;
        gameState->items[i].type = ITEM_TYPE_NONE;
        gameState->items[i].sprite = NULL; // Aucun sprite assigné par défaut
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

            // Assigner un sprite si disponible (pour l'instant, non)
            if (type == ITEM_TYPE_SCREEN_CLEAR && gameState->sprite_item_screen_clear) {
                item->sprite = gameState->sprite_item_screen_clear;
                item->w = item->sprite->w; // Utiliser la taille du sprite si chargé
                item->h = item->sprite->h;
            } else {
                item->sprite = NULL; // Pas de sprite, sera dessiné comme un rectangle
            }

            printf("Item de type %d apparu en (%d, %d) monde.\n", type, x_world, y_world); fflush(stdout);
            return;
        }
    }
    printf("Attention: Impossible de faire apparaître l'item, tableau plein !\n"); fflush(stdout);
}

void mettre_a_jour_items(GameState *gameState) {
    if (!gameState) return;
    // Les items sont statiques par rapport au monde, donc leur x_world, y_world ne changent pas.
    // Leur position à l'écran changera avec le scrolling.
    // On pourrait ajouter une logique pour les faire disparaître après un certain temps.
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (gameState->items[i].active) {
            // Vérifier si l'item sort de l'écran par la gauche (à cause du scrolling)
            int screen_x = gameState->items[i].x_world - gameState->scroll_x;
            if (screen_x + gameState->items[i].w < 0) {
                // gameState->items[i].active = 0; // Optionnel: faire disparaître les items qui sortent
                // printf("Item %d désactivé (hors écran gauche)\n", i);
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
            int screen_y = item->y_world - gameState->scroll_y; // Supposant pas de scroll_y pour les items

            // Ne dessiner que si visible à l'écran
            if (screen_x + item->w > 0 && screen_x < gameState->screen_width_allegro &&
                screen_y + item->h > 0 && screen_y < gameState->screen_height_allegro) {

                if (item->sprite) {
                    draw_sprite(dest, item->sprite, screen_x, screen_y);
                } else {
                    // Dessiner un placeholder si pas de sprite
                    int color = makecol(0, 255, 255); // Cyan pour l'item Screen Clear
                    if (item->type == ITEM_TYPE_SCREEN_CLEAR) {
                        rectfill(dest, screen_x, screen_y, screen_x + item->w -1, screen_y + item->h -1, color);
                        // textout_ex(dest, font, screen_x + 2, screen_y + 2, makecol(0,0,0), -1, "C"); // Debug
                    }
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
        printf("Sprite item_screen_clear détruit.\n"); fflush(stdout);
    }
}
