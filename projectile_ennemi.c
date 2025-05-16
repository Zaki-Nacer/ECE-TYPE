#include "projectile_ennemi.h"
#include "defs.h"
#include "graphics.h" // Pour charger_bitmap_safe, make_white_transparent
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Pour roundf si redimensionnement

// --- Implémentation des Fonctions ---

void charger_sprite_projectile_ennemi(GameState *gameState) {
    if (!gameState) {
        printf("ERREUR: gameState NULL dans charger_sprite_projectile_ennemi\n"); fflush(stdout);
        return;
    }

    printf("Chargement sprite projectile ennemi...\n"); fflush(stdout);
    // *** ASSUREZ-VOUS QUE CE FICHIER EXISTE ***
    BITMAP *original_bmp = charger_bitmap_safe("tir_joueur.bmp");

    // Redimensionnement basé sur le facteur d'échelle défini
    int new_w = (int)roundf(original_bmp->w * PROJECTILE_ENNEMI_SCALE_FACTOR);
    int new_h = (int)roundf(original_bmp->h * PROJECTILE_ENNEMI_SCALE_FACTOR);
    if (new_w <= 0) new_w = 1;
    if (new_h <= 0) new_h = 1;

    BITMAP *resized_bmp = create_bitmap(new_w, new_h);
    if (!resized_bmp) {
        allegro_message("Erreur creation bitmap redimensionné pour projectile ennemi!");
        exit(EXIT_FAILURE);
    }

    stretch_blit(original_bmp, resized_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
    destroy_bitmap(original_bmp);

    make_white_transparent(gameState, resized_bmp);

    gameState->sprite_projectile_ennemi = resized_bmp;
    printf("Sprite projectile ennemi chargé, redimensionné (%dx%d) et traité.\n", new_w, new_h); fflush(stdout);

     if (gameState->sprite_projectile_ennemi == NULL) {
        printf("ERREUR FATALE: gameState->sprite_projectile_ennemi est NULL après chargement!\n"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

void initialiser_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    // printf("Initialisation tableau projectiles ennemis...\n"); fflush(stdout); // Optionnel
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        gameState->projectiles_ennemi[i].active = 0;
    }
}

void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter) {
    if (!gameState || !shooter || !shooter->active) {
        // Ne pas tirer si le tireur est invalide ou inactif
        return;
    }
    if (!gameState->sprite_projectile_ennemi) {
        printf("ERREUR: Tentative de spawn projectile ennemi mais sprite non chargé.\n"); fflush(stdout);
        return;
    }

    // Chercher un slot inactif dans le tableau des projectiles ennemis
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        if (!gameState->projectiles_ennemi[i].active) {
            ProjectileEnnemi *p = &gameState->projectiles_ennemi[i];

            p->active = 1;
            p->speed = PROJECTILE_ENNEMI_SPEED;
            p->sprite = gameState->sprite_projectile_ennemi; // Assigner le sprite chargé
            p->w = p->sprite->w;
            p->h = p->sprite->h;

            // Calculer la position de départ (en coordonnées ÉCRAN)
            // Devant l'ennemi, centré verticalement
            // Attention: shooter->x et y sont en coordonnées MONDE
            int shooter_screen_x = shooter->x - gameState->scroll_x;
            int shooter_screen_y = shooter->y - gameState->scroll_y;

            // Le projectile part de la gauche de l'ennemi (vers le joueur)
            p->x = shooter_screen_x - p->w; // Juste à gauche du bord gauche de l'ennemi
            p->y = shooter_screen_y + shooter->h / 2 - p->h / 2; // Centré verticalement

            // printf("DEBUG: Projectile ennemi spawné par ennemi %ld en slot %d à écran (%d, %d)\n",
            //        (shooter - gameState->ennemis), i, p->x, p->y); fflush(stdout);

            return; // Sortir dès qu'un slot est trouvé
        }
    }
    // Si on arrive ici, le tableau est plein
    // printf("Attention: Impossible de spawner projectile ennemi, tableau plein !\n"); fflush(stdout);
}

void mettre_a_jour_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        ProjectileEnnemi *p = &gameState->projectiles_ennemi[i];
        if (p->active) {
            // Déplacer le projectile vers la gauche
            p->x -= p->speed;

            // Vérifier sortie d'écran (côté gauche)
            if (p->x + p->w < 0) {
                p->active = 0; // Désactiver s'il sort de l'écran
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
            if (p->sprite) {
                // Dessiner le sprite du projectile ennemi
                draw_sprite(dest, p->sprite, p->x, p->y);
            } else {
                // Fallback si sprite invalide
                rectfill(dest, p->x, p->y, p->x + 5, p->y + 5, gameState->couleur_blanche);
            }
        }
    }
}

void nettoyer_ressources_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources projectiles ennemis...\n"); fflush(stdout);
    if (gameState->sprite_projectile_ennemi) {
        destroy_bitmap(gameState->sprite_projectile_ennemi);
        gameState->sprite_projectile_ennemi = NULL;
        printf("Sprite modèle projectile ennemi détruit.\n"); fflush(stdout);
    }
}

