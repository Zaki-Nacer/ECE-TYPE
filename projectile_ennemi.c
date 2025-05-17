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
    // Utilise actuellement "tir_joueur.bmp" comme placeholder.
    // Changez "tir_ennemi.bmp" (ou autre) si vous avez un sprite spécifique.
    // Pour l'instant, tous les tirs ennemis utilisent ce même sprite.
    BITMAP *original_bmp = charger_bitmap_safe("tir_joueur.bmp");

    int new_w = (int)roundf(original_bmp->w * PROJECTILE_ENNEMI_SCALE_FACTOR);
    int new_h = (int)roundf(original_bmp->h * PROJECTILE_ENNEMI_SCALE_FACTOR);
    if (new_w <= 0) new_w = 1;
    if (new_h <= 0) new_h = 1;

    BITMAP *resized_bmp = create_bitmap(new_w, new_h);
    if (!resized_bmp) {
        allegro_message("Erreur creation bitmap redimensionné pour projectile ennemi!");
        allegro_exit();
        exit(EXIT_FAILURE);
    }

    stretch_blit(original_bmp, resized_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
    destroy_bitmap(original_bmp);

    make_white_transparent(gameState, resized_bmp);

    gameState->sprite_projectile_ennemi = resized_bmp;
    printf("Sprite projectile ennemi chargé, redimensionné (%dx%d) et traité.\n", new_w, new_h); fflush(stdout);

     if (gameState->sprite_projectile_ennemi == NULL) {
        printf("ERREUR FATALE: gameState->sprite_projectile_ennemi est NULL après chargement!\n"); fflush(stdout);
        allegro_exit();
        exit(EXIT_FAILURE);
    }
}

void initialiser_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        gameState->projectiles_ennemi[i].active = 0;
    }
}

// La définition de la fonction spawn_projectile_ennemi accepte maintenant 4 arguments:
// gameState, l'ennemi tireur (shooter), un décalage vertical (y_offset), et la vitesse du projectile (speed).
void spawn_projectile_ennemi(GameState *gameState, Ennemi *shooter, int y_offset, int speed) {
    if (!gameState || !shooter || !shooter->active) {
        return; // Ne pas tirer si le tireur est invalide ou inactif
    }
    if (!gameState->sprite_projectile_ennemi) {
        printf("ERREUR: Tentative de spawn projectile ennemi mais sprite non chargé.\n"); fflush(stdout);
        return; // Ne pas tirer si le sprite du projectile n'est pas chargé
    }

    // Chercher un slot inactif pour le nouveau projectile
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        if (!gameState->projectiles_ennemi[i].active) {
            ProjectileEnnemi *p = &gameState->projectiles_ennemi[i]; // Pointeur vers le projectile

            p->active = 1; // Activer le projectile
            p->speed = speed; // Assigner la vitesse passée en argument
            p->sprite = gameState->sprite_projectile_ennemi; // Assigner le sprite modèle

            // Vérifier si le sprite est valide avant d'accéder à w et h
            if (p->sprite) {
                p->w = p->sprite->w; // Largeur du projectile
                p->h = p->sprite->h; // Hauteur du projectile
            } else {
                // Fallback si le sprite est NULL (ne devrait pas arriver si le chargement est correct)
                p->w = 5;
                p->h = 5;
                printf("WARN: Sprite de projectile ennemi est NULL lors du spawn.\n"); fflush(stdout);
            }


            // Calculer les coordonnées de l'ennemi à l'écran
            int shooter_screen_x = shooter->x - gameState->scroll_x;
            int shooter_screen_y = shooter->y - gameState->scroll_y;

            // Positionner le projectile
            // Le projectile part du "nez" de l'ennemi (bord gauche du sprite de l'ennemi)
            p->x = shooter_screen_x - p->w;
            // Centré verticalement par rapport au centre de l'ennemi, plus le décalage y_offset
            p->y = shooter_screen_y + (shooter->h / 2) - (p->h / 2) + y_offset;

            // printf("DEBUG: Projectile ennemi (offset %d, speed %d) spawné par ennemi type %d en slot %d à écran (%d, %d)\n",
            //        y_offset, speed, shooter->type, i, p->x, p->y); fflush(stdout);

            return; // Sortir de la fonction dès qu'un slot est trouvé et le projectile créé
        }
    }
    // Si on arrive ici, c'est que le tableau des projectiles ennemis est plein
    // printf("Attention: Impossible de spawner projectile ennemi, tableau plein !\n"); fflush(stdout);
}

void mettre_a_jour_projectiles_ennemi(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_PROJECTILES_ENNEMI; i++) {
        ProjectileEnnemi *p = &gameState->projectiles_ennemi[i];
        if (p->active) {
            // Déplacer le projectile vers la gauche en utilisant sa vitesse individuelle
            p->x -= p->speed;

            // Vérifier si le projectile sort de l'écran par la gauche
            if (p->x + p->w < 0) {
                p->active = 0; // Désactiver le projectile
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
                // Dessiner le sprite du projectile ennemi inversé horizontalement
                draw_sprite_h_flip(dest, p->sprite, p->x, p->y);
            } else {
                // Fallback: dessiner un rectangle si le sprite est manquant
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
