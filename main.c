#include "defs.h"
#include "graphics.h"
#include "player.h"   // Inclut init_player_state, mettre_a_jour_joueur, dessiner_joueur, damage_player
#include "enemy.h"    // Inclut initialiser_ennemis_array, spawn_ennemi, mettre_a_jour_ennemis, dessiner_ennemis, set_enemy_state
#include "projectile.h" // Inclut initialiser_projectiles_joueur, spawn_projectile_joueur, mettre_a_jour_projectiles_joueur, dessiner_projectiles_joueur
#include "hud.h"      // Inclut charger_ressources_hud, mettre_a_jour_hud, dessiner_hud, nettoyer_ressources_hud
#include "effect.h"   // Inclut charger_sprites_effects, initialiser_effects, spawn_impact_effect, mettre_a_jour_effects, dessiner_effects, nettoyer_ressources_effects
#include "savegame.h" // Inclut load_progress, save_progress
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <allegro.h> // Pour key[], rest(), etc.
#include "menu.h"


// --- Définition Variable Globale Compteur ---
// Déplacée dans GameState pour une meilleure encapsulation
// long game_loop_counter = 0; // Remplacé par gameState.game_loop_counter

// --- Fonction de Nettoyage Globale ---
// *** MODIFIÉ: Appelle les fonctions de nettoyage spécifiques ***
void cleanup(GameState *gameState) {
    printf("--- Nettoyage global ---\n"); fflush(stdout);
    if (!gameState) return;

    // 1. Nettoyer les ressources spécifiques à chaque module
    nettoyer_ressources_projectiles_joueur(gameState); // Nettoie sprite vol (impact partagé)
    nettoyer_ressources_effects(gameState);           // Nettoie sprites impact
    nettoyer_ressources_ennemis(gameState);           // Nettoie sprites ennemis
    nettoyer_ressources_joueur(gameState);            // Nettoie sprites joueur
    nettoyer_ressources_hud(gameState);               // Nettoie sprites HUD

    // 2. Nettoyer les ressources graphiques générales
    nettoyer_ressources_graphiques(gameState);        // Nettoie buffer, decor

    // 3. Arrêter Allegro (si nécessaire, souvent fait à la fin de main)
    // allegro_exit(); // Déplacé à la toute fin de main

    printf("--- Nettoyage terminé ---\n"); fflush(stdout);
}

// Fonction de détection de collision simple (Bounding Box)
int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    // Vérifie s'il n'y a PAS de collision
    if (x1 + w1 < x2 || x1 > x2 + w2 || y1 + h1 < y2 || y1 > y2 + h2) {
        return 0; // Pas de collision
    }
    // Sinon, il y a collision
    return 1;
}

// Fonction pour gérer les collisions
// *** RÉVISÉ: Logique de collision et changement d'état ***
void gerer_collisions(GameState *gameState) {
    if (!gameState) return;

    Vaisseau *player = &gameState->joueur;

    // --- Collision Projectiles Joueur vs Ennemis ---
    for (int i = 0; i < MAX_PROJECTILES_JOUEUR; i++) {
        Projectile *p = &gameState->projectiles_joueur[i];
        // Vérifier uniquement les projectiles actifs et en vol
        if (!p->active || p->state != PROJECTILE_STATE_FLYING) continue;

        for (int j = 0; j < MAX_ENNEMIS; j++) {
            Ennemi *e = &gameState->ennemis[j];
            // Vérifier uniquement les ennemis actifs et non mourants/déjà touchés (pour éviter multi-hit par 1 tir)
            if (!e->active || e->state == ENEMY_STATE_DYING || e->state == ENEMY_STATE_HIT) continue;

            // Convertir les coordonnées MONDE de l'ennemi en coordonnées ECRAN
            int ennemi_screen_x = e->x - gameState->scroll_x;
            int ennemi_screen_y = e->y - gameState->scroll_y; // Supposant scroll_y = 0

            // Les coordonnées du projectile (p->x, p->y) sont déjà en ECRAN
            if (check_collision(p->x, p->y, p->w, p->h,
                                ennemi_screen_x, ennemi_screen_y, e->w, e->h))
            {
                // --- Collision détectée ---
                // Debug: printf("Collision: Projectile %d avec Ennemi %d (type %d)\n", i, j, e->type); fflush(stdout);

                // 1. Créer un effet visuel à l'impact (centré sur le projectile)
                int impact_x = p->x + p->w / 2;
                int impact_y = p->y + p->h / 2;
                spawn_impact_effect(gameState, impact_x, impact_y);

                // 2. Mettre le projectile en état d'impact (il jouera son anim et se désactivera)
                p->state = PROJECTILE_STATE_IMPACTING;
                p->impact_frame = 0; // Démarrer l'animation d'impact
                p->impact_timer = 0;
                // Note: p->active reste 1 pendant l'impact

                // 3. Infliger des dégâts à l'ennemi
                e->health -= 1; // Simplifié: 1 dégât par projectile
                // Debug: printf("   -> Vie Ennemi %d: %d\n", j, e->health); fflush(stdout);

                // 4. Changer l'état de l'ennemi
                if (e->health <= 0) {
                    // Si la vie tombe à 0 ou moins, passer à l'état DYING
                    set_enemy_state(gameState, e, ENEMY_STATE_DYING);
                    // Debug: printf("   -> Ennemi %d passe à l'état DYING !\n", j); fflush(stdout);
                } else {
                    // Sinon, passer à l'état HIT (courte invincibilité et anim de dégât)
                    set_enemy_state(gameState, e, ENEMY_STATE_HIT);
                    // Debug: printf("   -> Ennemi %d passe à l'état HIT !\n", j); fflush(stdout);
                }

                // Puisqu'un projectile a touché cet ennemi, passer au projectile suivant
                // (évite qu'un même projectile touche plusieurs ennemis en une frame)
                goto next_projectile; // Utilisation prudente de goto pour sortir de la boucle interne

            } // Fin if check_collision
        } // Fin boucle ennemis (j)

        next_projectile:; // Label pour le goto

    } // Fin boucle projectiles (i)


    // --- Collision Joueur vs Ennemis ---
    // Vérifier seulement si le joueur est actif, pas mourant et pas invincible (state_timer <= 0)
    if (player->active && player->state != STATE_DYING && player->state != STATE_HIT && player->state_timer <= 0) {
        for (int j = 0; j < MAX_ENNEMIS; j++) {
            Ennemi *e = &gameState->ennemis[j];
            // Ignorer ennemis inactifs, mourants ou déjà touchés (évite double collision)
            if (!e->active || e->state == ENEMY_STATE_DYING || e->state == ENEMY_STATE_HIT) continue;

            // Convertir coords ennemi MONDE -> ECRAN
            int ennemi_screen_x = e->x - gameState->scroll_x;
            int ennemi_screen_y = e->y - gameState->scroll_y;

            // Coords joueur (player->x, player->y) sont ECRAN
            if (check_collision(player->x, player->y, player->w, player->h,
                                ennemi_screen_x, ennemi_screen_y, e->w, e->h))
            {
                 // --- Collision Joueur-Ennemi détectée ---
                 // Debug: printf("COLLISION JOUEUR vs ENNEMI %d !\n", j); fflush(stdout);

                 // 1. Infliger des dégâts au joueur (ex: 1 point)
                 // La fonction damage_player gère la vie, l'état HIT, l'invincibilité et le GAME OVER
                 damage_player(gameState, 1);

                 // 2. Optionnel: Infliger aussi des dégâts à l'ennemi ou le détruire ?
                 //    Ici, on choisit de détruire l'ennemi au contact pour simplifier.
                 if (e->state != ENEMY_STATE_DYING) { // S'il n'est pas déjà en train de mourir
                     e->health = 0; // Met sa vie à 0
                     set_enemy_state(gameState, e, ENEMY_STATE_DYING); // Lance son animation de mort
                     // Debug: printf("   -> Ennemi %d détruit par collision joueur.\n", j); fflush(stdout);
                 }


                 // 3. Si le joueur est touché (damage_player l'a mis en état HIT),
                 //    on arrête de vérifier les collisions joueur-ennemi pour cette frame.
                 if (player->state == STATE_HIT) {
                     break; // Sortir de la boucle des ennemis (j)
                 }
            } // Fin if check_collision joueur-ennemi
        } // Fin boucle ennemis (j) pour collision joueur
    } // Fin if (player peut être touché)

    // Ajouter autres types de collisions ici (ex: Joueur vs Projectiles Ennemis)
}


// Fonction pour préparer un niveau
void load_level(GameState *gameState, int level_number) {
    if (!gameState) return;
    printf("Chargement Niveau %d...\n", level_number); fflush(stdout);
    gameState->current_level = level_number;

    // Réinitialiser états spécifiques au niveau
    gameState->scroll_x = 0;
    gameState->scroll_y = 0;
    gameState->game_loop_counter = 0; // Réinitialiser le compteur de temps/progression du niveau
    gameState->hud_displayed_level_progress = 0; // Réinitialiser l'affichage de la barre

    // Réinitialiser joueur (position, état initial, vie pleine)
    init_player_state(gameState); // Cette fonction devrait remettre la vie au max, position initiale, état IDLE

    // Vider les listes d'entités actives du niveau précédent
    initialiser_ennemis_array(gameState);       // Marque tous les ennemis comme inactifs
    initialiser_projectiles_joueur(gameState);  // Marque tous les projectiles comme inactifs
    initialiser_effects(gameState);             // Marque tous les effets comme inactifs

    // Prêt à commencer à jouer ce niveau
    printf("Niveau %d prêt.\n", level_number); fflush(stdout);
    gameState->current_game_state = GAME_STATE_PLAYING; // Mettre l'état du jeu à PLAYING
}


// --- Point d'Entrée Principal ---
int main() {
    printf("Démarrage du jeu...\n"); fflush(stdout);
    GameState gameState;
    memset(&gameState, 0, sizeof(GameState)); // Initialiser toute la structure à zéro/NULL
    gameState.current_game_state = GAME_STATE_INIT; // État initial avant chargement Allegro
    gameState.current_level = 1; // Niveau de départ par défaut
    gameState.quit_game = 0; // Flag pour quitter le jeu

    srand(time(NULL)); // Initialiser le générateur de nombres aléatoires

    // --- Initialisation Allegro ---
    if (initialisation_allegro(&gameState) != 0) {
        // Erreur gérée dans la fonction initialisation_allegro
        return 1; // Quitter si Allegro ne s'initialise pas
    }
    printf("Allegro initialisé.\n"); fflush(stdout);

    // --- Logique Menu Simplifiée (Console) ---
    // (Peut être remplacé par un vrai menu graphique plus tard)
    printf("\n===========================\n");
    printf("      R-TYPE LIKE GAME\n");
    printf("===========================\n");
    // Demander le nom du joueur (pour la sauvegarde)
    printf("Entrez votre nom: "); fflush(stdout);
    if (fgets(gameState.player_name, MAX_PLAYER_NAME_LEN, stdin)) {
        gameState.player_name[strcspn(gameState.player_name, "\n")] = 0; // Enlever le newline
    } else {
        strcpy(gameState.player_name, "Player"); // Nom par défaut
    }
    printf("Bienvenue, %s!\n", gameState.player_name); fflush(stdout);

    // Charger la progression si elle existe
    int saved_level = load_progress(gameState.player_name);
    if (saved_level > 1 && saved_level <= MAX_LEVEL) {
        printf("Progression trouvée au niveau %d. Continuer (C) ou Nouvelle partie (N)? ", saved_level); fflush(stdout);
        char choice = getchar();
        while (getchar() != '\n'); // Vider buffer stdin
        if (choice == 'C' || choice == 'c') {
            gameState.current_level = saved_level;
            printf("Reprise au niveau %d.\n", gameState.current_level); fflush(stdout);
        } else {
             gameState.current_level = 1;
             printf("Nouvelle partie au niveau 1.\n"); fflush(stdout);
        }
    } else {
        gameState.current_level = 1;
        printf("Début au niveau 1.\n"); fflush(stdout);
    }
    // ------------------------------------

    printf("Chargement des ressources globales...\n"); fflush(stdout);
    // Charger le décor
    gameState.decor = charger_bitmap_safe("background.bmp");
    if (!gameState.decor) { cleanup(&gameState); allegro_exit(); return 1; }
    // Vérifier si le décor est assez large pour le scrolling
    if (gameState.decor->w < gameState.screen_width_allegro) {
        allegro_message("Erreur: Le bitmap de fond ('background.bmp') est moins large que l'écran (%d < %d)!", gameState.decor->w, gameState.screen_width_allegro);
        cleanup(&gameState); allegro_exit(); return 1;
    }

    // Charger les sprites et animations pour chaque type d'entité
    load_player_animations(&gameState); // Charge toutes les anims joueur
    charger_sprites_ennemis(&gameState); // Charge anims ennemis (move, hit, death)
    charger_et_redimensionner_sprite_projectile_joueur(&gameState); // Charge sprite vol projectile
    charger_sprites_effects(&gameState); // Charge anims effets (impact)
    charger_ressources_hud(&gameState); // Charge sprites HUD (barres)

    printf("Initialisation finale des états...\n"); fflush(stdout);
    // Initialiser les tableaux d'entités (les marquer comme inactifs)
    // Note: Le joueur est initialisé dans load_level
    initialiser_ennemis_array(&gameState);
    initialiser_projectiles_joueur(&gameState);
    initialiser_effects(&gameState);

    // Charger le premier niveau (ou le niveau sauvegardé)
    // Cette fonction initialise le joueur et met gameState.current_game_state = GAME_STATE_PLAYING
    load_level(&gameState, gameState.current_level);

    // Variables pour la boucle de jeu
    gameState.can_shoot = 1; // Le joueur peut tirer au début
    gameState.shoot_timer = 0;
    int shoot_delay = 8; // Délai entre les tirs (en nombre de boucles) - Peut être mis dans defs.h
    int spawn_timer = 0; // Timer pour l'apparition des ennemis
    int spawn_interval = 90; // Intervalle d'apparition initial (en boucles) - Peut varier selon niveau/temps

    printf("Entrée dans la boucle principale...\n"); fflush(stdout);
    // --- Boucle de Jeu Principale ---
    while (gameState.current_game_state != GAME_STATE_EXITING) { // Boucle tant qu'on ne demande pas de quitter

        // --- Gestion des états de jeu ---
        switch (gameState.current_game_state) {

            case GAME_STATE_PLAYING:
                // Incrémenter le compteur global de la boucle (pour la durée du niveau)
                gameState.game_loop_counter++;
                // Debug: if (gameState.game_loop_counter % 100 == 0) { printf("--- Boucle PLAYING: Tour %ld ---\n", gameState.game_loop_counter); fflush(stdout); }

                // 1. --- Gestion des Entrées ---
                // Tir du joueur
                if (key[KEY_SPACE] && gameState.can_shoot && gameState.joueur.active && gameState.joueur.state != STATE_DYING) {
                    // Calculer position de départ du tir (devant le vaisseau)
                    int tir_start_x = gameState.joueur.x + gameState.joueur.w; // A droite du joueur
                    int tir_start_y = gameState.joueur.y + gameState.joueur.h / 2; // Au milieu verticalement
                    spawn_projectile_joueur(&gameState, tir_start_x, tir_start_y);
                    gameState.can_shoot = 0; // Empêcher tir immédiat
                    gameState.shoot_timer = 0; // Réinitialiser timer de tir
                }
                // Mettre à jour timer de tir si on ne peut pas tirer
                if (!gameState.can_shoot) {
                    gameState.shoot_timer++;
                    if (gameState.shoot_timer >= shoot_delay) {
                        gameState.can_shoot = 1; // Autoriser à nouveau le tir
                    }
                }
                // Les mouvements du joueur sont gérés dans mettre_a_jour_joueur() en lisant key[] directement

                // Quitter avec Echap
                if (key[KEY_ESC]) {
                    gameState.current_game_state = GAME_STATE_EXITING;
                    printf("Touche Echap pressée, sortie demandée.\n"); fflush(stdout);
                    continue; // Aller directement à la fin de la boucle while
                }

                // 2. --- Mises à Jour Logiques ---
                mettre_a_jour_scrolling(&gameState); // Déplacer le fond
                mettre_a_jour_joueur(&gameState);    // Gérer état, anim, déplacement joueur
                mettre_a_jour_ennemis(&gameState);   // Gérer état, anim, déplacement ennemis
                mettre_a_jour_projectiles_joueur(&gameState); // Gérer déplacement, impact, désactivation projectiles
                mettre_a_jour_effects(&gameState);   // Gérer animation et désactivation effets
                mettre_a_jour_hud(&gameState);       // Mettre à jour affichage barre progression

                // --- Spawning Ennemis ---
                spawn_timer++;
                if (spawn_timer >= spawn_interval) {
                    spawn_timer = 0; // Réinitialiser timer
                    // Logique de spawn (peut être plus complexe)
                    int type_ennemi = rand() % 3; // Type aléatoire 0, 1 ou 2
                    // Position Y aléatoire (dans les limites de l'écran, avec marge)
                    int pos_y_ennemi = 30 + rand() % (gameState.screen_height_allegro - 60 - 50); // 50 = hauteur max approx ennemi
                    // Position X: juste à droite de l'écran (coordonnées MONDE)
                    int pos_x_ennemi = gameState.scroll_x + gameState.screen_width_allegro + rand() % 50;
                    spawn_ennemi(&gameState, type_ennemi, pos_x_ennemi, pos_y_ennemi);

                    // Optionnel: Rendre le spawn plus fréquent avec le temps?
                    // if (spawn_interval > 30) spawn_interval--;
                }

                // --- Gestion des Collisions ---
                gerer_collisions(&gameState); // Vérifie et gère toutes les collisions
                // Note: gerer_collisions peut changer l'état du jeu en GAME_OVER si le joueur meurt

                // --- Vérifier Condition de Fin de Niveau (par temps) ---
                if (gameState.game_loop_counter >= LEVEL_DURATION_FRAMES) {
                    gameState.current_game_state = GAME_STATE_LEVEL_COMPLETE;
                    printf("Niveau %d terminé (temps écoulé) !\n", gameState.current_level); fflush(stdout);
                }
                break; // Fin de l'état PLAYING


            case GAME_STATE_LEVEL_COMPLETE:
                { // Bloc pour déclaration de variable locale
                    int next_level = gameState.current_level + 1;
                    // Sauvegarder la progression (atteinte du niveau suivant)
                    save_progress(gameState.player_name, next_level);

                    if (next_level > MAX_LEVEL) {
                        // Si c'était le dernier niveau, passer à l'état GAME_WON
                        gameState.current_game_state = GAME_STATE_GAME_WON;
                        printf("Jeu terminé ! Vous avez gagné !\n"); fflush(stdout);
                        rest(1000); // Petite pause avant l'écran de victoire
                    } else {
                        // Sinon, charger le niveau suivant
                        printf("Passage au niveau %d...\n", next_level); fflush(stdout);
                        rest(1500); // Pause avant de charger
                        load_level(&gameState, next_level); // Charge le niveau et met l'état à PLAYING
                    }
                }
                break; // Fin de l'état LEVEL_COMPLETE


            case GAME_STATE_GAME_OVER:
                // Afficher "GAME OVER" et attendre une touche pour quitter
                printf("GAME OVER - Appuyez sur Entrée ou Echap pour quitter...\n"); fflush(stdout);
                // Dessiner "GAME OVER" sur le buffer
                clear_bitmap(gameState.buffer); // Effacer l'écran de jeu
                textout_centre_ex(gameState.buffer, font, "GAME OVER",
                                  gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2,
                                  makecol(255, 0, 0), -1); // Texte rouge
                // Afficher le buffer à l'écran
                blit(gameState.buffer, screen, 0, 0, 0, 0, gameState.screen_width_allegro, gameState.screen_height_allegro);

                // Attendre Entrée ou Echap
                clear_keybuf(); // Vider buffer clavier pour éviter détection immédiate
                while (!key[KEY_ENTER] && !key[KEY_ESC]) {
                    rest(50); // Petite pause pour ne pas saturer le CPU
                }
                gameState.current_game_state = GAME_STATE_EXITING; // Demander la sortie de la boucle principale
                break; // Fin de l'état GAME_OVER


            case GAME_STATE_GAME_WON:
                 // Afficher "VICTOIRE !" et attendre une touche pour quitter
                printf("VICTOIRE ! - Appuyez sur Entrée ou Echap pour quitter...\n"); fflush(stdout);
                // Dessiner "VICTOIRE !" sur le buffer
                clear_bitmap(gameState.buffer);
                textout_centre_ex(gameState.buffer, font, "VICTOIRE !",
                                  gameState.screen_width_allegro / 2, gameState.screen_height_allegro / 2,
                                  makecol(0, 255, 0), -1); // Texte vert
                 // Afficher le buffer à l'écran
                blit(gameState.buffer, screen, 0, 0, 0, 0, gameState.screen_width_allegro, gameState.screen_height_allegro);

                // Attendre Entrée ou Echap
                clear_keybuf();
                while (!key[KEY_ENTER] && !key[KEY_ESC]) {
                    rest(50);
                }
                gameState.current_game_state = GAME_STATE_EXITING; // Demander la sortie
                break; // Fin de l'état GAME_WON


            case GAME_STATE_INIT:
            case GAME_STATE_MENU: // Ces états ne devraient pas être atteints dans cette boucle principale
            case GAME_STATE_EXITING: // Géré par la condition while
                 // Ne rien faire ou logguer une erreur si on arrive ici de manière inattendue
                 printf("WARN: État de jeu inattendu (%d) dans la boucle principale.\n", gameState.current_game_state); fflush(stdout);
                 gameState.current_game_state = GAME_STATE_EXITING; // Forcer la sortie par sécurité
                 break;

        } // Fin switch (current_game_state)


        // --- Dessin (uniquement si on n'est pas dans un état final comme GAME OVER/WON) ---
        if (gameState.current_game_state == GAME_STATE_PLAYING) {
            // 3. Affichage sur le buffer
            dessiner_decor(&gameState);             // Dessine le fond scrollé
            dessiner_ennemis(&gameState);           // Dessine les ennemis actifs
            dessiner_projectiles_joueur(&gameState);// Dessine les projectiles actifs (vol ou impact)
            dessiner_effects(&gameState);           // Dessine les effets visuels (explosions, etc.)
            dessiner_joueur(&gameState);            // Dessine le joueur (avec son état/animation)
            dessiner_hud(&gameState);               // Dessine l'interface (barre de progression, vie...)

            // 4. Copier le buffer complet à l'écran (double buffering)
            blit(gameState.buffer, screen, 0, 0, 0, 0, gameState.screen_width_allegro, gameState.screen_height_allegro);
        }

        // 5. Pause globale pour réguler la vitesse du jeu
        rest(15); // Vise environ 66 FPS (1000ms / 15ms ~= 66.6 FPS) - Ajuster si besoin

    } // --- Fin de la Boucle while principale ---

    printf("Sortie de la boucle principale (État final: %d).\n", gameState.current_game_state); fflush(stdout);

    // --- Nettoyage avant de quitter ---
    cleanup(&gameState);

    // --- Arrêt Allegro ---
    allegro_exit();
    printf("Allegro arrêté.\n"); fflush(stdout);

    printf("Programme terminé.\n"); fflush(stdout);
    return 0; // Fin normale du programme
}
END_OF_MAIN(); // Macro Allegro requise
