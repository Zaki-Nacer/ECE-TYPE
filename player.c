#include "player.h"
#include "graphics.h" // Pour charger_bitmap_safe, make_white_transparent
#include <stdio.h>
#include "defs.h"
#include <math.h>
#include <allegro.h>



// --- Constantes Vaisseau Joueur ---
#define PLAYER_NBFRAMES_IDLE_MOVE 4
#define PLAYER_NBFRAMES_HIT 2
#define PLAYER_NBFRAMES_DEATH 6
#define PLAYER_ANIM_SPEED_MOVE 5
#define PLAYER_ANIM_SPEED_HIT 8
#define PLAYER_ANIM_SPEED_DEATH 10
#define PLAYER_HIT_DURATION (TARGET_FPS / 3) // Durée état HIT (env 1/3 sec)
#define PLAYER_SCALE_FACTOR 0.375f
#define PLAYER_SPEED 4
#define PLAYER_INITIAL_HP 5 // Points de vie initiaux
#define PLAYER_INVINCIBILITY_DURATION (TARGET_FPS * 2) // 2 secondes d'invincibilité après HIT

// --- Implémentation des Fonctions ---

// Fonction interne pour charger, redimensionner et traiter une séquence d'animation joueur
void load_process_player_anim(GameState *gameState, BITMAP *dest_array[], int nb_frames, const char *base_filename_pattern) {
    if (!gameState || !dest_array) { printf("ERREUR: gameState ou dest_array NULL dans load_process_player_anim\n"); fflush(stdout); return; }

    char filename[100];
    BITMAP *original_bmp = NULL;
    BITMAP *resized_bmp = NULL;

    printf("Chargement animation joueur: %s (%d frames)...\n", base_filename_pattern, nb_frames); fflush(stdout);

    for (int i = 0; i < nb_frames; i++) { // Boucle de 0 à nb_frames - 1
        sprintf(filename, base_filename_pattern, i + 1);
        printf(" -> Chargement fichier: %s\n", filename); fflush(stdout); // Log du fichier tenté
        original_bmp = charger_bitmap_safe(filename); // charger_bitmap_safe gère l'erreur si NULL

        // Redimensionnement
        int new_w = (int)roundf(original_bmp->w * PLAYER_SCALE_FACTOR);
        int new_h = (int)roundf(original_bmp->h * PLAYER_SCALE_FACTOR);
        if (new_w <= 0) new_w = 1; if (new_h <= 0) new_h = 1;

        resized_bmp = create_bitmap(new_w, new_h);
        if (!resized_bmp) {
            allegro_message("Erreur creation bitmap redimensionné pour joueur (%s)!", filename);
            exit(EXIT_FAILURE);
        }

        stretch_blit(original_bmp, resized_bmp, 0, 0, original_bmp->w, original_bmp->h, 0, 0, new_w, new_h);
        destroy_bitmap(original_bmp);

        make_white_transparent(gameState, resized_bmp);

        dest_array[i] = resized_bmp;
        printf(" -> Frame %d (%s) chargée/redim/traitée (%dx%d) -> index %d\n", i + 1, filename, new_w, new_h, i); fflush(stdout);

        // Vérification immédiate si le sprite chargé est NULL
        if (dest_array[i] == NULL) {
             printf("ERREUR CRITIQUE: dest_array[%d] est NULL juste après assignation pour %s!\n", i, base_filename_pattern); fflush(stdout);
             exit(EXIT_FAILURE);
        }
    }
    printf(" -> Animation joueur %s chargée.\n", base_filename_pattern); fflush(stdout);
}



// Charge toutes les animations du joueur DANS gameState
void load_player_animations(GameState *gameState) {
    if (!gameState) { printf("ERREUR: gameState NULL dans load_player_animations\n"); fflush(stdout); return; }
    printf("--- Chargement Sprites Joueur ---\n"); fflush(stdout);

    // Charger l'animation combinée Idle/Move
    const char *idle_move_pattern = "skeletonally-MovingNIdle_ (%d).bmp"; // Adaptez le nom du fichier si nécessaire
    load_process_player_anim(gameState, gameState->sprites_player_idle_move, PLAYER_NBFRAMES_IDLE_MOVE, idle_move_pattern);
    // *** NOUVELLE VÉRIFICATION ***
    if (PLAYER_NBFRAMES_IDLE_MOVE > 0 && gameState->sprites_player_idle_move[0] == NULL) {
        printf("ERREUR FATALE: La première frame de l'animation '%s' n'a pas pu être chargée correctement!\n", idle_move_pattern); fflush(stdout);
        exit(EXIT_FAILURE); // Arrêter car le joueur ne peut pas être initialisé
    } else {
         printf("Vérification post-chargement OK pour %s (frame 0: %p)\n", idle_move_pattern, (void*)gameState->sprites_player_idle_move[0]); fflush(stdout);
    }


    // Charger l'animation Hit
    const char *hit_pattern = "skeletonally0-GetHit__(%d).bmp"; // Adaptez le nom du fichier
    load_process_player_anim(gameState, gameState->sprites_player_hit, PLAYER_NBFRAMES_HIT, hit_pattern);
    if (PLAYER_NBFRAMES_HIT > 0 && gameState->sprites_player_hit[0] == NULL) {
        printf("ERREUR FATALE: La première frame de l'animation '%s' n'a pas pu être chargée correctement!\n", hit_pattern); fflush(stdout);
        exit(EXIT_FAILURE);
    } else {
         printf("Vérification post-chargement OK pour %s (frame 0: %p)\n", hit_pattern, (void*)gameState->sprites_player_hit[0]); fflush(stdout);
    }

    // Charger l'animation Death
    const char *death_pattern = "skeletonally0-Destroy__(%d).bmp"; // Adaptez le nom du fichier
    load_process_player_anim(gameState, gameState->sprites_player_death, PLAYER_NBFRAMES_DEATH, death_pattern);
     // *** NOUVELLE VÉRIFICATION ***
    if (PLAYER_NBFRAMES_DEATH > 0 && gameState->sprites_player_death[0] == NULL) {
        printf("ERREUR FATALE: La première frame de l'animation '%s' n'a pas pu être chargée correctement!\n", death_pattern); fflush(stdout);
        exit(EXIT_FAILURE);
    } else {
         printf("Vérification post-chargement OK pour %s (frame 0: %p)\n", death_pattern, (void*)gameState->sprites_player_death[0]); fflush(stdout);
    }


    printf("--- Tous les sprites joueur chargés et traités ---\n"); fflush(stdout);
}

// Initialise l'état du joueur au début
void init_player_state(GameState *gameState) {
    if (!gameState) return;
    printf("Initialisation état joueur...\n"); fflush(stdout);
    Vaisseau *p = &gameState->joueur;
    p->active = 1;
    p->health = PLAYER_INITIAL_HP;
    p->x = 50;
    p->y = gameState->screen_height_allegro / 2;
    p->state_timer = 0;
    p->state = -1;
    printf("Appel de set_player_state avec STATE_IDLE...\n"); fflush(stdout);
    set_player_state(gameState, STATE_IDLE);
    printf("Retour de set_player_state. Vérification des sprites...\n"); fflush(stdout);
    if (p->active && p->current_sprites && p->current_nb_frames > 0 && p->current_sprites[0]) {
        p->w = p->current_sprites[0]->w;
        p->h = p->current_sprites[0]->h;
        printf("Joueur initialisé avec succès: état=%d, x=%d, y=%d, w=%d, h=%d, sprite[0]=%p\n",
               p->state, p->x, p->y, p->w, p->h, (void*)p->current_sprites[0]); fflush(stdout);
    } else {
         printf("ERREUR CRITIQUE: Sprites invalides lors de l'initialisation du joueur!\n");
         printf("  p->active = %d\n", p->active);
         printf("  p->current_sprites = %p\n", (void*)p->current_sprites);
         if (p->current_sprites) {
             printf("  p->current_nb_frames = %d\n", p->current_nb_frames);
             if (p->current_nb_frames > 0) {
                 printf("  p->current_sprites[0] = %p\n", (void*)p->current_sprites[0]);
             }
         }
         fflush(stdout);
         p->active = 0;
         exit(EXIT_FAILURE);
    }
}

// Définit l'état du joueur et met à jour les sprites/animations
void set_player_state(GameState *gameState, PlayerState new_state) {
    if (!gameState) return;
    Vaisseau *p = &gameState->joueur;
    if (p->state == new_state && new_state != STATE_HIT) return;
    PlayerState old_state = p->state;
    p->state = new_state;
    p->imgcourante = 0;
    p->cptimg = 0;
    p->state_timer = 0;
    p->current_sprites = NULL;
    p->current_nb_frames = 0;
    p->tmpimg = 0;
    switch (new_state) {
        case STATE_IDLE:
        case STATE_MOVING:
            p->current_sprites = gameState->sprites_player_idle_move;
            p->current_nb_frames = PLAYER_NBFRAMES_IDLE_MOVE;
            p->tmpimg = PLAYER_ANIM_SPEED_MOVE;
            if (new_state == STATE_IDLE) { p->imgcourante = 0; }
            break;
        case STATE_HIT:
            p->current_sprites = gameState->sprites_player_hit;
            p->current_nb_frames = PLAYER_NBFRAMES_HIT;
            p->tmpimg = PLAYER_ANIM_SPEED_HIT;
            p->state_timer = PLAYER_HIT_DURATION;
            break;
        case STATE_DYING:
            p->current_sprites = gameState->sprites_player_death;
            p->current_nb_frames = PLAYER_NBFRAMES_DEATH;
            p->tmpimg = PLAYER_ANIM_SPEED_DEATH;
            break;
        default:
            printf("ERREUR: set_player_state état inconnu %d\n", new_state); fflush(stdout);
            p->active = 0;
            return;
    }
    int error_found = 0;

    // pleins de debug utilie mais a enlevé plus tard
    if (p->current_sprites == NULL) {
        printf("ERREUR CRITIQUE: Dans set_player_state (nouvel état %d): current_sprites est NULL après assignation!\n", new_state); fflush(stdout);
        error_found = 1;
    } else if (p->current_nb_frames <= 0) {
        printf("ERREUR CRITIQUE: Dans set_player_state (nouvel état %d): current_nb_frames est %d!\n", new_state, p->current_nb_frames); fflush(stdout);
        error_found = 1;
    } else if (p->current_sprites[0] == NULL) {
        printf("ERREUR CRITIQUE: Dans set_player_state (nouvel état %d): La première frame (current_sprites[0]) est NULL!\n", new_state); fflush(stdout);
        error_found = 1;
    }
    if(error_found) {
        p->active = 0;
        p->current_sprites = NULL;
        printf(" -> Joueur marqué comme inactif à cause de l'erreur de sprite.\n"); fflush(stdout);
    } else if (p->active) {
        p->w = p->current_sprites[0]->w;
        p->h = p->current_sprites[0]->h;
    }
}

// Met à jour la logique du joueur
void mettre_a_jour_joueur(GameState *gameState) {
    if (!gameState || !gameState->joueur.active) return;
    Vaisseau *p = &gameState->joueur;
    int is_moving = 0; // Flag pour savoir si une touche de mouvement est pressée

    // 1. Gérer les états temporaires (HIT, DYING) et le retour à IDLE
    if (p->state == STATE_HIT) {
        p->state_timer--; // Décrémente le timer de l'état visuel HIT
        if (p->state_timer <= 0) {
            // Fin de l'état HIT visuel, repasser en IDLE
            // L'invincibilité (gérée par le timer plus long) peut continuer
            set_player_state(gameState, STATE_IDLE);
        }
        // *** NOTE: On ne bloque PLUS le mouvement ici ***
    } else if (p->state == STATE_DYING) {
        // L'animation de mort est gérée plus bas.
        // *** PAS de mouvement pendant l'état DYING ***
        // On sort de la logique de mouvement/état ci-dessous
        goto update_animation; // Saute directement à la mise à jour de l'animation
    }

    // 2. Gérer les déplacements (s'exécute si état IDLE, MOVING ou HIT)
    int dx = 0;
    int dy = 0;
    if (key[KEY_UP])   { dy -= PLAYER_SPEED; is_moving = 1; }
    if (key[KEY_DOWN]) { dy += PLAYER_SPEED; is_moving = 1; }
    if (key[KEY_LEFT]) { dx -= PLAYER_SPEED; is_moving = 1; }
    if (key[KEY_RIGHT]){ dx += PLAYER_SPEED; is_moving = 1; }

    // Appliquer le déplacement
    p->x += dx;
    p->y += dy;

    // Limiter le joueur à l'écran
    if (p->x < 0) p->x = 0;
    if (p->y < 0) p->y = 0;
    if (p->w > 0 && p->x + p->w > gameState->screen_width_allegro) {
        p->x = gameState->screen_width_allegro - p->w;
    }
    if (p->h > 0 && p->y + p->h > gameState->screen_height_allegro) {
        p->y = gameState->screen_height_allegro - p->h;
    }

    // 3. Changer l'état entre IDLE et MOVING (seulement si pas en état HIT)
    //    On ne veut pas passer en MOVING si on est visuellement en HIT
    if (p->state != STATE_HIT) {
        if (is_moving && p->state == STATE_IDLE) {
            set_player_state(gameState, STATE_MOVING);
        } else if (!is_moving && p->state == STATE_MOVING) {
            set_player_state(gameState, STATE_IDLE);
        }
    }


update_animation: // Label pour le goto de l'état DYING

    // 4. Mettre à jour l'animation (pour tous les états)
    if (p->current_sprites && p->current_nb_frames > 0 && p->tmpimg > 0) {
        // Ne pas animer si en état IDLE et on veut une frame fixe
        if (p->state == STATE_IDLE && p->current_nb_frames > 1) {
             p->imgcourante = 0; // Bloquer sur la frame 0 pour IDLE   à changer plus tard MAUVAISE LOGIQUE DE JEU
        } else {
            // Animer pour les autres états (MOVING, HIT, DYING)
            p->cptimg++;
            if (p->cptimg >= p->tmpimg) {
                p->cptimg = 0;
                p->imgcourante++;

                // Gérer la fin de l'animation
                if (p->imgcourante >= p->current_nb_frames) {
                    if (p->state == STATE_DYING) {
                        // Fin de l'animation de mort -> GAME OVER
                        p->active = 0; // Désactiver le joueur logiquement
                        gameState->current_game_state = GAME_STATE_GAME_OVER; // Changer l'état global du jeu
                        printf("Joueur désactivé (fin anim mort). GAME OVER.\n"); fflush(stdout);
                    } else if (p->state == STATE_HIT) {
                        // Fin anim HIT, mais l'état a déjà été changé par le timer.
                        // Reste sur la dernière frame de HIT jusqu'à ce que l'état change.
                        p->imgcourante = p->current_nb_frames - 1;
                    } else if (p->state == STATE_MOVING) {
                        // Boucler l'animation de mouvement (frames 1, 2, 3...)
                        if (PLAYER_NBFRAMES_IDLE_MOVE > 1) {
                            p->imgcourante = 1; // Retourne à la première frame de mouvement
                        }
                    } else {
                         p->imgcourante = 0; // Boucle simple
                    }
                }
            }
        }
         // Mettre à jour w/h basé sur la frame actuelle (si elle a changé)
         if (p->active && p->imgcourante >= 0 && p->imgcourante < p->current_nb_frames && p->current_sprites[p->imgcourante]) {
              p->w = p->current_sprites[p->imgcourante]->w;
              p->h = p->current_sprites[p->imgcourante]->h;
         }
    }

    // 5. Gérer le timer d'invincibilité (indépendant de l'état visuel HIT)
    //    Ce timer est démarré dans damage_player
    if (p->state_timer > PLAYER_HIT_DURATION) { // Si on est dans la période d'invincibilité étendue
        p->state_timer--;
        if (p->state_timer == PLAYER_HIT_DURATION) {
             // Fin de l'invincibilité pure, l'état visuel HIT peut encore être actif ou non
             // printf("DEBUG: Fin invincibilité joueur.\n"); fflush(stdout);
        }
    }
}


// Dessine le joueur
void dessiner_joueur(GameState *gameState) {
    if (!gameState || !gameState->buffer || !gameState->joueur.active) return;
    Vaisseau *p = &gameState->joueur;
    if (p->current_sprites &&
        p->imgcourante >= 0 && p->imgcourante < p->current_nb_frames &&
        p->current_sprites[p->imgcourante])
    {
        int doit_dessiner = 1;
        if (p->state != STATE_HIT && p->state_timer > PLAYER_HIT_DURATION) { // Clignote si invincible mais pas en état HIT visuel
            if ((gameState->game_loop_counter % 8) < 4) {
                doit_dessiner = 0;
            }
        }
        if (doit_dessiner) {
            draw_sprite(gameState->buffer, p->current_sprites[p->imgcourante], p->x, p->y);
        }
    } else if (p->active) {
        printf("ERREUR: Tentative de dessin joueur état %d frame %d invalide.\n", p->state, p->imgcourante);
        fflush(stdout);
        rectfill(gameState->buffer, p->x, p->y, p->x + 10, p->y + 10, makecol(255, 255, 0)); // Jaune
    }
}

// Inflige des dégâts au joueur
void damage_player(GameState *gameState, int damage_amount) {
     if (!gameState || !gameState->joueur.active || gameState->joueur.state == STATE_DYING) return;
     Vaisseau *p = &gameState->joueur;
     if (p->state_timer > PLAYER_HIT_DURATION) { // Utilise le timer étendu pour l'invincibilité
         return;
     }
     printf("Joueur touché! Vie: %d -> ", p->health); fflush(stdout);
     p->health -= damage_amount;
     printf("%d\n", p->health); fflush(stdout);
     if (p->health <= 0) {
         set_player_state(gameState, STATE_DYING);
         printf("Joueur passe à l'état DYING.\n"); fflush(stdout);
     } else {
         set_player_state(gameState, STATE_HIT);
         p->state_timer = PLAYER_INVINCIBILITY_DURATION; // Démarre l'invincibilité longue
         printf("Joueur passe à l'état HIT (Invincible pour %d frames).\n", p->state_timer); fflush(stdout);
     }
}
