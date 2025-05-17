#include "defs.h"
#include "graphics.h"
#include "player.h"
#include "enemy.h"
#include "projectile.h"
#include "projectile_ennemi.h"
#include "hud.h"
#include "effect.h"
#include "menu.h"
#include "item.h" // NOUVEAU: Inclure la gestion des items

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <allegro.h>

// --- Forward Declarations ---
void cleanup_game_resources(GameState *gameState);
int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void gerer_collisions(GameState *gameState); // Sera modifié pour inclure les items
void gerer_collision_joueur_decor_obstacle(GameState *gameState);
void load_level_data(GameState *gameState, int level_number);

// --- Main Game Logic Functions ---

void cleanup_game_resources(GameState *gameState) {
    printf("--- Nettoyage des ressources du jeu ---\n"); fflush(stdout);
    if (!gameState) return;

    nettoyer_ressources_projectiles_joueur(gameState);
    nettoyer_ressources_projectiles_ennemi(gameState);
    nettoyer_ressources_items(gameState); // NOUVEAU
    nettoyer_ressources_effects(gameState);
    nettoyer_ressources_ennemis(gameState);
    nettoyer_ressources_joueur(gameState);
    nettoyer_ressources_hud(gameState);
    nettoyer_ressources_graphiques(gameState);

    printf("--- Nettoyage des ressources du jeu terminé ---\n"); fflush(stdout);
}

int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    if (x1 + w1 < x2 || x1 > x2 + w2 || y1 + h1 < y2 || y1 > y2 + h2) {
        return 0;
    }
    return 1;
}

// MODIFIÉ: Ajout de la collision Joueur vs Items et effet Screen Clear
void gerer_collisions(GameState *gameState) {
    if (!gameState) return;
    Vaisseau *player = &gameState->joueur;

    // 1. Projectiles Joueur vs Ennemis (inchangé)
    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        Projectile *p = &gameState->projectiles_joueur[i];
        if (!p->active || p->state != PROJECTILE_STATE_FLYING) continue;
        for (int j = 0; j < MAX_ENNEMIS; j++) {
            Ennemi *e = &gameState->ennemis[j];
            if (!e->active || e->state == ENEMY_STATE_DYING || e->state == ENEMY_STATE_HIT) continue;
            int ennemi_screen_x = e->x - gameState->scroll_x;
            int ennemi_screen_y = e->y - gameState->scroll_y;
            if (check_collision(p->x, p->y, p->w, p->h, ennemi_screen_x, ennemi_screen_y, e->w, e->h)) {
                int impact_x = p->x + p->w / 2;
                int impact_y = p->y + p->h / 2;
                spawn_impact_effect(gameState, impact_x, impact_y);
                p->state = PROJECTILE_STATE_IMPACTING;
                p->impact_frame = 0;
                p->impact_timer = 0;
                e->health -= 1;
                if (e->health <= 0) {
                    set_enemy_state(gameState, e, ENEMY_STATE_DYING); // set_enemy_state gère le drop d'item pour Type 5
                } else {
                    set_enemy_state(gameState, e, ENEMY_STATE_HIT);
                }
                goto next_projectile_collision_check_joueur; // Renommé pour éviter confusion
            }
        }
        next_projectile_collision_check_joueur:;
    }

    // 2. Joueur vs Ennemis (collision directe) (inchangé)
    if (player->active && player->state != STATE_DYING && player->state != STATE_HIT && player->state_timer <= 0) {
        for (int j = 0; j < MAX_ENNEMIS; j++) {
            Ennemi *e = &gameState->ennemis[j];
            if (!e->active || e->state == ENEMY_STATE_DYING) continue;
            int ennemi_screen_x = e->x - gameState->scroll_x;
            int ennemi_screen_y = e->y - gameState->scroll_y;
            if (check_collision(player->x, player->y, player->w, player->h, ennemi_screen_x, ennemi_screen_y, e->w, e->h)) {
                damage_player(gameState, 1);
                if (e->state != ENEMY_STATE_DYING) {
                    e->health = 0; // L'ennemi est aussi détruit au contact
                    set_enemy_state(gameState, e, ENEMY_STATE_DYING); // Gère le drop d'item pour Type 5
                }
                if (player->state == STATE_HIT || player->state == STATE_DYING) break;
            }
        }
    }

    // 3. Joueur vs Projectiles Ennemis (inchangé)
    if (player->active && player->state != STATE_DYING && player->state_timer <= 0) {
        for (int k = 0; k < MAX_PROJECTILES_ENNEMI; k++) {
            ProjectileEnnemi *pe = &gameState->projectiles_ennemi[k];
            if (!pe->active) continue;
            if (check_collision(player->x, player->y, player->w, player->h, pe->x, pe->y, pe->w, pe->h)) {
                damage_player(gameState, 1);
                pe->active = 0;
                if (player->state == STATE_HIT || player->state == STATE_DYING) break;
            }
        }
    }

    // 4. Joueur vs Items (NOUVEAU)
    if (player->active) { // Le joueur doit être actif pour collecter des items
        for (int i = 0; i < MAX_ITEMS; i++) {
            Item *item = &gameState->items[i];
            if (item->active) {
                int item_screen_x = item->x_world - gameState->scroll_x;
                int item_screen_y = item->y_world - gameState->scroll_y;

                if (check_collision(player->x, player->y, player->w, player->h, item_screen_x, item_screen_y, item->w, item->h)) {
                    // Collision joueur-item détectée
                    printf("Joueur a collecté l'item de type: %d\n", item->type); fflush(stdout);

                    if (item->type == ITEM_TYPE_SCREEN_CLEAR) {
                        printf("Effet SCREEN CLEAR activé!\n"); fflush(stdout);
                        // Détruire tous les ennemis à l'écran
                        for (int j = 0; j < MAX_ENNEMIS; j++) {
                            if (gameState->ennemis[j].active) {
                                // gameState->ennemis[j].active = 0; // Option simple
                                gameState->ennemis[j].health = 0; // Pour déclencher leur état DYING et potentiels drops
                                set_enemy_state(gameState, &gameState->ennemis[j], ENEMY_STATE_DYING);
                            }
                        }
                        // Détruire tous les projectiles ennemis à l'écran
                        for (int k = 0; k < MAX_PROJECTILES_ENNEMI; k++) {
                            if (gameState->projectiles_ennemi[k].active) {
                                gameState->projectiles_ennemi[k].active = 0;
                            }
                        }
                        // Optionnel: Détruire aussi les projectiles du joueur
                        /*
                        for (int k = 0; k < MAX_PROJECTILES_JOUEUR; k++) {
                            if (gameState->projectiles_joueur[k].active && gameState->projectiles_joueur[k].state == PROJECTILE_STATE_FLYING) {
                                // gameState->projectiles_joueur[k].active = 0;
                                // Ou les faire exploser
                                gameState->projectiles_joueur[k].state = PROJECTILE_STATE_IMPACTING;
                                gameState->projectiles_joueur[k].impact_frame = 0;
                                gameState->projectiles_joueur[k].impact_timer = 0;
                            }
                        }
                        */
                    }
                    // Autres types d'items pourraient être gérés ici avec un switch(item->type)

                    item->active = 0; // Désactiver l'item collecté
                    // Pas besoin de 'break' ici, le joueur pourrait théoriquement collecter plusieurs items en une frame
                }
            }
        }
    }
}


void gerer_collision_joueur_decor_obstacle(GameState *gameState) {
    // ... (code inchangé, toujours commenté dans main si non désiré) ...
    if (!gameState || !gameState->joueur.active || !gameState->decor_obstacles) return;
    Vaisseau *player = &gameState->joueur;

    if (player->state == STATE_HIT || player->state == STATE_DYING || player->state_timer > 0) {
        return;
    }

    typedef struct { int x; int y; } Point;
    Point points_a_tester[] = {
        {player->w -1, player->h / 2}, {player->w -1, player->h / 4}, {player->w -1, player->h * 3/4},
        {player->w / 2, 0}, {player->w / 2, player->h -1},
        {player->w * 3/4, player->h / 4}, {player->w * 3/4, player->h * 3/4},
    };
    int nb_points = sizeof(points_a_tester) / sizeof(points_a_tester[0]);

    for (int i = 0; i < nb_points; i++) {
        int player_test_screen_x = player->x + points_a_tester[i].x;
        int player_test_screen_y = player->y + points_a_tester[i].y;
        int decor_collision_coord_x = player_test_screen_x + gameState->scroll_x;
        int decor_collision_coord_y = player_test_screen_y;

        if (gameState->decor_obstacles->w == 0 || gameState->decor_obstacles->h == 0) return;

        int pixel_x_in_obstacle_bmp = decor_collision_coord_x % gameState->decor_obstacles->w;
        if (pixel_x_in_obstacle_bmp < 0) pixel_x_in_obstacle_bmp += gameState->decor_obstacles->w;

        int pixel_y_in_obstacle_bmp = decor_collision_coord_y;

        if (pixel_x_in_obstacle_bmp >= 0 && pixel_x_in_obstacle_bmp < gameState->decor_obstacles->w &&
            pixel_y_in_obstacle_bmp >= 0 && pixel_y_in_obstacle_bmp < gameState->decor_obstacles->h) {
            int pixel_color = getpixel(gameState->decor_obstacles, pixel_x_in_obstacle_bmp, pixel_y_in_obstacle_bmp);
            if (pixel_color != gameState->couleur_magenta) {
                damage_player(gameState, player->health);
                return;
            }
        }
    }
}


void load_level_data(GameState *gameState, int level_number) {
    if (!gameState) return;
    printf("Chargement des données du Niveau %d pour %s...\n", level_number, gameState->player_name);
    fflush(stdout);
    gameState->current_level = level_number;

    char decor_visuel_path[100];
    char decor_obstacles_path[100];

    if (gameState->decor_visuel) {
        destroy_bitmap(gameState->decor_visuel);
        gameState->decor_visuel = NULL;
    }
    if (gameState->decor_obstacles) {
        destroy_bitmap(gameState->decor_obstacles);
        gameState->decor_obstacles = NULL;
    }

    if (level_number == 2) {
        sprintf(decor_visuel_path, "background_visual_2.bmp");
        sprintf(decor_obstacles_path, "background_obstacle_2.bmp");
    } else {
        sprintf(decor_visuel_path, "background_visual_1.bmp");
        sprintf(decor_obstacles_path, "background_obstacle_1.bmp");
    }

    printf("Chargement decor visuel: %s\n", decor_visuel_path); fflush(stdout);
    gameState->decor_visuel = charger_bitmap_safe(decor_visuel_path);
    printf("Chargement decor obstacles: %s\n", decor_obstacles_path); fflush(stdout);
    gameState->decor_obstacles = charger_bitmap_safe(decor_obstacles_path);

    if (!gameState->decor_visuel || !gameState->decor_obstacles) {
         allegro_message("Erreur: Impossible de charger les décors pour le niveau %d.", level_number);
         gameState->current_game_state = GAME_STATE_MENU;
         if (init_menu_resources(gameState) != 0) {
            gameState->current_game_state = GAME_STATE_EXITING;
         }
         return;
    }

    gameState->scroll_x = 0;
    gameState->scroll_y = 0;
    gameState->game_loop_counter = 0;
    gameState->hud_displayed_level_progress = 0;

    init_player_state(gameState);
    initialiser_ennemis_array(gameState);
    initialiser_projectiles_joueur(gameState);
    initialiser_projectiles_ennemi(gameState);
    initialiser_effects(gameState);
    initialiser_items(gameState); // NOUVEAU: Initialiser les items

    gameState->can_shoot = 1;
    gameState->shoot_timer = 0;

    printf("Niveau %d prêt.\n", level_number);
    fflush(stdout);
    gameState->current_game_state = GAME_STATE_PLAYING;
}

// --- Point d'Entrée Principal ---
int main() {
    GameState gameState;
    memset(&gameState, 0, sizeof(GameState));
    gameState.return_to_map_select = 0;

    printf("Démarrage du jeu...\n"); fflush(stdout);
    srand(time(NULL));

    if (initialisation_allegro(&gameState) != 0) {
        return 1;
    }
    printf("Allegro initialisé.\n"); fflush(stdout);

    printf("Chargement des ressources globales du jeu (sprites, etc.)...\n"); fflush(stdout);
    load_player_animations(&gameState);
    charger_sprites_ennemis(&gameState);
    charger_et_redimensionner_sprite_projectile_joueur(&gameState);
    charger_sprite_projectile_ennemi(&gameState);
    charger_sprites_items(&gameState); // NOUVEAU: Charger les sprites des items (même si vide pour l'instant)
    charger_sprites_effects(&gameState);
    charger_ressources_hud(&gameState);

    printf("Ressources globales du jeu chargées (sauf décors de niveau).\n"); fflush(stdout);

    gameState.current_game_state = GAME_STATE_MENU;
    if (init_menu_resources(&gameState) != 0) {
        allegro_message("Impossible d'initialiser les ressources du menu. Sortie.");
        cleanup_game_resources(&gameState);
        allegro_exit();
        return 1;
    }

    int shoot_delay = 12;
    int spawn_timer = 0;
    int spawn_interval = 75;

    printf("Entrée dans la boucle principale...\n"); fflush(stdout);
    while (gameState.current_game_state != GAME_STATE_EXITING) {

        switch (gameState.current_game_state) {
            case GAME_STATE_MENU:
                run_game_menu(&gameState);
                break;

            case GAME_STATE_LOAD_LEVEL:
                load_level_data(&gameState, gameState.current_level);
                break;

            case GAME_STATE_PLAYING:
                gameState.game_loop_counter++;

                if (key[KEY_SPACE] && gameState.can_shoot && gameState.joueur.active && gameState.joueur.state != STATE_DYING) {
                    int tir_start_x = gameState.joueur.x + gameState.joueur.w;
                    int tir_start_y = gameState.joueur.y + gameState.joueur.h / 2 - (gameState.sprite_projectile_joueur ? gameState.sprite_projectile_joueur->h / 2 : 0);
                    spawn_projectile_joueur(&gameState, tir_start_x, tir_start_y);
                    gameState.can_shoot = 0;
                    gameState.shoot_timer = 0;
                }
                if (!gameState.can_shoot) {
                    gameState.shoot_timer++;
                    if (gameState.shoot_timer >= shoot_delay) gameState.can_shoot = 1;
                }
                if (key[KEY_ESC]) {
                     printf("ESC en jeu: Retour au menu principal...\n");
                     gameState.current_game_state = GAME_STATE_MENU;
                     gameState.return_to_map_select = 0;
                     if (init_menu_resources(&gameState) != 0) {
                         gameState.current_game_state = GAME_STATE_EXITING;
                     }
                     clear_keybuf();
                     continue;
                }

                mettre_a_jour_scrolling(&gameState);
                mettre_a_jour_joueur(&gameState);
                mettre_a_jour_ennemis(&gameState);
                mettre_a_jour_projectiles_joueur(&gameState);
                mettre_a_jour_projectiles_ennemi(&gameState);
                mettre_a_jour_items(&gameState); // NOUVEAU: Mettre à jour les items
                mettre_a_jour_effects(&gameState);
                mettre_a_jour_hud(&gameState);

                spawn_timer++;
                if (spawn_timer >= spawn_interval) {
                    spawn_timer = 0;
                    if (rand() % 100 < 70) {
                        int type_ennemi_a_spawner;
                        if (gameState.current_level == 2) {
                            type_ennemi_a_spawner = ENEMY_TYPE_3 + (rand() % 4);
                        } else {
                            type_ennemi_a_spawner = rand() % 3;
                        }

                        int enemy_width_placeholder = 40;
                        int enemy_height_placeholder = 30;

                        if (type_ennemi_a_spawner <= ENEMY_TYPE_2) {
                             if(gameState.sprites_enemy0_move[0]) {
                                 enemy_height_placeholder = gameState.sprites_enemy0_move[0]->h;
                                 enemy_width_placeholder = gameState.sprites_enemy0_move[0]->w;
                             }
                        } else {
                            switch(type_ennemi_a_spawner) {
                                case ENEMY_TYPE_3: enemy_width_placeholder = 32; enemy_height_placeholder = 32; break;
                                case ENEMY_TYPE_4: enemy_width_placeholder = 40; enemy_height_placeholder = 40; break;
                                case ENEMY_TYPE_5: enemy_width_placeholder = 28; enemy_height_placeholder = 28; break;
                                case ENEMY_TYPE_6: enemy_width_placeholder = 30; enemy_height_placeholder = 30; break;
                            }
                        }

                        int pos_x_ennemi = gameState.scroll_x + gameState.screen_width_allegro - enemy_width_placeholder - (rand() % 30);

                        int pos_y_ennemi = 30 + rand() % (gameState.screen_height_allegro - 60 - enemy_height_placeholder );
                        if (pos_y_ennemi < 0) pos_y_ennemi = 0;
                        if (pos_y_ennemi + enemy_height_placeholder > gameState.screen_height_allegro) {
                            pos_y_ennemi = gameState.screen_height_allegro - enemy_height_placeholder;
                        }

                        spawn_ennemi(&gameState, type_ennemi_a_spawner, pos_x_ennemi, pos_y_ennemi);
                    }
                }

                // gerer_collision_joueur_decor_obstacle(&gameState); // APPEL COMMENTÉ
                if (gameState.joueur.active) {
                    gerer_collisions(&gameState); // Gère maintenant aussi Joueur vs Items
                }

                if (gameState.joueur.active && gameState.game_loop_counter >= LEVEL_DURATION_FRAMES) {
                    gameState.current_game_state = GAME_STATE_LEVEL_COMPLETE;
                }
                break;

            case GAME_STATE_LEVEL_COMPLETE:
                {
                    clear_to_color(gameState.buffer, makecol(0,0,20));
                    textprintf_centre_ex(gameState.buffer, font,
                                         gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2 - 20,
                                         makecol(100,255,100), -1,
                                         "Niveau %d Termine !", gameState.current_level);
                    textout_centre_ex(gameState.buffer, font,
                                      "Appuyez sur ENTREE pour continuer",
                                      gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2 + 20,
                                      makecol(200,200,200), -1);
                    blit(gameState.buffer, screen, 0,0,0,0,gameState.screen_width_allegro, gameState.screen_height_allegro);

                    rest(100);
                    clear_keybuf();

                    int wait_for_input = 1;
                    while(wait_for_input){
                        if(key[KEY_ENTER] || key[KEY_ESC]){
                            wait_for_input = 0;
                        }
                        rest(10);
                    }
                    clear_keybuf();

                    printf("Niveau %d terminé! Passage à la suite.\n", gameState.current_level);
                    menu_update_and_save_progress(&gameState, gameState.current_level, 1);

                    if (gameState.current_level == MAX_LEVEL) {
                        gameState.current_game_state = GAME_STATE_GAME_WON;
                    } else {
                        gameState.return_to_map_select = 1;
                        gameState.current_game_state = GAME_STATE_MENU;

                        if (init_menu_resources(&gameState) != 0) {
                            gameState.current_game_state = GAME_STATE_EXITING;
                        }
                    }
                }
                break;

            case GAME_STATE_GAME_OVER:
                clear_to_color(gameState.buffer, makecol(0,0,0));
                textout_centre_ex(gameState.buffer, font, "GAME OVER", gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2 - 40, makecol(255,50,50), -1);
                textprintf_centre_ex(gameState.buffer, font, gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2, makecol(200,200,200), -1, "Joueur: %s", gameState.player_name);
                textout_centre_ex(gameState.buffer, font, "Appuyez sur ENTREE pour retourner au menu", gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2 + 40, makecol(200,200,200), -1);
                blit(gameState.buffer, screen, 0,0,0,0,gameState.screen_width_allegro, gameState.screen_height_allegro);
                rest(100);
                clear_keybuf();

                while(1){
                    if(key[KEY_ENTER] || key[KEY_ESC]){
                        gameState.current_game_state = GAME_STATE_MENU;
                        gameState.return_to_map_select = 0;
                        if (init_menu_resources(&gameState) != 0) {
                             gameState.current_game_state = GAME_STATE_EXITING;
                        }
                        rest(100);
                        clear_keybuf();
                        break;
                    }
                    rest(10);
                }
                break;

            case GAME_STATE_GAME_WON:
                clear_to_color(gameState.buffer, makecol(20,20,80));
                textout_centre_ex(gameState.buffer, font, "VICTOIRE !", gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2 - 40, makecol(50,255,50), -1);
                textprintf_centre_ex(gameState.buffer, font, gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2, makecol(200,200,200), -1, "Bravo %s !", gameState.player_name);
                textout_centre_ex(gameState.buffer, font, "Appuyez sur ENTREE pour retourner au menu", gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2 + 40, makecol(200,200,200), -1);
                blit(gameState.buffer, screen, 0,0,0,0,gameState.screen_width_allegro, gameState.screen_height_allegro);
                rest(100);
                clear_keybuf();

                while(1){
                     if(key[KEY_ENTER] || key[KEY_ESC]){
                        gameState.current_game_state = GAME_STATE_MENU;
                        gameState.return_to_map_select = 0;
                        if (init_menu_resources(&gameState) != 0) {
                             gameState.current_game_state = GAME_STATE_EXITING;
                        }
                        rest(100);
                        clear_keybuf();
                        break;
                    }
                    rest(10);
                }
                break;

            default:
                printf("Etat de jeu inconnu: %d. Sortie.\n", gameState.current_game_state);
                gameState.current_game_state = GAME_STATE_EXITING;
                break;
        }

        if (gameState.current_game_state == GAME_STATE_PLAYING) {
            clear_bitmap(gameState.buffer);

            dessiner_decor_layer(&gameState, gameState.decor_visuel);

            dessiner_items(&gameState); // NOUVEAU: Dessiner les items
            dessiner_ennemis(&gameState);
            dessiner_projectiles_joueur(&gameState);
            dessiner_projectiles_ennemi(&gameState);
            dessiner_effects(&gameState);
            dessiner_joueur(&gameState);
            dessiner_hud(&gameState);
        }

        if (gameState.current_game_state == GAME_STATE_MENU ||
            gameState.current_game_state == GAME_STATE_PLAYING ||
            gameState.current_game_state == GAME_STATE_LEVEL_COMPLETE ) {
            blit(gameState.buffer, screen, 0, 0, 0, 0, gameState.screen_width_allegro, gameState.screen_height_allegro);
        }

        rest(TARGET_FPS > 0 ? (1000 / TARGET_FPS) : 15);
    }

    printf("Sortie de la boucle principale (État final: %d).\n", gameState.current_game_state);

    if (gameState.current_game_state == GAME_STATE_MENU || gameState.quit_game) {
        deinit_menu_resources();
    }
    if (gameState.decor_visuel || (PLAYER_NBFRAMES_IDLE_MOVE > 0 && gameState.sprites_player_idle_move[1] != NULL) ) {
        cleanup_game_resources(&gameState);
    }

    allegro_exit();
    printf("Allegro arrêté.\n");
    printf("Programme terminé.\n");
    fflush(stdout);
    return 0;
}
END_OF_MAIN();

