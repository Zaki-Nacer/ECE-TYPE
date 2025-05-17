#include "projectile.h"
#include "defs.h"
#include "graphics.h" // Pour charger_bitmap_safe et make_white_transparent
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// --- Implémentation des Fonctions ---

// Charge et prépare le sprite du projectile joueur DANS gameState
void charger_et_redimensionner_sprite_projectile_joueur(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans charger_sprite_projectile\n"); fflush(stdout); return; }

    printf("Chargement sprite projectile joueur...\n"); fflush(stdout);
    BITMAP *original_bmp = charger_bitmap_safe("tir_joueur.bmp"); // Assurez-vous que le fichier existe

    // Calculer les nouvelles dimensions basées sur le facteur d'échelle
    int new_w = (int)roundf(original_bmp->w * PROJECTILE_JOUEUR_SCALE_FACTOR);
    int new_h = (int)roundf(original_bmp->h * PROJECTILE_JOUEUR_SCALE_FACTOR);
    if (new_w <= 0) new_w = 1;
    if (new_h <= 0) new_h = 1;

    // Créer un nouveau bitmap pour le sprite redimensionné
    BITMAP *resized_bmp = create_bitmap(new_w, new_h);
    if (!resized_bmp) {
        allegro_message("Erreur creation bitmap redimensionné pour projectile joueur!");
        exit(EXIT_FAILURE);
    }

    // Redimensionner l'image originale vers le nouveau bitmap
    stretch_blit(original_bmp, resized_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
    destroy_bitmap(original_bmp); // Libérer l'original

    make_white_transparent(gameState, resized_bmp); // Rendre le fond blanc transparent

    // Stocker le sprite traité dans gameState
    gameState->sprite_projectile_joueur = resized_bmp;
    printf("Sprite projectile joueur chargé, redimensionné (%dx%d) et traité.\n", new_w, new_h); fflush(stdout);


}

// Initialise le tableau des projectiles joueur DANS gameState
void initialiser_projectiles_joueur(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        gameState->projectiles_joueur[i].active = 0;
        gameState->projectiles_joueur[i].state = PROJECTILE_STATE_INACTIVE; // Marquer comme inactif
    }
}

// Crée un nouveau projectile joueur DANS gameState
void spawn_projectile_joueur(GameState *gameState, int start_x, int start_y) {
    if (!gameState || !gameState->sprite_projectile_joueur) {
        printf("ERREUR: Tentative de spawn projectile mais sprite non chargé.\n"); fflush(stdout);
        return;
    }

    // Chercher un slot inactif
    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        if (!gameState->projectiles_joueur[i].active) {
            Projectile *p = &gameState->projectiles_joueur[i]; // Pointeur

            p->active = 1;
            p->state = PROJECTILE_STATE_FLYING; // État initial
            p->speed = PROJECTILE_JOUEUR_SPEED;

            // Assigner les sprites (pointeurs vers ceux dans gameState)
            p->sprite_flying = gameState->sprite_projectile_joueur;
            p->sprites_impact = gameState->sprites_impact; // Tableau des sprites d'impact

            // Définir dimensions en vol
            p->w = p->sprite_flying->w;
            p->h = p->sprite_flying->h;

            // Définir dimensions impact (basé sur la 1ère frame d'impact)
            if (p->sprites_impact && IMPACT_NBFRAMES > 0 && p->sprites_impact[0]) {
                p->impact_w = p->sprites_impact[0]->w;
                p->impact_h = p->sprites_impact[0]->h;
            } else {
                p->impact_w = 10; // Valeur par défaut si sprites impact non chargés
                p->impact_h = 10;
                printf("WARN: Sprites d'impact non disponibles pour projectile %d.\n", i); fflush(stdout);
            }

            // Position initiale (centrée sur start_y, devant start_x)
            p->x = start_x;
            p->y = start_y - p->h / 2;

            // Réinitialiser l'animation d'impact
            p->impact_frame = 0;
            p->impact_timer = 0;

            return; // Sortir dès qu'un slot est trouvé
        }
    }
    // Si on arrive ici, le tableau est plein
}

// Met à jour les projectiles joueur DANS gameState
void mettre_a_jour_projectiles_joueur(GameState *gameState) {
    if (!gameState) return;

    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        Projectile *p = &gameState->projectiles_joueur[i];

        if (p->active) {
            switch (p->state) {
                case PROJECTILE_STATE_FLYING:
                    // Déplacer le projectile vers la droite
                    p->x += p->speed;

                    // Vérifier sortie d'écran (côté droit)
                    if (p->x > gameState->screen_width_allegro) {
                        p->active = 0; // Désactiver s'il sort de l'écran
                        p->state = PROJECTILE_STATE_INACTIVE;
                        // Debug: printf("DEBUG: Projectile %d désactivé (hors écran droit)\n", i); fflush(stdout);
                    }
                    break;

                case PROJECTILE_STATE_IMPACTING:
                    // Gérer l'animation d'impact
                    if (p->sprites_impact && IMPACT_NBFRAMES > 0) {
                        p->impact_timer++;
                        if (p->impact_timer >= IMPACT_ANIM_SPEED) { // Utiliser la vitesse définie
                            p->impact_timer = 0;
                            p->impact_frame++;
                            // Si l'animation d'impact est terminée
                            if (p->impact_frame >= IMPACT_NBFRAMES) {
                                p->active = 0; // Désactiver le projectile
                                p->state = PROJECTILE_STATE_INACTIVE;
                                // Debug: printf("DEBUG: Projectile %d désactivé (fin anim impact)\n", i); fflush(stdout);
                            }
                        }
                    } else {
                        // Pas d'animation d'impact, désactiver immédiatement
                        p->active = 0;
                        p->state = PROJECTILE_STATE_INACTIVE;
                    }
                    break;

                case PROJECTILE_STATE_INACTIVE:
                    // Ne rien faire si déjà inactif
                    break;
            }
        } // fin if (p->active)
    } // fin for
}

// Dessine les projectiles joueur depuis gameState sur le buffer de gameState
void dessiner_projectiles_joueur(GameState *gameState) {
    if (!gameState || !gameState->buffer) return;
    BITMAP *dest = gameState->buffer;

    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        Projectile *p = &gameState->projectiles_joueur[i];

        if (p->active) {
            switch (p->state) {
                case PROJECTILE_STATE_FLYING:
                    // Dessiner le sprite de vol normal
                    if (p->sprite_flying) {
                        // Utiliser draw_sprite car prétraité
                        draw_sprite(dest, p->sprite_flying, p->x, p->y);
                    } else {
                         rectfill(dest, p->x, p->y, p->x + 5, p->y + 2, gameState->couleur_blanche); // Fallback
                    }
                    break;

                case PROJECTILE_STATE_IMPACTING:
                    // Dessiner la frame actuelle de l'animation d'impact
                    if (p->sprites_impact &&
                        p->impact_frame >= 0 && p->impact_frame < IMPACT_NBFRAMES &&
                        p->sprites_impact[p->impact_frame])
                    {
                        // Centrer l'impact là où était le projectile (approximativement)
                        int impact_draw_x = p->x + p->w / 2 - p->impact_w / 2;
                        int impact_draw_y = p->y + p->h / 2 - p->impact_h / 2;
                        draw_sprite(dest, p->sprites_impact[p->impact_frame], impact_draw_x, impact_draw_y);
                    }
                     // Ne pas dessiner de fallback ici, car l'impact est court
                    break;

                 case PROJECTILE_STATE_INACTIVE:
                    // Ne rien dessiner
                    break;
            }
        } // fin if (p->active)
    }
}


// Nettoie les ressources spécifiques aux projectiles joueur (modèles dans gameState)
void nettoyer_ressources_projectiles_joueur(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources projectiles joueur...\n"); fflush(stdout);

    // Détruire le sprite de vol
    if (gameState->sprite_projectile_joueur) {
        destroy_bitmap(gameState->sprite_projectile_joueur);
        gameState->sprite_projectile_joueur = NULL;
        printf("Sprite modèle projectile joueur détruit.\n"); fflush(stdout);
    }


}
