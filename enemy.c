#include "enemy.h"
#include "defs.h"
#include "graphics.h"
#include "projectile_ennemi.h"
#include "item.h" // NOUVEAU: Inclure pour pouvoir spawner des items
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// --- Implémentation des Fonctions ---

void load_process_enemy_anim(GameState *gameState, BITMAP *dest_array[], int nb_frames, const char *base_filename_pattern) {
    if (!gameState) { printf("ERREUR: gameState NULL dans load_process_enemy_anim\n"); fflush(stdout); return; }
    char filename[100];
    BITMAP *original_bmp = NULL;
    BITMAP *resized_bmp = NULL;
    for (int i = 0; i < nb_frames; i++) {
        sprintf(filename, base_filename_pattern, i + 1);
        original_bmp = charger_bitmap_safe(filename);
        int new_w = (int)roundf(original_bmp->w * PLAYER_SCALE_FACTOR);
        int new_h = (int)roundf(original_bmp->h * PLAYER_SCALE_FACTOR);
        if (new_w <= 0) new_w = 1; if (new_h <= 0) new_h = 1;
        resized_bmp = create_bitmap(new_w, new_h);
        if (!resized_bmp) {
            allegro_message("Erreur creation bitmap redimensionné pour ennemi!");
            allegro_exit(); exit(EXIT_FAILURE);
        }
        stretch_blit(original_bmp, resized_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
        destroy_bitmap(original_bmp);
        make_white_transparent(gameState, resized_bmp);
        dest_array[i] = resized_bmp;
    }
}

void charger_sprites_ennemis(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans charger_sprites_ennemis\n"); fflush(stdout); return; }
    printf("--- Chargement Sprites Ennemis (Types 0-2 pour Niveaux 1 & 3) ---\n"); fflush(stdout);
    load_process_enemy_anim(gameState, gameState->sprites_enemy0_move, ENEMY_NBFRAMES_MOVE, "skeleton0-Moving_ (%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy0_hit, ENEMY_NBFRAMES_HIT, "skeleton0-GetHit_ (%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy0_death, ENEMY_NBFRAMES_DEATH, "skeleton0-Destroyed_ (%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy1_move, ENEMY_NBFRAMES_MOVE, "skeleton1-Moving_-_(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy1_hit, ENEMY_NBFRAMES_HIT, "skeleton1-GetHit__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy1_death, ENEMY_NBFRAMES_DEATH, "skeleton1-Destroyed__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy2_move, ENEMY_NBFRAMES_MOVE, "skeleton2-Moving__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy2_hit, ENEMY_NBFRAMES_HIT, "skeleton2-GetHit__(%d).bmp");
    load_process_enemy_anim(gameState, gameState->sprites_enemy2_death, ENEMY_NBFRAMES_DEATH, "skeleton2-Destroyed__(%d).bmp");
    printf("Sprites ennemis (types 0-2) chargés et traités.\n"); fflush(stdout);
}

void initialiser_ennemis_array(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        gameState->ennemis[i].active = 0;
        gameState->ennemis[i].current_sprites = NULL;
    }
}

void set_enemy_state(GameState *gameState, Ennemi *e, EnemyState new_state) {
    if (!gameState || !e) return;
    // Ne pas changer l'état d'un ennemi inactif sauf à l'initialisation (state == -1)
    if (!e->active && new_state != ENEMY_STATE_MOVING && e->state != -1) return;
    // Éviter changement inutile sauf si on passe à HIT (pour réinitialiser l'anim HIT)
    if (e->state == new_state && new_state != ENEMY_STATE_HIT) return;

    EnemyState old_state = e->state; // Garder l'ancien état pour la logique de drop d'item
    e->state = new_state;
    e->imgcourante = 0;
    e->cptimg = 0;
    e->current_sprites = NULL;
    e->current_nb_frames = 0;
    e->tmpimg = ENEMY_ANIM_SPEED_MOVE;

    if (e->type >= ENEMY_TYPE_0 && e->type <= ENEMY_TYPE_2) { // Sprites pour les types 0, 1, 2
        switch (e->type) {
            case ENEMY_TYPE_0:
                switch (new_state) {
                    case ENEMY_STATE_MOVING: e->current_sprites = gameState->sprites_enemy0_move; e->current_nb_frames = ENEMY_NBFRAMES_MOVE; e->tmpimg = ENEMY_ANIM_SPEED_MOVE; break;
                    case ENEMY_STATE_HIT:    e->current_sprites = gameState->sprites_enemy0_hit;  e->current_nb_frames = ENEMY_NBFRAMES_HIT;  e->tmpimg = ENEMY_ANIM_SPEED_HIT; e->state_timer = ENEMY_HIT_DURATION; break;
                    case ENEMY_STATE_DYING:  e->current_sprites = gameState->sprites_enemy0_death; e->current_nb_frames = ENEMY_NBFRAMES_DEATH; e->tmpimg = ENEMY_ANIM_SPEED_DEATH; break;
                }
                break;
            case ENEMY_TYPE_1:
                 switch (new_state) {
                    case ENEMY_STATE_MOVING: e->current_sprites = gameState->sprites_enemy1_move; e->current_nb_frames = ENEMY_NBFRAMES_MOVE; e->tmpimg = ENEMY_ANIM_SPEED_MOVE; break;
                    case ENEMY_STATE_HIT:    e->current_sprites = gameState->sprites_enemy1_hit;  e->current_nb_frames = ENEMY_NBFRAMES_HIT;  e->tmpimg = ENEMY_ANIM_SPEED_HIT; e->state_timer = ENEMY_HIT_DURATION; break;
                    case ENEMY_STATE_DYING:  e->current_sprites = gameState->sprites_enemy1_death; e->current_nb_frames = ENEMY_NBFRAMES_DEATH; e->tmpimg = ENEMY_ANIM_SPEED_DEATH; break;
                }
                break;
            case ENEMY_TYPE_2:
                 switch (new_state) {
                    case ENEMY_STATE_MOVING: e->current_sprites = gameState->sprites_enemy2_move; e->current_nb_frames = ENEMY_NBFRAMES_MOVE; e->tmpimg = ENEMY_ANIM_SPEED_MOVE; break;
                    case ENEMY_STATE_HIT:    e->current_sprites = gameState->sprites_enemy2_hit;  e->current_nb_frames = ENEMY_NBFRAMES_HIT;  e->tmpimg = ENEMY_ANIM_SPEED_HIT; e->state_timer = ENEMY_HIT_DURATION; break;
                    case ENEMY_STATE_DYING:  e->current_sprites = gameState->sprites_enemy2_death; e->current_nb_frames = ENEMY_NBFRAMES_DEATH; e->tmpimg = ENEMY_ANIM_SPEED_DEATH; break;
                }
                break;
        }
        if (e->current_sprites && e->current_nb_frames > 0 && e->current_sprites[0]) {
            e->w = e->current_sprites[0]->w;
            e->h = e->current_sprites[0]->h;
        } else if (new_state != ENEMY_STATE_DYING) {
             if (e->w <=0) e->w = 30; // Utiliser les tailles de spawn_ennemi si pas de sprite
             if (e->h <=0) e->h = 30;
        }
    } else { // Pour les nouveaux types (3-6), pas de sprites pour l'instant
        if (e->w <=0) e->w = 30;
        if (e->h <=0) e->h = 30;
        if (new_state == ENEMY_STATE_HIT) e->state_timer = ENEMY_HIT_DURATION;
    }

    // --- MODIFICATION: Faire tomber un item si ENEMY_TYPE_5 passe à DYING ---
    if (new_state == ENEMY_STATE_DYING && old_state != ENEMY_STATE_DYING) { // S'assurer que c'est la première fois qu'on passe à DYING
        if (e->type == ENEMY_TYPE_5) {
            printf("Ennemi Type 5 (bleu) détruit, tentative de drop d'item Screen Clear.\n"); fflush(stdout);
            // L'item apparaît au centre de l'ennemi
            spawn_item(gameState, ITEM_TYPE_SCREEN_CLEAR, e->x + e->w / 2, e->y + e->h / 2);
        }
    }
    // --- FIN MODIFICATION ---
}

void spawn_ennemi(GameState *gameState, int type, int pos_x_monde, int pos_y_monde) {
    if (!gameState) return;

    if (type == ENEMY_TYPE_4) {
        int active_type4_count = 0;
        for (int i = 0; i < MAX_ENNEMIS; i++) {
            if (gameState->ennemis[i].active && gameState->ennemis[i].type == ENEMY_TYPE_4) {
                active_type4_count++;
            }
        }
        if (active_type4_count >= MAX_ACTIVE_ENEMY_TYPE_4) {
            return;
        }
    }

    int slot_trouve = -1;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        if (!gameState->ennemis[i].active) {
            slot_trouve = i;
            break;
        }
    }

    if (slot_trouve != -1) {
        Ennemi *e = &gameState->ennemis[slot_trouve];
        e->active = 1;
        e->type = type;
        e->x = pos_x_monde;
        e->y = pos_y_monde;
        e->state = -1;
        e->current_sprites = NULL;
        e->current_nb_frames = 0;
        // Les tailles par défaut sont définies ci-dessous par type, set_enemy_state les utilisera si pas de sprites
        e->w = 30; e->h = 30;

        switch (type) {
            case ENEMY_TYPE_0: e->health = ENEMY_HP_TYPE_0; e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_0; break;
            case ENEMY_TYPE_1: e->health = ENEMY_HP_TYPE_1; e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_1; break;
            case ENEMY_TYPE_2: e->health = ENEMY_HP_TYPE_2; e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_2; break;

            case ENEMY_TYPE_3:
                e->health = ENEMY_HP_TYPE_3;
                e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_3;
                e->speed_y = ENEMY_SPEED_Y_TYPE3;
                e->direction_y = (rand() % 2 == 0) ? 1 : -1;
                e->w = 32; e->h = 32;
                break;
            case ENEMY_TYPE_4:
                e->health = ENEMY_HP_TYPE_4;
                e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_4;
                e->speed_y = 0;
                e->direction_y = 0;
                e->w = 40; e->h = 40;
                break;
            case ENEMY_TYPE_5:
                e->health = ENEMY_HP_TYPE_5;
                e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_5;
                e->speed_y = 0;
                e->direction_y = 0;
                e->w = 28; e->h = 28;
                break;
            case ENEMY_TYPE_6:
                e->health = ENEMY_HP_TYPE_6;
                e->fire_interval = ENEMY_FIRE_INTERVAL_TYPE_6;
                e->speed_y = 0;
                e->direction_y = 0;
                e->w = 30; e->h = 30;
                break;
            default:
                printf("ERREUR: Type d'ennemi %d inconnu lors du spawn.\n", type); fflush(stdout);
                e->active = 0;
                return;
        }
        e->fire_timer = e->fire_interval / 2 + rand() % (e->fire_interval / 2);
        set_enemy_state(gameState, e, ENEMY_STATE_MOVING);
    }
}


void mettre_a_jour_ennemis(GameState *gameState) {
    if (!gameState) return;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        Ennemi *e = &gameState->ennemis[i];
        if (e->active) {
            int vitesse_x_propre = 0;

            switch (e->type) {
                case ENEMY_TYPE_0: vitesse_x_propre = ENEMY_SPEED_TYPE0; break;
                case ENEMY_TYPE_1: vitesse_x_propre = ENEMY_SPEED_TYPE1; break;
                case ENEMY_TYPE_2: vitesse_x_propre = ENEMY_SPEED_TYPE2; break;

                case ENEMY_TYPE_3:
                    vitesse_x_propre = -SCROLL_SPEED + ENEMY_SPEED_X_TYPE3_WORLD;
                    e->y += e->direction_y * e->speed_y;
                    if (e->y < 0) {
                        e->y = 0;
                        e->direction_y = 1;
                    } else if (e->y + e->h > gameState->screen_height_allegro) {
                        e->y = gameState->screen_height_allegro - e->h;
                        e->direction_y = -1;
                    }
                    break;
                case ENEMY_TYPE_4:
                    vitesse_x_propre = -SCROLL_SPEED;
                    break;
                case ENEMY_TYPE_5:
                    vitesse_x_propre = ENEMY_SPEED_X_TYPE5;
                    break;
                case ENEMY_TYPE_6:
                    vitesse_x_propre = ENEMY_SPEED_X_TYPE6;
                    break;
                default: vitesse_x_propre = 1;
            }
            e->x -= vitesse_x_propre;

            switch (e->state) {
                case ENEMY_STATE_MOVING:
                    e->fire_timer--;
                    if (e->fire_timer <= 0) {
                        e->fire_timer = e->fire_interval;

                        if (e->fire_interval < (TARGET_FPS * 900)) {
                            switch (e->type) {
                                case ENEMY_TYPE_0:
                                    for (int k = 0; k < ENEMY0_WALL_PROJECTILE_COUNT; k++) {
                                        int y_offset = (int)((k - (ENEMY0_WALL_PROJECTILE_COUNT - 1) / 2.0f) * ENEMY0_WALL_PROJECTILE_SPREAD);
                                        spawn_projectile_ennemi(gameState, e, y_offset, PROJECTILE_ENNEMI_SPEED_SLOW);
                                    }
                                    break;
                                case ENEMY_TYPE_1:
                                    spawn_projectile_ennemi(gameState, e, 0, PROJECTILE_ENNEMI_SPEED_FAST);
                                    break;
                                case ENEMY_TYPE_2:
                                    spawn_projectile_ennemi(gameState, e, (int)(-e->h * ENEMY2_SPREAD_FACTOR_VS_HEIGHT), PROJECTILE_ENNEMI_SPEED_MEDIUM);
                                    spawn_projectile_ennemi(gameState, e, (int)(e->h * ENEMY2_SPREAD_FACTOR_VS_HEIGHT), PROJECTILE_ENNEMI_SPEED_MEDIUM);
                                    break;
                                case ENEMY_TYPE_3:
                                    spawn_projectile_ennemi(gameState, e, 0, PROJECTILE_ENNEMI_SPEED_MEDIUM);
                                    break;
                                case ENEMY_TYPE_4:
                                    spawn_projectile_ennemi(gameState, e, 0, PROJECTILE_ENNEMI_SPEED_MEDIUM);
                                    break;
                                case ENEMY_TYPE_6:
                                    spawn_projectile_ennemi(gameState, e, 0, PROJECTILE_ENNEMI_SPEED_MEDIUM);
                                    break;
                            }
                        }
                    }
                    break;
                case ENEMY_STATE_HIT:
                    e->state_timer--;
                    if (e->state_timer <= 0) {
                        set_enemy_state(gameState, e, ENEMY_STATE_MOVING);
                    }
                    break;
                case ENEMY_STATE_DYING:
                    // La désactivation se fait plus bas si pas d'animation
                    // ou après l'animation pour les types 0-2
                    break;
            }

            // Mettre à jour l'animation (uniquement pour les types avec sprites)
            if (e->type >= ENEMY_TYPE_0 && e->type <= ENEMY_TYPE_2) {
                if (e->current_sprites && e->current_nb_frames > 0 && e->tmpimg > 0) {
                    e->cptimg++;
                    if (e->cptimg >= e->tmpimg) {
                        e->cptimg = 0;
                        e->imgcourante++;
                        if (e->imgcourante >= e->current_nb_frames) {
                            if (e->state == ENEMY_STATE_DYING) {
                                e->active = 0;
                            } else if (e->state == ENEMY_STATE_HIT) {
                                 e->imgcourante = e->current_nb_frames - 1;
                            } else {
                                e->imgcourante = 0;
                            }
                        }
                    }
                }
            } else {
                if (e->state == ENEMY_STATE_DYING) {
                    // Pour les types 3-6 sans animation de mort,
                    // le drop d'item est géré dans set_enemy_state.
                    // La désactivation effective de l'ennemi se fait ici.
                    e->active = 0;
                }
            }

            if (e->active && (e->x + e->w < gameState->scroll_x - e->w * 2)) {
                e->active = 0;
            }
        }
    }
}

void dessiner_ennemis(GameState *gameState) {
     if (!gameState || !gameState->buffer) return;
     BITMAP *dest = gameState->buffer;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        Ennemi *e = &gameState->ennemis[i];
        if (e->active) {
            int screen_x = e->x - gameState->scroll_x;
            int screen_y = e->y - gameState->scroll_y;

            if (screen_x + e->w > 0 && screen_x < gameState->screen_width_allegro &&
                screen_y + e->h > 0 && screen_y < gameState->screen_height_allegro) {

                if (e->current_sprites && e->imgcourante >= 0 && e->imgcourante < e->current_nb_frames && e->current_sprites[e->imgcourante]) {
                     draw_sprite(dest, e->current_sprites[e->imgcourante], screen_x, screen_y);
                } else {
                    int color = makecol(200, 200, 200);
                    switch(e->type) {
                        case ENEMY_TYPE_3: color = makecol(255, 165, 0); break;
                        case ENEMY_TYPE_4: color = makecol(128, 128, 128); break;
                        case ENEMY_TYPE_5: color = makecol(100, 149, 237); break; // Bleu pour l'ennemi qui drop l'item
                        case ENEMY_TYPE_6: color = makecol(148, 0, 211); break;
                        default: if (e->type >= ENEMY_TYPE_0 && e->type <= ENEMY_TYPE_2) color = makecol(255,0,0);
                    }
                    rectfill(dest, screen_x, screen_y, screen_x + e->w -1 , screen_y + e->h -1, color);
                }
            }
        }
    }
}

void nettoyer_ressources_ennemis(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources ennemis...\n"); fflush(stdout);
    void destroy_bitmap_array(BITMAP* arr[], int size) {
        if (!arr) return;
        for (int i = 0; i < size; i++) {
            if (arr[i]) {
                destroy_bitmap(arr[i]);
                arr[i] = NULL;
            }
        }
    }
    destroy_bitmap_array(gameState->sprites_enemy0_move, ENEMY_NBFRAMES_MOVE);
    destroy_bitmap_array(gameState->sprites_enemy0_hit, ENEMY_NBFRAMES_HIT);
    destroy_bitmap_array(gameState->sprites_enemy0_death, ENEMY_NBFRAMES_DEATH);
    destroy_bitmap_array(gameState->sprites_enemy1_move, ENEMY_NBFRAMES_MOVE);
    destroy_bitmap_array(gameState->sprites_enemy1_hit, ENEMY_NBFRAMES_HIT);
    destroy_bitmap_array(gameState->sprites_enemy1_death, ENEMY_NBFRAMES_DEATH);
    destroy_bitmap_array(gameState->sprites_enemy2_move, ENEMY_NBFRAMES_MOVE);
    destroy_bitmap_array(gameState->sprites_enemy2_hit, ENEMY_NBFRAMES_HIT);
    destroy_bitmap_array(gameState->sprites_enemy2_death, ENEMY_NBFRAMES_DEATH);
    printf("Sprites modèles ennemis (types 0-2) détruits.\n"); fflush(stdout);
}

