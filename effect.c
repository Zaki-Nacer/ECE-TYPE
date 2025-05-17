#include "effect.h"
#include "defs.h"
#include "graphics.h" // Pour charger_bitmap_safe et make_white_transparent
#include <stdio.h>
#include <math.h>

// --- PAS DE VARIABLES GLOBALES ICI ---

// --- Implémentation des Fonctions ---

void charger_sprites_effects(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans charger_sprites_effects\n"); fflush(stdout); return; }

    char nom_fichier[50];
    int i;
    BITMAP *original_bmp = NULL;
    BITMAP *processed_bmp = NULL;

    printf("Chargement/Traitement sprites effets (impact)...\n"); fflush(stdout);

    for (i = 0; i < IMPACT_NBFRAMES; i++) {
        sprintf(nom_fichier, "impact_(%d).bmp", i+1); // chargement du fichier de l'impact des collision projectiles
        original_bmp = charger_bitmap_safe(nom_fichier);

         processed_bmp = original_bmp;
         printf("Impact frame %d chargée (%dx%d).\n", i, processed_bmp->w, processed_bmp->h); fflush(stdout);
        // ---------------------------------

        make_white_transparent(gameState, processed_bmp); // Traiter transparence

        // Stocker dans gameState
        gameState->sprites_impact[i] = processed_bmp;
    }
    printf("Sprites d'impact chargés et traités.\n"); fflush(stdout);
}

void initialiser_effects(GameState *gameState) {
    if (!gameState) return;
    printf("Initialisation tableau effets...\n"); fflush(stdout);
    for (int i = 0; i < MAX_EFFECTS; i++) {
        gameState->effects[i].active = 0;
        gameState->effects[i].sprites = NULL;
    }
}

void spawn_impact_effect(GameState *gameState, int x, int y) {
    if (!gameState || !gameState->sprites_impact[0]) {
        printf("ERREUR: Tentative de spawn impact mais sprites non chargés dans gameState.\n"); fflush(stdout);
        return;
    }

    for (int i = 0; i < MAX_EFFECTS; i++) {
        if (!gameState->effects[i].active) {
            Effect *e = &gameState->effects[i]; // Pointeur

            e->active = 1;
            e->sprites = gameState->sprites_impact; // Pointe vers modèle dans gameState
            e->nb_frames = IMPACT_NBFRAMES;
            e->anim_speed = IMPACT_ANIM_SPEED;
            e->current_frame = 0;
            e->frame_timer = 0;
            e->w = gameState->sprites_impact[0]->w;
            e->h = gameState->sprites_impact[0]->h;
            e->x = x - e->w / 2; // Centrer
            e->y = y - e->h / 2;

            printf("DEBUG: Effet Impact spawné en slot %d à (%d, %d)\n", i, e->x, e->y); fflush(stdout);
            return;
        }
    }
    printf("Attention: Impossible de spawner effet impact, tableau plein !\n"); fflush(stdout);
}

void mettre_a_jour_effects(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_EFFECTS; i++) {
        Effect *e = &gameState->effects[i];
        if (e->active) {
            e->frame_timer++;
            if (e->frame_timer >= e->anim_speed) {
                e->frame_timer = 0;
                e->current_frame++;
                if (e->current_frame >= e->nb_frames) {
                    e->active = 0;
                    // printf("DEBUG: Effet %d terminé.\n", i); fflush(stdout);
                }
            }
        }
    }
}

void dessiner_effects(GameState *gameState) {
    if (!gameState || !gameState->buffer) return;
    BITMAP *dest = gameState->buffer;

    for (int i = 0; i < MAX_EFFECTS; i++) {
        Effect *e = &gameState->effects[i];
        if (e->active) {
            if (e->sprites && e->current_frame < e->nb_frames && e->sprites[e->current_frame])
            {
                 // Utiliser draw_sprite car prétraité
                 draw_sprite(dest, e->sprites[e->current_frame], e->x, e->y);
            } else {
                 printf("ERREUR: Tentative de dessin effet %d avec sprite/frame invalide\n", i); fflush(stdout);
                 rectfill(dest, e->x, e->y, e->x + 10, e->y + 10, makecol(255, 255, 0));
            }
        }
    }
}

void nettoyer_ressources_effects(GameState *gameState) {
     if (!gameState) return;
    printf("Nettoyage ressources effets...\n"); fflush(stdout);
    for (int i = 0; i < IMPACT_NBFRAMES; i++) {
        if (gameState->sprites_impact[i]) {
            destroy_bitmap(gameState->sprites_impact[i]);
            gameState->sprites_impact[i] = NULL;
        }
    }
    printf("Sprites modèles effets détruits.\n"); fflush(stdout);
}
