#include "defs.h"
#include "graphics.h"
#include "player.h"
#include "enemy.h"
#include "projectile.h"
#include "projectile_ennemi.h"
#include "hud.h"
#include "effect.h"
#include "menu.h"
#include "item.h"
#include "boss.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <allegro.h>

// proto de fonction
void cleanup_game_resources(GameState *gameState);
int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void gerer_collisions(GameState *gameState);
void gerer_collision_joueur_decor_obstacle(GameState *gameState);
void load_level_data(GameState *gameState, int level_number);

// logique du jeu dans les fonctions suivante :

void cleanup_game_resources(GameState *gameState) {
    printf("--- Nettoyage des ressources du jeu ---\n"); fflush(stdout);
    if (!gameState) return;

    nettoyer_ressources_projectiles_joueur(gameState);
    nettoyer_ressources_projectiles_ennemis(gameState);
    nettoyer_ressources_items(gameState);
    nettoyer_ressources_effects(gameState);
    nettoyer_ressources_ennemis(gameState);
    nettoyer_ressources_boss(gameState);
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

void gerer_collisions(GameState *gameState) {
    if (!gameState) return;
    Vaisseau *player = &gameState->joueur;
    Boss *boss = &gameState->boss; // Pointeur vers le boss

    // 1. Projectiles Joueur vs Ennemis
    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        Projectile *p = &gameState->projectiles_joueur[i];
        if (!p->active || p->state != PROJECTILE_STATE_FLYING) continue;
        for (int j = 0; j < MAX_ENNEMIS; j++) {
            Ennemi *e = &gameState->ennemis[j];
            if (!e->active || e->state == ENEMY_STATE_DYING || e->state == ENEMY_STATE_HIT) continue;
            int ennemi_screen_x = e->x - gameState->scroll_x;
            int ennemi_screen_y = e->y - gameState->scroll_y_dummy; // Correction: scroll_y_dummy -> scroll_y (même si scroll_y est 0)
            if (check_collision(p->x, p->y, p->w, p->h, ennemi_screen_x, ennemi_screen_y, e->w, e->h)) {
                // ... (logique de collision projectile joueur vs ennemi existante) ...
                int impact_x = p->x + p->w / 2;
                int impact_y = p->y + p->h / 2;
                spawn_impact_effect(gameState, impact_x, impact_y);
                p->state = PROJECTILE_STATE_IMPACTING;
                p->impact_frame = 0;
                p->impact_timer = 0;
                e->health -= 1;
                if (e->health <= 0) {
                    set_enemy_state(gameState, e, ENEMY_STATE_DYING);
                } else {
                    set_enemy_state(gameState, e, ENEMY_STATE_HIT);
                }
                goto next_projectile_collision_j_vs_e;
            }
        }
        next_projectile_collision_j_vs_e:;

        //  Projectiles Joueur vs Boss
        if (boss->active && boss->current_phase == BOSS_PHASE_2_VULNERABLE) { // Boss ne prend des dégâts qu'en phase 2
            // Les coordonnées du boss sont déjà des coordonnées écran car il ne scrolle pas
            if (check_collision(p->x, p->y, p->w, p->h, boss->x, boss->y, boss->w, boss->h)) {
                printf("Boss touché par projectile joueur! HP Boss: %d -> ", boss->health);
                int impact_x = p->x + p->w / 2;
                int impact_y = p->y + p->h / 2;
                spawn_impact_effect(gameState, impact_x, impact_y); // Effet sur le boss
                p->state = PROJECTILE_STATE_IMPACTING;
                p->impact_frame = 0;
                p->impact_timer = 0;

                boss->health -= 1; // Dégâts au boss
                printf("%d\n", boss->health); fflush(stdout);
                // La logique de changement de phase du boss ou de mort est dans mettre_a_jour_boss
            }
        }
    }


    // 2. Joueur vs Ennemis (collision directe)
    if (player->active && player->state != STATE_DYING && player->state != STATE_HIT && player->state_timer <= 0) {
        for (int j = 0; j < MAX_ENNEMIS; j++) {
            Ennemi *e = &gameState->ennemis[j];
            if (!e->active || e->state == ENEMY_STATE_DYING) continue;
            int ennemi_screen_x = e->x - gameState->scroll_x;
            int ennemi_screen_y = e->y - gameState->scroll_y_dummy;
            if (check_collision(player->x, player->y, player->w, player->h, ennemi_screen_x, ennemi_screen_y, e->w, e->h)) {
                damage_player(gameState, 1);
                if (e->state != ENEMY_STATE_DYING) {
                    e->health = 0;
                    set_enemy_state(gameState, e, ENEMY_STATE_DYING);
                }
                if (player->state == STATE_HIT || player->state == STATE_DYING) break;
            }
        }
        // NOUVEAU: Joueur vs Boss (collision directe)
        if (boss->active) {
             if (check_collision(player->x, player->y, player->w, player->h, boss->x, boss->y, boss->w, boss->h)) {
                damage_player(gameState, player->health); // Collision avec boss = mort instantanée pour le joueur
                // Le boss ne prend pas de dégâts par collision directe pour l'instant
             }
        }
    }

    // 3. Joueur vs Projectiles Ennemis
    if (player->active && player->state != STATE_DYING && player->state_timer <= 0 ) {
        for (int k = 0; k < MAX_PROJECTILES_ENNEMI; k++) {
            ProjectileEnnemi *pe = &gameState->projectiles_ennemi[k];
            if (!pe->active) continue;
            if (check_collision(player->x, player->y, player->w, player->h, pe->x, pe->y, pe->w, pe->h)) {
                damage_player(gameState, pe->damage);
                pe->active = 0;
                if (player->state == STATE_HIT || player->state == STATE_DYING) break;
            }
        }
    }

    // 4. Joueur vs Items
    if (player->active) {
        for (int i = 0; i < MAX_ITEMS; i++) {
            Item *item = &gameState->items[i];
            if (item->active) {
                int item_screen_x = item->x_world - gameState->scroll_x;
                int item_screen_y = item->y_world - gameState->scroll_y_dummy;

                if (check_collision(player->x, player->y, player->w, player->h, item_screen_x, item_screen_y, item->w, item->h)) {
                    printf("Joueur a collecté l'item de type: %d\n", item->type); fflush(stdout);

                    if (item->type == ITEM_TYPE_SCREEN_CLEAR) {
                        printf("Effet SCREEN CLEAR activé!\n"); fflush(stdout);
                        for (int j = 0; j < MAX_ENNEMIS; j++) {
                            if (gameState->ennemis[j].active) {
                                gameState->ennemis[j].health = 0;
                                set_enemy_state(gameState, &gameState->ennemis[j], ENEMY_STATE_DYING);
                            }
                        }
                        for (int k = 0; k < MAX_PROJECTILES_ENNEMI; k++) {
                            if (gameState->projectiles_ennemi[k].active) {
                                gameState->projectiles_ennemi[k].active = 0;
                            }
                        }
                    } else if (item->type == ITEM_TYPE_HEALTH_PACK) {
                        printf("Pack de Santé collecté !\n"); fflush(stdout);
                        if (gameState->joueur.health < PLAYER_INITIAL_HP) {
                            gameState->joueur.health++;
                            printf("PV du joueur augmentés à %d\n", gameState->joueur.health); fflush(stdout);
                        } else {
                            printf("PV du joueur déjà au max.\n"); fflush(stdout);
                        }
                    }
                    item->active = 0;
                }
            }
        }
    }
}


void gerer_collision_joueur_decor_obstacle(GameState *gameState) {
    // ... (code inchangé) ...
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
    // ... (code inchangé, s'assure que boss_fight_active est à 0) ...
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
    gameState->scroll_y_dummy = 0;
    gameState->game_loop_counter = 0;
    gameState->hud_displayed_level_progress = 0;
    gameState->boss_fight_active = 0; // NOUVEAU: S'assurer que le combat de boss n'est pas actif au début d'un niveau

    init_player_state(gameState);
    initialiser_ennemis_array(gameState);
    initialiser_projectiles_joueur(gameState);
    initialiser_projectiles_ennemi(gameState);
    initialiser_effects(gameState);
    initialiser_items(gameState);
    // initialiser_boss(gameState); // Le boss est initialisé une fois, puis spawn_boss est appelé

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
    gameState.boss_fight_active = 0; // Initialiser le drapeau du boss

    printf("Démarrage du jeu...\n"); fflush(stdout);
    srand(time(NULL));

    if (initialisation_allegro(&gameState) != 0) {
        return 1;
    }
    printf("Allegro initialisé.\n"); fflush(stdout);

    printf("Chargement des ressources globales du jeu (sprites, etc.)...\n"); fflush(stdout);
    load_player_animations(&gameState);
    charger_sprites_ennemis(&gameState);
    charger_sprites_projectiles_ennemis(&gameState);
    charger_et_redimensionner_sprite_projectile_joueur(&gameState);
    charger_sprites_items(&gameState);
    charger_sprites_effects(&gameState);
    charger_ressources_hud(&gameState);
    charger_sprites_boss(&gameState); // NOUVEAU: Charger les sprites du boss
    initialiser_boss(&gameState);     // NOUVEAU: Initialiser la structure du boss

    printf("Ressources globales du jeu chargées.\n"); fflush(stdout);

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
    int current_scroll_speed = SCROLL_SPEED; // Pour pouvoir arrêter le scrolling

    printf("Entrée dans la boucle principale...\n"); fflush(stdout);
    while (gameState.current_game_state != GAME_STATE_EXITING) {

        // Gérer l'arrêt du scrolling pour le combat de boss
        if (gameState.boss_fight_active) {
            current_scroll_speed = 0;
        } else {
            current_scroll_speed = SCROLL_SPEED;
        }


        switch (gameState.current_game_state) {
            case GAME_STATE_MENU:
                run_game_menu(&gameState);
                break;

            case GAME_STATE_LOAD_LEVEL:
                gameState.boss_fight_active = 0; // S'assurer que le combat de boss est désactivé lors du chargement d'un niveau
                current_scroll_speed = SCROLL_SPEED; // Rétablir la vitesse de scrolling
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

                if (!gameState.boss_fight_active) { // Ne pas mettre à jour le scrolling si combat de boss
                    mettre_a_jour_scrolling(&gameState); // Utilise SCROLL_SPEED de defs.h
                } else {
                    // Le scrolling est arrêté, mais on pourrait vouloir des effets de "tremblement" ou autre.
                    // Pour l'instant, on ne fait rien de plus que de ne pas appeler mettre_a_jour_scrolling.
                }

                mettre_a_jour_joueur(&gameState);

                if (gameState.boss_fight_active) {
                    mettre_a_jour_boss(&gameState);
                } else {
                    mettre_a_jour_ennemis(&gameState); // Mettre à jour les ennemis normaux si pas de combat de boss
                }

                mettre_a_jour_projectiles_joueur(&gameState);
                mettre_a_jour_projectiles_ennemi(&gameState);
                mettre_a_jour_items(&gameState);
                mettre_a_jour_effects(&gameState);
                mettre_a_jour_hud(&gameState);

                // Logique de spawn des ennemis (uniquement si pas de combat de boss)
                if (!gameState.boss_fight_active) {
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
                }

                // gerer_collision_joueur_decor_obstacle(&gameState); // APPEL COMMENTÉ
                if (gameState.joueur.active) {
                    gerer_collisions(&gameState);
                }

                // Condition de fin de niveau ou d'apparition du boss
                if (gameState.joueur.active) {
                    if (gameState.current_level == MAX_LEVEL && !gameState.boss_fight_active && gameState.game_loop_counter >= LEVEL_DURATION_FRAMES) {
                        // Fin du temps du niveau 3, faire apparaître le boss
                        printf("Temps du niveau 3 écoulé. Apparition du BOSS !\n");
                        spawn_boss(&gameState); // spawn_boss mettra gameState.boss_fight_active à 1
                        // Arrêter le spawn des ennemis normaux (déjà géré par la condition !gameState.boss_fight_active plus haut)
                        // Le scrolling s'arrêtera grâce au drapeau boss_fight_active
                    } else if (gameState.current_level == MAX_LEVEL && gameState.boss_fight_active && !gameState.boss.active) {
                        // Le boss a été vaincu
                        printf("Boss vaincu! Niveau 3 terminé.\n");
                        gameState.current_game_state = GAME_STATE_LEVEL_COMPLETE; // Mènera à GAME_WON
                    } else if (gameState.current_level != MAX_LEVEL && gameState.game_loop_counter >= LEVEL_DURATION_FRAMES) {
                        // Fin du temps pour les niveaux 1 ou 2
                        gameState.current_game_state = GAME_STATE_LEVEL_COMPLETE;
                    }
                }
                break;

            //  (autres cases GAME_STATE_ si y'en a
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

                    if (gameState.current_level == MAX_LEVEL) { // Si c'était le niveau du boss et qu'il est vaincu
                        gameState.current_game_state = GAME_STATE_GAME_WON;
                    } else { // Pour les niveaux 1 et 2
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

            dessiner_items(&gameState);
            if (gameState.boss_fight_active) {
                dessiner_boss(&gameState);
            } else {
                dessiner_ennemis(&gameState);
            }
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

