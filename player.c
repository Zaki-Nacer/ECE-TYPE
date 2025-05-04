#include "player.h"
#include "defs.h"
#include "graphics.h" // Pour charger_bitmap_safe ET make_white_transparent
#include <stdio.h>
#include <math.h>

// --- PAS DE VARIABLE GLOBALE 'joueur' ICI ---

// --- Implémentation des Fonctions ---

// Fonction helper pour charger/redim/traiter une animation
void load_process_player_anim(GameState *gameState, BITMAP *dest_array[], int nb_frames, const char *base_filename_pattern) {
    if (!gameState) return;
    char filename[100];
    BITMAP *original_bmp = NULL;
    BITMAP *processed_bmp = NULL;

    printf("Chargement animation joueur: %s...\n", base_filename_pattern); fflush(stdout);

    for (int i = 1; i < nb_frames+1; i++) {
        sprintf(filename, base_filename_pattern, i);
        original_bmp = charger_bitmap_safe(filename);

        int new_w = (int)roundf(original_bmp->w * PLAYER_SCALE_FACTOR);
        int new_h = (int)roundf(original_bmp->h * PLAYER_SCALE_FACTOR);
        if (new_w <= 0) new_w = 1; if (new_h <= 0) new_h = 1;

        processed_bmp = create_bitmap(new_w, new_h);
        if (!processed_bmp) { /* ... gestion erreur ... */ exit(EXIT_FAILURE); }

        stretch_blit(original_bmp, processed_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
        destroy_bitmap(original_bmp);

        make_white_transparent(gameState, processed_bmp); // Traiter transparence

        dest_array[i] = processed_bmp;
        printf(" -> Frame %d (%s) chargée/redim/traitée (%dx%d)\n", i, filename, new_w, new_h); fflush(stdout);
    }
}

// Charge toutes les animations du joueur
void load_player_animations(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans load_player_animations\n"); fflush(stdout); return; }

    // Charger Idle/Move (ex: skeleton-MovingNIdle_%d.bmp)
    load_process_player_anim(gameState, gameState->sprites_player_idle_move, PLAYER_NBFRAMES_IDLE_MOVE, "skeletonally-MovingNIdle_ (%d).bmp");

    // Charger Hit (ex: player_hit_%d.bmp) - !!! ADAPTER NOM FICHIER !!!
    load_process_player_anim(gameState, gameState->sprites_player_hit, PLAYER_NBFRAMES_HIT, "skeletonally0-GetHit__(%d).bmp");

    // Charger Death (ex: player_death_%d.bmp) - !!! ADAPTER NOM FICHIER !!!
    load_process_player_anim(gameState, gameState->sprites_player_death, PLAYER_NBFRAMES_DEATH, "skeletonally0-Destroy__(%d).bmp");

    // Initialiser dimensions basées sur la première frame idle/move
    if (gameState->sprites_player_idle_move[1]) {
        gameState->joueur.w = gameState->sprites_player_idle_move[1]->w;
        gameState->joueur.h = gameState->sprites_player_idle_move[1]->h;
        printf("Dimensions joueur initialisées: %dx%d\n", gameState->joueur.w, gameState->joueur.h); fflush(stdout);
    } else {
         allegro_message("ERREUR: Impossible de déterminer la taille du joueur (sprite 0 manquant).");
         exit(EXIT_FAILURE);
    }

    printf("Toutes les animations joueur chargées.\n"); fflush(stdout);
}

// Initialise l'état du joueur (appelé au début du niveau/respawn)
void init_player_state(GameState *gameState) {
     if (!gameState) return;
     printf("Initialisation état joueur...\n"); fflush(stdout);
     gameState->joueur.active = 1; // Vivant
     gameState->joueur.health = PLAYER_INITIAL_HP;
     gameState->joueur.x = 50; // Position de départ
     gameState->joueur.y = gameState->screen_height_allegro / 2 - gameState->joueur.h / 2;
     set_player_state(gameState, STATE_IDLE); // Commence immobile
}

// Change l'état et l'animation du joueur
void set_player_state(GameState *gameState, PlayerState new_state) {
    if (!gameState || !gameState->joueur.active) return;
    Vaisseau *p = &gameState->joueur;

    // Éviter changement inutile sauf si on passe à HIT
    if (p->state == new_state && new_state != STATE_HIT) return;

    printf("DEBUG JOUEUR: Changement état de %d vers %d\n", p->state, new_state); fflush(stdout);

    p->state = new_state;
    p->imgcourante = 0;
    p->cptimg = 0;
    p->state_timer = 0; // Réinitialiser le timer d'état

    switch (new_state) {
        case STATE_IDLE:
        case STATE_MOVING: // Idle et Moving utilisent la même séquence de base
            p->current_sprites = gameState->sprites_player_idle_move;
            p->current_nb_frames = PLAYER_NBFRAMES_IDLE_MOVE;
            p->tmpimg = PLAYER_ANIM_SPEED_MOVE;
            // La frame spécifique (0 pour idle, 1+ pour moving) est gérée dans mettre_a_jour_joueur
            break;
        case STATE_HIT:
            p->current_sprites = gameState->sprites_player_hit;
            p->current_nb_frames = PLAYER_NBFRAMES_HIT;
            p->tmpimg = PLAYER_ANIM_SPEED_HIT;
            p->state_timer = PLAYER_HIT_DURATION; // Durée de l'état touché
            break;
        case STATE_DYING:
            p->current_sprites = gameState->sprites_player_death;
            p->current_nb_frames = PLAYER_NBFRAMES_DEATH;
            p->tmpimg = PLAYER_ANIM_SPEED_DEATH;
            break;
    }

    // Vérifier si les sprites pour le nouvel état sont valides
    if (!p->current_sprites || !p->current_sprites[0]) {
        printf("ERREUR: Sprites non valides pour l'état joueur %d\n", new_state); fflush(stdout);
        // Que faire ? Peut-être revenir à IDLE ou marquer comme mort ?
        // Pour l'instant, on laisse, mais le dessin échouera.
    } else {
        // Mettre à jour w/h au cas où les anims ont des tailles différentes
        p->w = p->current_sprites[0]->w;
        p->h = p->current_sprites[0]->h;
    }
}

// Inflige des dégâts au joueur
void damage_player(GameState *gameState, int amount) {
    if (!gameState || !gameState->joueur.active || gameState->joueur.state == STATE_DYING || gameState->joueur.state_timer > 0) {
        // Ne pas infliger de dégâts si mort, mourant, ou invincible (état HIT ou timer invincibilité)
        return;
    }

    printf("Joueur touché ! (-%d HP)\n", amount); fflush(stdout);
    gameState->joueur.health -= amount;

    if (gameState->joueur.health <= 0) {
        gameState->joueur.health = 0;
        set_player_state(gameState, STATE_DYING); // Déclencher l'animation de mort
    } else {
        set_player_state(gameState, STATE_HIT); // Déclencher l'animation "touché"
        // Mettre un timer d'invincibilité plus long que l'anim HIT
        gameState->joueur.state_timer = PLAYER_INVINCIBILITY_DURATION;
    }
}


// Met à jour le joueur (état, position, animation)
void mettre_a_jour_joueur(GameState *gameState) {
    if (!gameState || !gameState->joueur.active) return;
    Vaisseau *p = &gameState->joueur;

    // --- Gérer les états temporisés ---
    if (p->state_timer > 0) {
        p->state_timer--;
        if (p->state_timer == 0 && p->state == STATE_HIT) {
            // Fin de l'invincibilité/état HIT, revenir à IDLE (sera mis à MOVING si touche pressée)
            set_player_state(gameState, STATE_IDLE);
        }
    }

    // --- Mouvement (seulement si pas en train de mourir) ---
    int wants_to_move = 0;
    if (p->state != STATE_DYING) {
        int dx = 0, dy = 0;
        if (key[KEY_UP])    { dy -= PLAYER_SPEED; wants_to_move = 1; }
        if (key[KEY_DOWN])  { dy += PLAYER_SPEED; wants_to_move = 1; }
        if (key[KEY_LEFT])  { dx -= PLAYER_SPEED; wants_to_move = 1; }
        if (key[KEY_RIGHT]) { dx += PLAYER_SPEED; wants_to_move = 1; }
        p->x += dx;
        p->y += dy;
    }

    // --- Mise à jour de l'état (Idle/Moving si pas HIT ou DYING) ---
    if (p->state != STATE_HIT && p->state != STATE_DYING) {
        if (wants_to_move) {
            if (p->state == STATE_IDLE) {
                set_player_state(gameState, STATE_MOVING);
                if (PLAYER_NBFRAMES_IDLE_MOVE > 1) p->imgcourante = 1; // Commencer anim move à frame 1
            }
        } else {
            if (p->state == STATE_MOVING) {
                set_player_state(gameState, STATE_IDLE);
            }
        }
    }

    // --- Mise à jour de l'animation de l'état actuel ---
    if (p->current_sprites) {
        p->cptimg++;
        if (p->cptimg >= p->tmpimg) {
            p->cptimg = 0;
            p->imgcourante++;

            // Gérer la fin de l'animation
            if (p->imgcourante >= p->current_nb_frames) {
                if (p->state == STATE_DYING) {
                    // Fin de l'animation de mort
                    p->imgcourante = p->current_nb_frames - 1; // Rester sur la dernière frame
                    p->active = 0; // Marquer comme inactif (mort)
                    gameState->current_game_state = GAME_STATE_GAME_OVER; // Déclencher Game Over
                    printf("Joueur animation mort terminée -> GAME OVER\n"); fflush(stdout);
                } else if (p->state == STATE_HIT) {
                    // Boucler sur l'animation HIT tant que l'état est HIT
                    p->imgcourante = 0;
                } else if (p->state == STATE_MOVING) {
                    // Boucler sur les frames de mouvement (1 à N-1)
                    p->imgcourante = (PLAYER_NBFRAMES_IDLE_MOVE > 1) ? 1 : 0;
                } else { // STATE_IDLE
                    p->imgcourante = 0; // Rester sur la frame idle
                }
            }
        }
    }

    // --- Contrôle des bords (si toujours actif) ---
    if (p->active) {
        if (p->x < 0) p->x = 0;
        if (p->x > gameState->screen_width_allegro - p->w) p->x = gameState->screen_width_allegro - p->w;
        if (p->y < 0) p->y = 0;
        if (p->y > gameState->screen_height_allegro - p->h) p->y = gameState->screen_height_allegro - p->h;
    }
}

// Dessine le joueur en utilisant l'animation de l'état actuel
void dessiner_joueur(GameState *gameState) {
     if (!gameState || !gameState->buffer || !gameState->joueur.active) return;
     Vaisseau *p = &gameState->joueur;
     BITMAP *dest = gameState->buffer;

    // Vérifier si les sprites et la frame sont valides
    if (p->current_sprites &&
        p->imgcourante >= 0 && p->imgcourante < p->current_nb_frames &&
        p->current_sprites[p->imgcourante])
    {
        // Utiliser draw_sprite car les bitmaps sont prétraités
        draw_sprite(dest, p->current_sprites[p->imgcourante], p->x, p->y);

        // Optionnel: Afficher un clignotement pendant l'invincibilité après HIT
        if (p->state == STATE_HIT && p->state_timer > 0 && (p->state_timer / 4) % 2 == 0) {
             // Ne rien dessiner une frame sur deux (ou utiliser un filtre)
             // Ou dessiner avec alpha : draw_trans_sprite(dest, p->current_sprites[p->imgcourante], p->x, p->y);
             // Pour l'instant, on ne fait rien de spécial, juste l'anim HIT
        }

    } else {
         printf("ERREUR: Tentative de dessin joueur état %d frame %d invalide.\n", p->state, p->imgcourante); fflush(stdout);
         rectfill(dest, p->x, p->y, p->x + p->w, p->y + p->h, makecol(0, 255, 0));
    }
}

void nettoyer_ressources_joueur(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources joueur...\n"); fflush(stdout);
    // Fonction helper pour détruire un tableau de bitmaps
    void destroy_bitmap_array(BITMAP* arr[], int size) {
        if (!arr) return;
        for (int i = 0; i < size; i++) { if (arr[i]) { destroy_bitmap(arr[i]); arr[i] = NULL; } }
    }
    // Détruire toutes les animations
    destroy_bitmap_array(gameState->sprites_player_idle_move, PLAYER_NBFRAMES_IDLE_MOVE);
    destroy_bitmap_array(gameState->sprites_player_hit, PLAYER_NBFRAMES_HIT);
    destroy_bitmap_array(gameState->sprites_player_death, PLAYER_NBFRAMES_DEATH);
    printf("Sprites joueur détruits.\n"); fflush(stdout);
}
