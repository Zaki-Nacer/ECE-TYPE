#include "enemy.h"
#include "defs.h"
#include "graphics.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
// --- Constantes Ennemis ---   A mettre dans un autre fichier plus tard
#define MAX_ENNEMIS 50
#define ENEMY_NBFRAMES_MOVE 4//Nombre de frames pour l'animation de mouvement *** 1 frame = 1 image
#define ENEMY_NBFRAMES_HIT 2//Nombre de frames pour l'animation de dégat prit le hit  *** 1 frame = 1 image
#define ENEMY_NBFRAMES_DEATH 5 //Nombre de frames pour l'animation de mort *** 1 frame = 1 image
#define ENEMY_ANIM_SPEED_MOVE 8
#define ENEMY_ANIM_SPEED_HIT 6
#define ENEMY_ANIM_SPEED_DEATH 10
#define ENEMY_HIT_DURATION (TARGET_FPS / 4) // Durée état HIT ennemi
#define ENEMY_HP 2 // Points de vie des ennemis (uniforme pour simplifier)
#define ENEMY_SPEED_TYPE0 2
#define ENEMY_SPEED_TYPE1 3
#define ENEMY_SPEED_TYPE2 1

// --- Implémentation des Fonctions ---

// la fonction pour charger les sprites d'animations des ennemie, identique a  celle pour les joueurs
void load_process_enemy_anim(GameState *gameState, BITMAP *dest_array[], int nb_frames, const char *base_filename_pattern) {
    if (!gameState) { printf("ERREUR: gameState NULL dans load_process_enemy_anim\n"); fflush(stdout); return; }

    char filename[100];
    BITMAP *original_bmp = NULL;
    BITMAP *resized_bmp = NULL;

    printf("Chargement animation ennemi: %s (%d frames)...\n", base_filename_pattern, nb_frames); fflush(stdout);

    for (int i = 0; i < nb_frames; i++) { // Boucle de 0 à nb_frames - 1
        sprintf(filename, base_filename_pattern, i + 1);
        original_bmp = charger_bitmap_safe(filename);

        int new_w = (int)roundf(original_bmp->w * PLAYER_SCALE_FACTOR);
        int new_h = (int)roundf(original_bmp->h * PLAYER_SCALE_FACTOR);
        if (new_w <= 0) new_w = 1; if (new_h <= 0) new_h = 1;

        resized_bmp = create_bitmap(new_w, new_h);
        if (!resized_bmp) {
            allegro_message("Erreur creation bitmap redimensionné pour ennemi!");
            exit(EXIT_FAILURE);
        }

        stretch_blit(original_bmp, resized_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
        destroy_bitmap(original_bmp);

        make_white_transparent(gameState, resized_bmp);

        dest_array[i] = resized_bmp;
    }
     printf(" -> Animation %s chargée.\n", base_filename_pattern); fflush(stdout);
}

// Charge tous les sprites modèles des ennemis DANS gameState
void charger_sprites_ennemis(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans charger_sprites_ennemis\n"); fflush(stdout); return; }
    printf("--- Chargement Sprites Ennemis ---\n"); fflush(stdout);

    // Type 0
    load_process_enemy_anim(gameState, gameState->sprites_enemy0_move, ENEMY_NBFRAMES_MOVE, "skeleton0-Moving_ (%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy0_hit, ENEMY_NBFRAMES_HIT, "skeleton0-GetHit_ (%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy0_death, ENEMY_NBFRAMES_DEATH, "skeleton0-Destroyed_ (%d).bmp");

    // Type 1
    load_process_enemy_anim(gameState, gameState->sprites_enemy1_move, ENEMY_NBFRAMES_MOVE, "skeleton1-Moving_-_(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy1_hit, ENEMY_NBFRAMES_HIT, "skeleton1-GetHit__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy1_death, ENEMY_NBFRAMES_DEATH, "skeleton1-Destroyed__(%d).bmp");

    // Type 2
    load_process_enemy_anim(gameState, gameState->sprites_enemy2_move, ENEMY_NBFRAMES_MOVE, "skeleton2-Moving__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy2_hit, ENEMY_NBFRAMES_HIT, "skeleton2-GetHit__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy2_death, ENEMY_NBFRAMES_DEATH, "skeleton2-Destroyed__(%d).bmp");

    printf("Tous les sprites ennemis chargés et traités.\n"); fflush(stdout);
}

// Initialise le tableau ennemis DANS gameState
void initialiser_ennemis_array(GameState *gameState) {
    if (!gameState) return;
    printf("Initialisation du tableau d'ennemis...\n"); fflush(stdout);
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        gameState->ennemis[i].active = 0;
        gameState->ennemis[i].current_sprites = NULL;
    }
}

// Définit l'état d'un ennemi spécifique DANS gameState
void set_enemy_state(GameState *gameState, Ennemi *e, EnemyState new_state) {
    if (!gameState || !e) return; // Vérifier e aussi
    // Si l'ennemi n'était pas actif, on ne change pas son état (sauf si c'est l'init dans spawn)
    if (!e->active && new_state != ENEMY_STATE_MOVING) return;
    // Éviter changement inutile sauf si on passe à HIT
    if (e->state == new_state && new_state != ENEMY_STATE_HIT) return;

    long enemy_index = e - gameState->ennemis; // Pour identifier l'ennemi dans les logs
    // printf("DEBUG: set_enemy_state pour Ennemi %ld -> nouvel état %d (était %d)\n", enemy_index, new_state, e->state); fflush(stdout);

    e->state = new_state;
    e->imgcourante = 0;
    e->cptimg = 0;
    e->state_timer = 0;
    e->current_sprites = NULL; // Important de réinitialiser avant d'assigner
    e->current_nb_frames = 0;
    e->tmpimg = 0;

    // Assigner le bon tableau de sprites, nb_frames et vitesse
    switch (e->type) {
        case 0:
            switch (new_state) {
                case ENEMY_STATE_MOVING: e->current_sprites = gameState->sprites_enemy0_move; e->current_nb_frames = ENEMY_NBFRAMES_MOVE; e->tmpimg = ENEMY_ANIM_SPEED_MOVE; break;
                case ENEMY_STATE_HIT:    e->current_sprites = gameState->sprites_enemy0_hit;  e->current_nb_frames = ENEMY_NBFRAMES_HIT;  e->tmpimg = ENEMY_ANIM_SPEED_HIT; e->state_timer = ENEMY_HIT_DURATION; break;
                case ENEMY_STATE_DYING:  e->current_sprites = gameState->sprites_enemy0_death; e->current_nb_frames = ENEMY_NBFRAMES_DEATH; e->tmpimg = ENEMY_ANIM_SPEED_DEATH; break;
            }
            break;
        case 1:
             switch (new_state) {
                case ENEMY_STATE_MOVING: e->current_sprites = gameState->sprites_enemy1_move; e->current_nb_frames = ENEMY_NBFRAMES_MOVE; e->tmpimg = ENEMY_ANIM_SPEED_MOVE; break;
                case ENEMY_STATE_HIT:    e->current_sprites = gameState->sprites_enemy1_hit;  e->current_nb_frames = ENEMY_NBFRAMES_HIT;  e->tmpimg = ENEMY_ANIM_SPEED_HIT; e->state_timer = ENEMY_HIT_DURATION; break;
                case ENEMY_STATE_DYING:  e->current_sprites = gameState->sprites_enemy1_death; e->current_nb_frames = ENEMY_NBFRAMES_DEATH; e->tmpimg = ENEMY_ANIM_SPEED_DEATH; break;
            }
            break;
        case 2:
             switch (new_state) {
                case ENEMY_STATE_MOVING: e->current_sprites = gameState->sprites_enemy2_move; e->current_nb_frames = ENEMY_NBFRAMES_MOVE; e->tmpimg = ENEMY_ANIM_SPEED_MOVE; break;
                case ENEMY_STATE_HIT:    e->current_sprites = gameState->sprites_enemy2_hit;  e->current_nb_frames = ENEMY_NBFRAMES_HIT;  e->tmpimg = ENEMY_ANIM_SPEED_HIT; e->state_timer = ENEMY_HIT_DURATION; break;
                case ENEMY_STATE_DYING:  e->current_sprites = gameState->sprites_enemy2_death; e->current_nb_frames = ENEMY_NBFRAMES_DEATH; e->tmpimg = ENEMY_ANIM_SPEED_DEATH; break;
            }
            break;
        default:
            printf("ERREUR: set_enemy_state type ennemi inconnu %d pour ennemi %ld\n", e->type, enemy_index); fflush(stdout);
            e->active = 0; // Désactiver si type inconnu
            return;
    }

    // *** Vérification cruciale des sprites assignés ***
    if (e->current_sprites == NULL) {
        printf("ERREUR CRITIQUE: Ennemi %ld (type %d), état %d: current_sprites est NULL après assignation!\n", enemy_index, e->type, new_state); fflush(stdout);
        e->active = 0; // Désactiver par sécurité

    } else if (e->current_sprites[0] == NULL) {
        // Vérifier au moins la première frame
        printf("ERREUR CRITIQUE: Ennemi %ld (type %d), état %d: La première frame (current_sprites[0]) est NULL!\n", enemy_index, e->type, new_state); fflush(stdout);
        e->active = 0; // Désactiver par sécurité
    } else {
        // Si tout semble OK, mettre à jour w/h
        e->w = e->current_sprites[0]->w;
        e->h = e->current_sprites[0]->h;
    }
     // Si l'ennemi a été désactivé à cause d'une erreur de sprite, s'assurer que current_sprites est NULL
    if (!e->active) {
        e->current_sprites = NULL;
    }
}


// Crée un ennemi DANS gameState
void spawn_ennemi(GameState *gameState, int type, int pos_x_monde, int pos_y_monde) {
    if (!gameState) return;
    int slot_trouve = -1;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        if (!gameState->ennemis[i].active) {
            slot_trouve = i;
            break;
        }
    }

    if (slot_trouve != -1) {
        Ennemi *e = &gameState->ennemis[slot_trouve];

        // Initialiser les propriétés de base
        e->active = 1; // Marquer comme actif *avant* d'appeler set_enemy_state
        e->type = type;
        e->x = pos_x_monde;
        e->y = pos_y_monde;
        e->health = ENEMY_HP;
        e->state = -1; // Mettre un état invalide pour forcer le changement dans set_enemy_state

        printf("DEBUG: Tentative de spawn ennemi type %d dans slot %d à monde (%d, %d)\n", type, slot_trouve, e->x, e->y); fflush(stdout);

        // Définir l'état initial (MOVING) et charger les sprites correspondants
        set_enemy_state(gameState, e, ENEMY_STATE_MOVING);

        // *** Vérifier si l'ennemi est toujours actif APRES set_enemy_state ***
        if (e->active) {
             printf("DEBUG: Ennemi %d (type %d) spawné avec succès. État: %d, HP: %d, w:%d, h:%d\n",
                    slot_trouve, type, e->state, e->health, e->w, e->h); fflush(stdout);
        } else {
             // Si set_enemy_state l'a désactivé (probablement erreur sprite)
             printf("ERREUR: Ennemi %d (type %d) a été désactivé immédiatement après spawn (vérifier logs set_enemy_state).\n",
                    slot_trouve, type); fflush(stdout);
             // L'ennemi reste inactif dans le slot
        }

    } else {
        printf("Attention: Tableau ennemis plein, impossible de spawner type %d !\n", type); fflush(stdout);
    }
}


// Met à jour les ennemis DANS gameState
void mettre_a_jour_ennemis(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        Ennemi *e = &gameState->ennemis[i];
        if (e->active) {

            // 1. Gérer les états et transitions
            switch (e->state) {
                case ENEMY_STATE_MOVING:
                    {
                        int vitesse = 0;
                        switch(e->type) {
                            case 0: vitesse = ENEMY_SPEED_TYPE0; break;
                            case 1: vitesse = ENEMY_SPEED_TYPE1; break;
                            case 2: vitesse = ENEMY_SPEED_TYPE2; break;
                            default: vitesse = 1;
                        }
                        e->x -= vitesse;
                    }
                    break;

                case ENEMY_STATE_HIT:
                    e->state_timer--;
                    if (e->state_timer <= 0) {
                         // Repasser en MOVING (la vie est gérée par la collision)
                        set_enemy_state(gameState, e, ENEMY_STATE_MOVING);
                    }
                    break;

                case ENEMY_STATE_DYING:
                    // L'animation gère la désactivation
                    break;
            }

            // 2. Mettre à jour l'animation
            if (e->current_sprites && e->current_nb_frames > 0 && e->tmpimg > 0) {
                e->cptimg++;
                if (e->cptimg >= e->tmpimg) {
                    e->cptimg = 0;
                    e->imgcourante++;

                    if (e->imgcourante >= e->current_nb_frames) {
                        if (e->state == ENEMY_STATE_DYING) {
                            e->active = 0; // Désactiver à la fin de l'anim de mort
                        } else if (e->state == ENEMY_STATE_HIT) {
                             e->imgcourante = e->current_nb_frames - 1; // Bloquer sur derniere frame si HIT
                        } else {
                            e->imgcourante = 0; // Boucler pour MOVING
                        }
                    }
                }
                // Mettre à jour w/h si l'ennemi est toujours actif et frame valide
                if (e->active && e->imgcourante < e->current_nb_frames && e->current_sprites[e->imgcourante]) {
                     e->w = e->current_sprites[e->imgcourante]->w;
                     e->h = e->current_sprites[e->imgcourante]->h;
                } else if (e->active && e->state != ENEMY_STATE_DYING) {
                    // Si actif mais frame invalide (ne devrait pas arriver sauf fin anim mort)
                     // printf("WARN: Ennemi %d frame %d invalide pour état %d\n", i, e->imgcourante, e->state); fflush(stdout);
                }
            } else if (e->active && e->state != ENEMY_STATE_DYING) {
                 // printf("WARN: Ennemi %d actif mais sprites/nb_frames/tmpimg invalides (état %d)\n", i, e->state); fflush(stdout);
                 // e->active = 0; // Peut-être trop agressif de désactiver ici
            }


            // 3. Vérifier sortie d'écran (côté gauche)
            if (e->active && (e->x + e->w < gameState->scroll_x)) {
                e->active = 0;
                // printf("DEBUG: Ennemi %d désactivé (hors écran gauche)\n", i); fflush(stdout);
            }
        } // fin if (e->active)
    } // fin for
}


// Dessine les ennemis depuis gameState sur le buffer de gameState
void dessiner_ennemis(GameState *gameState) {
     if (!gameState || !gameState->buffer) return;
     BITMAP *dest = gameState->buffer;

    for (int i = 0; i < MAX_ENNEMIS; i++) {
        Ennemi *e = &gameState->ennemis[i];
        if (e->active) {
            int screen_x = e->x - gameState->scroll_x;
            int screen_y = e->y - gameState->scroll_y;
            int dessine = 0; // Flag pour savoir si on a dessiné

            if (screen_x + e->w > 0 && screen_x < gameState->screen_width_allegro &&
                screen_y + e->h > 0 && screen_y < gameState->screen_height_allegro)
            {
                if (e->current_sprites &&
                    e->imgcourante >= 0 && e->imgcourante < e->current_nb_frames &&
                    e->current_sprites[e->imgcourante])
                {
                     draw_sprite(dest, e->current_sprites[e->imgcourante], screen_x, screen_y);
                     dessine = 1; // On a réussi à dessiner
                } else {
                    // Log d'erreur si sprites invalides pour un ennemi actif et visible
                    printf("ERREUR DESSIN: Sprite invalide pour ennemi %d (état %d, frame %d/%d, pointeur sprite: %p)\n",
                           i, e->state, e->imgcourante, e->current_nb_frames, (void*)e->current_sprites);
                    fflush(stdout);
                    // Dessiner un rectangle d'erreur pour le voir
                    rectfill(dest, screen_x, screen_y, screen_x + 10, screen_y + 10, makecol(255, 0, 0)); // Rouge
                    dessine = 1; // On a dessiné (le rectangle d'erreur)
                }
            }



        }
    }
}
