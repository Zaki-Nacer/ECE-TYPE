#include "projectile_ennemi.h"
#include "defs.h"
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// --- Implémentation des Fonctions ---

// Nom de fonction modifié pour charger LES sprites des projectiles ennemis
void charger_sprites_projectiles_ennemis(GameState *gameState) {
    if (!gameState) {
        printf("ERREUR: gameState NULL dans charger_sprites_projectiles_ennemis\n"); fflush(stdout);
        return;
    }

    printf("Chargement des sprites des projectiles ennemis...\n"); fflush(stdout);

    BITMAP *original_bmp1 = NULL;
    BITMAP *resized_bmp1 = NULL;
    BITMAP *original_bmp2 = NULL;
    BITMAP *resized_bmp2 = NULL;

    // Charger tire_ennemie1.bmp
    original_bmp1 = charger_bitmap_safe("tire_ennemie1.bmp"); // Assurez-vous que ce fichier existe
    if (original_bmp1) {
        int new_w1 = (int)roundf(original_bmp1->w * PROJECTILE_ENNEMI_SCALE_FACTOR);
        int new_h1 = (int)roundf(original_bmp1->h * PROJECTILE_ENNEMI_SCALE_FACTOR);
        if (new_w1 <= 0) new_w1 = 1;
        if (new_h1 <= 0) new_h1 = 1;

        resized_bmp1 = create_bitmap(new_w1, new_h1);
        if (!resized_bmp1) {
            allegro_message("Erreur creation bitmap redimensionné pour tire_ennemie1!");
            if(original_bmp1) destroy_bitmap(original_bmp1);
            allegro_exit();
            exit(EXIT_FAILURE);
        }
        stretch_blit(original_bmp1, resized_bmp1, 0, 0, original_bmp1->w, original_bmp1->h, 0, 0, new_w1, new_h1);
        destroy_bitmap(original_bmp1);
        make_white_transparent(gameState, resized_bmp1);
        gameState->sprite_projectile_ennemi1 = resized_bmp1;
        printf("Sprite tire_ennemie1.bmp chargé et traité (%dx%d).\n", new_w1, new_h1); fflush(stdout);
    } else {
        printf("ERREUR FATALE: Impossible de charger tire_ennemie1.bmp\n"); fflush(stdout);
        // Vous pourriez vouloir quitter ou utiliser un placeholder ici
        gameState->sprite_projectile_ennemi1 = NULL;
    }

    // Charger tire_ennemie2.bmp
    original_bmp2 = charger_bitmap_safe("tire_ennemie2.bmp"); // Assurez-vous que ce fichier existe
    if (original_bmp2) {
        int new_w2 = (int)roundf(original_bmp2->w * PROJECTILE_ENNEMI_SCALE_FACTOR);
        int new_h2 = (int)roundf(original_bmp2->h * PROJECTILE_ENNEMI_SCALE_FACTOR);
        if (new_w2 <= 0) new_w2 = 1;
        if (new_h2 <= 0) new_h2 = 1;

        resized_bmp2 = create_bitmap(new_w2, new_h2);
        if (!resized_bmp2) {
            allegro_message("Erreur creation bitmap redimensionné pour tire_ennemie2!");
            if(original_bmp2) destroy_bitmap(original_bmp2);
            if(gameState->sprite_projectile_ennemi1) destroy_bitmap(gameState->sprite_projectile_ennemi1); // Nettoyer le précédent si échec ici
            allegro_exit();
            exit(EXIT_FAILURE);
        }
        stretch_blit(original_bmp2, resized_bmp2, 0, 0, original_bmp2->w, original_bmp2->h, 0, 0, new_w2, new_h2);
        destroy_bitmap(original_bmp2);
        make_white_transparent(gameState, resized_bmp2);
        gameState->sprite_projectile_ennemi2 = resized_bmp2;
        printf("Sprite tire_ennemie2.bmp chargé et traité (%dx%d).\n", new_w2, new_h2); fflush(stdout);
    } else {
        printf("ERREUR FATALE: Impossible de charger tire_ennemie2.bmp\n"); fflush(stdout);
        gameState->sprite_projectile_ennemi2 = NULL;
    }

    // Vérification finale
    if (!gameState->sprite_projectile_ennemi1 || !gameState->sprite_projectile_ennemi2) {
        printf("AVERTISSEMENT: Un ou plusieurs sprites de projectiles ennemis n'ont pas pu être chargés.\n");
        // Le jeu peut continuer avec des projectiles invisibles ou des placeholders si géré dans dessiner_projectiles_ennemi
    }
}

void initialiser_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        gameState->projectiles_ennemi[i].active = 0;
    }
}

// MODIFIÉ: Accepte sprite_to_use et damage_value
void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter, int y_offset, int speed, BITMAP *sprite_to_use, int damage_value) {
    if (!gameState || !shooter || !shooter->active) {
        return;
    }
    if (!sprite_to_use) { // Vérifier si le sprite fourni est valide
        printf("ERREUR: Tentative de spawn projectile ennemi avec sprite NULL.\n"); fflush(stdout);
        return;
    }

    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        if (!gameState->projectiles_ennemi[i].active) {
            ProjectileEnnemi *p = &gameState->projectiles_ennemi[i];

            p->active = 1;
            p->speed = speed;
            p->sprite = sprite_to_use; // Utiliser le sprite passé en paramètre
            p->damage = damage_value;  // Assigner les dégâts

            if (p->sprite) {
                p->w = p->sprite->w;
                p->h = p->sprite->h;
            } else { // Fallback si, pour une raison quelconque, le sprite est encore NULL
                p->w = 5;
                p->h = 5;
            }

            int shooter_screen_x = shooter->x - gameState->scroll_x;
            int shooter_screen_y = shooter->y - gameState->scroll_y_dummy;

            p->x = shooter_screen_x - p->w;
            p->y = shooter_screen_y + (shooter->h / 2) - (p->h / 2) + y_offset;

            return;
        }
    }
}

void mettre_a_jour_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        ProjectileEnnemi *p = &gameState->projectiles_ennemi[i];
        if (p->active) {
            p->x -= p->speed;

            if (p->x + p->w < 0) {
                p->active = 0;
            }
        }
    }
}

void dessiner_projectiles_ennemi(GameState *gameState) {
    if (!gameState || !gameState->buffer) return;
    BITMAP *dest = gameState->buffer;

    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        ProjectileEnnemi *p = &gameState->projectiles_ennemi[i];
        if (p->active) {
            if (p->sprite) { // Utiliser le sprite assigné au projectile
                draw_sprite_h_flip(dest, p->sprite, p->x, p->y);
            } else {
                // Fallback si sprite est NULL (ne devrait pas arriver si spawn_projectile_ennemi vérifie)
                rectfill(dest, p->x, p->y, p->x + (p->w > 0 ? p->w : 5) -1 , p->y + (p->h > 0 ? p->h : 5) -1, gameState->couleur_blanche);
            }
        }
    }
}

// Nom de fonction modifié
void nettoyer_ressources_projectiles_ennemis(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources projectiles ennemis...\n"); fflush(stdout);
    if (gameState->sprite_projectile_ennemi1) {
        destroy_bitmap(gameState->sprite_projectile_ennemi1);
        gameState->sprite_projectile_ennemi1 = NULL;
    }
    if (gameState->sprite_projectile_ennemi2) {
        destroy_bitmap(gameState->sprite_projectile_ennemi2);
        gameState->sprite_projectile_ennemi2 = NULL;
    }
    printf("Sprites modèles projectiles ennemis détruits.\n"); fflush(stdout);
}
