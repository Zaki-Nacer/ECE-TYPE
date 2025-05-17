#include "boss.h"
#include "defs.h"
#include "graphics.h" // Pour charger_bitmap_safe
#include "projectile_ennemi.h" // Pour que le boss puisse tirer
#include <stdio.h>
#include <stdlib.h> // Pour rand()

// --- Fonctions du Boss ---

void charger_sprites_boss(GameState *gameState) {
    if (!gameState) return;
    // printf("Chargement des sprites du boss...\n");

    gameState->boss.sprite_phase1 = charger_bitmap_safe("boss_phase1.bmp");
    gameState->boss.sprite_phase2 = charger_bitmap_safe("boss_phase2.bmp");

    if (!gameState->boss.sprite_phase1 || !gameState->boss.sprite_phase2) {
        allegro_message("ERREUR: Impossible de charger un ou plusieurs sprites du boss!");
        if (gameState->boss.sprite_phase1) destroy_bitmap(gameState->boss.sprite_phase1);
        if (gameState->boss.sprite_phase2) destroy_bitmap(gameState->boss.sprite_phase2);
        gameState->boss.sprite_phase1 = NULL;
        gameState->boss.sprite_phase2 = NULL;
        // On pourrait quitter ici si c'est critique
    } else {
        // printf("Sprites du boss chargés.\n");
        gameState->boss.w = gameState->boss.sprite_phase1->w;
        gameState->boss.h = gameState->boss.sprite_phase1->h;
    }
    gameState->boss.current_sprite = gameState->boss.sprite_phase1; // Sprite par défaut
}

void initialiser_boss(GameState *gameState) {
    if (!gameState) return;
    // printf("Initialisation du boss...\n");
    Boss *boss = &gameState->boss;

    boss->active = 0; // Inactif au début
    boss->health = BOSS_INITIAL_HP;
    boss->max_health = BOSS_INITIAL_HP;
    boss->current_phase = BOSS_PHASE_NONE;
    boss->phase1_attack_counter = 0;
    boss->phase1_attack_timer = 0;
    boss->phase2_attack_timer = 0;
    boss->has_triggered_mid_phase_switch = 0;

    if (boss->sprite_phase1) {
        boss->current_sprite = boss->sprite_phase1;
        boss->w = boss->sprite_phase1->w;
        boss->h = boss->sprite_phase1->h;
    } else { // Si les sprites ne sont pas là
        boss->w = 150;
        boss->h = 200;
        boss->current_sprite = NULL;
    }
    boss->x = gameState->screen_width_allegro; // Position hors écran au début
    boss->y = gameState->screen_height_allegro / 2 - boss->h / 2;
}

void spawn_boss(GameState *gameState) {
    if (!gameState) return;
    Boss *boss = &gameState->boss;

    if (boss->active) {
        // printf("Boss déjà actif, spawn annulé.\n");
        return;
    }
    printf("Apparition du BOSS !!!\n");

    boss->active = 1;
    boss->health = BOSS_INITIAL_HP;
    boss->max_health = BOSS_INITIAL_HP;
    boss->current_phase = BOSS_PHASE_1_ATTACKING;
    boss->current_sprite = boss->sprite_phase1;
    if (boss->current_sprite) {
        boss->w = boss->current_sprite->w;
        boss->h = boss->current_sprite->h;
    } else {
        boss->w = 150; boss->h = 200; // Dimensions par défaut
        // printf("AVERTISSEMENT: Boss spawné sans sprite pour la phase 1.\n");
    }

    boss->x = gameState->screen_width_allegro - boss->w - 20; // À droite de l'écran
    boss->y = gameState->screen_height_allegro / 2 - boss->h / 2; // Centré verticalement

    boss->phase1_attack_counter = 0;
    boss->phase1_attack_timer = BOSS_PHASE1_ATTACK_INTERVAL;
    boss->phase2_attack_timer = 0;
    boss->has_triggered_mid_phase_switch = 0;

    gameState->boss_fight_active = 1;
    // printf("Boss spawné à (%d, %d) avec %d PV, phase %d.\n", boss->x, boss->y, boss->health, boss->current_phase);
}

void mettre_a_jour_boss(GameState *gameState) {
    if (!gameState || !gameState->boss.active) return;
    Boss *boss = &gameState->boss;

    if (boss->health <= 0) {
        printf("BOSS VAINCU !\n");
        boss->active = 0;
        gameState->boss_fight_active = 0;
        // La suite (écran de victoire, etc.) est gérée ailleurs
        return;
    }

    // Si PV à moitié et en phase 2 vulnérable, on repasse en phase 1
    if (boss->health <= BOSS_HP_PHASE_2_TRIGGER && !boss->has_triggered_mid_phase_switch && boss->current_phase == BOSS_PHASE_2_VULNERABLE) {
        // printf("Boss: Retour à la Phase 1 (PV à moitié) !\n");
        boss->current_phase = BOSS_PHASE_1_ATTACKING;
        boss->current_sprite = boss->sprite_phase1;
        boss->phase1_attack_counter = 0;
        boss->phase1_attack_timer = BOSS_PHASE1_ATTACK_INTERVAL;
        boss->has_triggered_mid_phase_switch = 1;
    }

    switch (boss->current_phase) {
        case BOSS_PHASE_1_ATTACKING:
            // Invulnérable (géré dans les collisions)
            boss->phase1_attack_timer--;
            if (boss->phase1_attack_timer <= 0 && boss->phase1_attack_counter < BOSS_PHASE1_ATTACK_COUNT) {
                // printf("Boss: Attaque Mur de projectiles #%d\n", boss->phase1_attack_counter + 1);
                int gap_start_y_index = rand() % (gameState->screen_height_allegro / ITEM_SIZE - BOSS_PHASE1_WALL_GAP_SIZE);

                for (int i = 0; i < gameState->screen_height_allegro / ITEM_SIZE; ++i) {
                    if (i < gap_start_y_index || i >= gap_start_y_index + BOSS_PHASE1_WALL_GAP_SIZE) {
                        int projectile_y = i * ITEM_SIZE + ITEM_SIZE / 2;
                        // TODO: Adapter spawn_projectile_ennemi ou créer une fonction pour le boss
                        // Pour l'instant, on simule que ça marche en appelant avec des params adaptés
                        // Il faudrait une fonction genre spawn_boss_projectile(gameState, x_depart, y_depart, vitesse, sprite, degats)
                        // ou modifier spawn_projectile_ennemi pour qu'il accepte des coordonnées si le tireur est NULL.
                        // Ici, on va imaginer que spawn_projectile_ennemi est un peu flexible.
                        // Le y_offset est calculé par rapport au centre du boss.
                        spawn_projectile_ennemi(gameState, NULL, // Pas de shooter spécifique, c'est le boss
                                                projectile_y - (boss->y + boss->h/2),
                                                BOSS_PHASE1_PROJECTILE_SPEED,
                                                gameState->sprite_projectile_ennemi1,
                                                BOSS_PHASE1_PROJECTILE_DAMAGE);
                    }
                }
                boss->phase1_attack_timer = BOSS_PHASE1_ATTACK_INTERVAL;
                boss->phase1_attack_counter++;
            }

            if (boss->phase1_attack_counter >= BOSS_PHASE1_ATTACK_COUNT) {
                // printf("Boss: Fin de la Phase 1, passage à la Phase 2.\n");
                boss->current_phase = BOSS_PHASE_2_VULNERABLE;
                boss->current_sprite = boss->sprite_phase2;
                boss->phase2_attack_timer = BOSS_PHASE2_ATTACK_INTERVAL;
            }
            break;

        case BOSS_PHASE_2_VULNERABLE:
            // Le boss peut prendre des dégâts (géré dans les collisions)
            boss->phase2_attack_timer--;
            if (boss->phase2_attack_timer <= 0) {
                // printf("Boss: Attaque simple Phase 2\n");
                // TODO: Tirer 1 ou 2 projectiles vers la position Y du joueur.
                // Exemple: spawn_projectile_ennemi(gameState, NULL, y_offset_vers_joueur, ...);
                boss->phase2_attack_timer = BOSS_PHASE2_ATTACK_INTERVAL;
            }
            break;
        case BOSS_PHASE_NONE:
            // Ne rien faire
            break;
    }
}

void dessiner_boss(GameState *gameState) {
    if (!gameState || !gameState->boss.active || !gameState->boss.current_sprite) return;
    Boss *boss = &gameState->boss;
    BITMAP *buffer = gameState->buffer;

    draw_sprite(buffer, boss->current_sprite, boss->x, boss->y);

    // Barre de vie du boss
    if (boss->active && boss->max_health > 0) {
        int bar_width = 200;
        int bar_height = 15;
        int bar_x = gameState->screen_width_allegro / 2 - bar_width / 2;
        int bar_y = 15; // En haut de l'écran
        float health_ratio = (float)boss->health / boss->max_health;
        int current_health_width = (int)(health_ratio * bar_width);

        rectfill(buffer, bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, makecol(50,50,50)); // Fond
        if (current_health_width > 0) {
            rectfill(buffer, bar_x, bar_y, bar_x + current_health_width, bar_y + bar_height, makecol(200,0,0)); // Vie
        }
        rect(buffer, bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, gameState->couleur_blanche); // Bordure
        textprintf_centre_ex(buffer, font, gameState->screen_width_allegro / 2, bar_y + bar_height + 5, gameState->couleur_blanche, -1, "BOSS HP");
    }
}

void nettoyer_ressources_boss(GameState *gameState) {
    if (!gameState) return;
    // printf("Nettoyage des ressources du boss...\n");
    if (gameState->boss.sprite_phase1) {
        destroy_bitmap(gameState->boss.sprite_phase1);
        gameState->boss.sprite_phase1 = NULL;
    }
    if (gameState->boss.sprite_phase2) {
        destroy_bitmap(gameState->boss.sprite_phase2);
        gameState->boss.sprite_phase2 = NULL;
    }
    // printf("Sprites du boss détruits.\n");
}
