#include "player.h"
#include "defs.h"
#include "graphics.h"
#include <stdio.h>
#include <math.h>

// --- Implémentation des Fonctions ---

void load_process_player_anim(GameState *gameState, BITMAP *dest_array[], int nb_frames, const char *base_filename_pattern) {
    if (!gameState) return;
    char filename[100];
    BITMAP *original_bmp = NULL;
    BITMAP *processed_bmp = NULL;

    for (int i = 0; i < nb_frames; i++) {
        sprintf(filename, base_filename_pattern, i + 1);
        original_bmp = charger_bitmap_safe(filename);

        int new_w = (int)roundf(original_bmp->w * PLAYER_SCALE_FACTOR);
        int new_h = (int)roundf(original_bmp->h * PLAYER_SCALE_FACTOR);
        if (new_w <= 0) new_w = 1; if (new_h <= 0) new_h = 1;

        processed_bmp = create_bitmap(new_w, new_h);
        if (!processed_bmp) {
            allegro_message("Erreur creation bitmap joueur pour animation");
            allegro_exit();
            exit(EXIT_FAILURE);
        }

        stretch_blit(original_bmp, processed_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
        destroy_bitmap(original_bmp);

        make_white_transparent(gameState, processed_bmp);

        dest_array[i] = processed_bmp;
    }
}

void load_player_animations(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans load_player_animations\n"); fflush(stdout); return; }

    printf("Chargement des animations du joueur...\n"); fflush(stdout);
    load_process_player_anim(gameState, gameState->sprites_player_idle_move, PLAYER_NBFRAMES_IDLE_MOVE, "skeletonally-MovingNIdle_ (%d).bmp");
    load_process_player_anim(gameState, gameState->sprites_player_hit, PLAYER_NBFRAMES_HIT, "skeletonally0-GetHit__(%d).bmp");
    load_process_player_anim(gameState, gameState->sprites_player_death, PLAYER_NBFRAMES_DEATH, "skeletonally0-Destroy__(%d).bmp");

    if (PLAYER_NBFRAMES_IDLE_MOVE > 0 && gameState->sprites_player_idle_move[0]) {
        gameState->joueur.w = gameState->sprites_player_idle_move[0]->w;
        gameState->joueur.h = gameState->sprites_player_idle_move[0]->h;
    } else {
         allegro_message("ERREUR CRITIQUE: Impossible de déterminer la taille du joueur (sprites_player_idle_move[0] manquant ou PLAYER_NBFRAMES_IDLE_MOVE est 0).");
         allegro_exit();
         exit(EXIT_FAILURE);
    }
    printf("Animations du joueur chargées et dimensions initialisées (w:%d, h:%d).\n", gameState->joueur.w, gameState->joueur.h); fflush(stdout);
}

void init_player_state(GameState *gameState) {
     if (!gameState) return;
     Vaisseau *p = &gameState->joueur;

     p->active = 1;
     p->health = PLAYER_INITIAL_HP;

     if (PLAYER_NBFRAMES_IDLE_MOVE > 0 && gameState->sprites_player_idle_move[0]) {
        p->w = gameState->sprites_player_idle_move[0]->w;
        p->h = gameState->sprites_player_idle_move[0]->h;
     } else {
        p->w = 30;
        p->h = 30;
     }

     p->x = 50;
     p->y = gameState->screen_height_allegro / 2 - p->h / 2;



     set_player_state(gameState, STATE_IDLE);
     printf("État du joueur initialisé. HP: %d, Pos: (%d,%d)\n", p->health, p->x, p->y); fflush(stdout);
}

void set_player_state(GameState *gameState, PlayerState new_state) {
    if (!gameState) return;
    Vaisseau *p = &gameState->joueur;

    if ( (p->state == STATE_DYING && new_state != STATE_DYING) ||
         (!p->active && new_state != STATE_DYING) ) {
        return;
    }

    if (p->state == new_state && new_state != STATE_HIT) return;

    p->state = new_state;
    p->imgcourante = 0;
    p->cptimg = 0;

    switch (new_state) {
        case STATE_IDLE:
        case STATE_MOVING:
            p->current_sprites = gameState->sprites_player_idle_move;
            p->current_nb_frames = PLAYER_NBFRAMES_IDLE_MOVE;
            p->tmpimg = PLAYER_ANIM_SPEED_MOVE;
            break;
        case STATE_HIT:
            p->current_sprites = gameState->sprites_player_hit;
            p->current_nb_frames = PLAYER_NBFRAMES_HIT;
            p->tmpimg = PLAYER_ANIM_SPEED_HIT;
            p->state_timer = PLAYER_INVINCIBILITY_DURATION;
            break;
        case STATE_DYING:
            p->current_sprites = gameState->sprites_player_death;
            p->current_nb_frames = PLAYER_NBFRAMES_DEATH;
            p->tmpimg = PLAYER_ANIM_SPEED_DEATH;
            // p->shield_active = 0; // Supprimé
            // p->shield_timer = 0;  // Supprimé
            break;
    }

    if (p->current_sprites && p->current_nb_frames > 0 && p->current_sprites[0]) {
        p->w = p->current_sprites[0]->w;
        p->h = p->current_sprites[0]->h;
    } else {
        if (new_state != STATE_DYING) {
            // printf("AVERTISSEMENT: Sprites non valides pour l'état joueur %d.\n", new_state);
        }
    }
}

void damage_player(GameState *gameState, int amount) {
    if (!gameState || !gameState->joueur.active || gameState->joueur.state == STATE_DYING || gameState->joueur.state_timer > 0) {
        return;
    }

    // La logique du bouclier est supprimée ici

    printf("Joueur touché ! (-%d HP)\n", amount); fflush(stdout);
    gameState->joueur.health -= amount;

    if (gameState->joueur.health <= 0) {
        gameState->joueur.health = 0;
        set_player_state(gameState, STATE_DYING);
    } else {
        set_player_state(gameState, STATE_HIT);
    }
}


void mettre_a_jour_joueur(GameState *gameState) {
    if (!gameState) return;
    Vaisseau *p = &gameState->joueur;

    if (!p->active && p->state != STATE_DYING) return;

    if (p->state_timer > 0 && p->state == STATE_HIT) {
        p->state_timer--;
        if (p->state_timer == 0) {
            set_player_state(gameState, STATE_IDLE);
        }
    }

    // La logique du timer de bouclier est supprimée

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

    if (p->state != STATE_HIT && p->state != STATE_DYING) {
        if (wants_to_move) {
            if (p->state != STATE_MOVING) {
                set_player_state(gameState, STATE_MOVING);
                if (PLAYER_NBFRAMES_IDLE_MOVE > 1 && p->current_nb_frames > 1) p->imgcourante = 1; else p->imgcourante = 0;
            }
        } else {
            if (p->state != STATE_IDLE) {
                set_player_state(gameState, STATE_IDLE);
            }
        }
    }

    if (p->current_sprites && p->current_nb_frames > 0 && p->tmpimg > 0) {
        p->cptimg++;
        if (p->cptimg >= p->tmpimg) {
            p->cptimg = 0;
            p->imgcourante++;

            if (p->imgcourante >= p->current_nb_frames) {
                if (p->state == STATE_DYING) {
                    p->imgcourante = p->current_nb_frames - 1;
                    p->active = 0;
                    if(gameState->current_game_state == GAME_STATE_PLAYING) {
                        gameState->current_game_state = GAME_STATE_GAME_OVER;
                    }
                    printf("Joueur animation mort terminée -> GAME OVER\n"); fflush(stdout);
                } else if (p->state == STATE_HIT) {
                    p->imgcourante = 0;
                } else if (p->state == STATE_MOVING) {
                    p->imgcourante = (PLAYER_NBFRAMES_IDLE_MOVE > 1 && p->current_nb_frames > 1) ? 1 : 0;
                } else {
                    p->imgcourante = 0;
                }
            }
        }
    } else if (p->state == STATE_DYING && !p->current_sprites) {
        p->active = 0;
        if(gameState->current_game_state == GAME_STATE_PLAYING) {
            gameState->current_game_state = GAME_STATE_GAME_OVER;
        }
    }

    if (p->active) {
        if (p->x < 0) p->x = 0;
        if (p->x + p->w > gameState->screen_width_allegro) p->x = gameState->screen_width_allegro - p->w;
        if (p->y < 0) p->y = 0;
        if (p->y + p->h > gameState->screen_height_allegro) p->y = gameState->screen_height_allegro - p->h;
    }
}

void dessiner_joueur(GameState *gameState) {
     if (!gameState || !gameState->buffer ) return;
     Vaisseau *p = &gameState->joueur;
     BITMAP *dest = gameState->buffer;

    if (!p->active && p->state != STATE_DYING) return;

    if (p->current_sprites && p->imgcourante >= 0 && p->imgcourante < p->current_nb_frames && p->current_sprites[p->imgcourante]) {
        if (p->state == STATE_HIT && p->state_timer > 0 && (p->state_timer / (TARGET_FPS / 10)) % 2 == 0) {
            // Ne rien dessiner pour faire clignoter
        } else {
            draw_sprite(dest, p->current_sprites[p->imgcourante], p->x, p->y);
        }
    } else if (p->active || p->state == STATE_DYING) {
         if (p->w > 0 && p->h > 0) {
            rectfill(dest, p->x, p->y, p->x + p->w -1 , p->y + p->h -1, makecol(0, 255, 0));
         }
    }

    // La logique de dessin du bouclier est supprimée
}

void nettoyer_ressources_joueur(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources joueur...\n"); fflush(stdout);
    void destroy_bitmap_array(BITMAP* arr[], int size) {
        if (!arr) return;
        for (int i = 0; i < size; i++) {
            if (arr[i]) {
                destroy_bitmap(arr[i]);
                arr[i] = NULL;
            }
        }
    }
    destroy_bitmap_array(gameState->sprites_player_idle_move, PLAYER_NBFRAMES_IDLE_MOVE);
    destroy_bitmap_array(gameState->sprites_player_hit, PLAYER_NBFRAMES_HIT);
    destroy_bitmap_array(gameState->sprites_player_death, PLAYER_NBFRAMES_DEATH);
    printf("Sprites joueur détruits.\n"); fflush(stdout);
}

