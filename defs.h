#ifndef DEFS_H
#define DEFS_H

#include <allegro.h>

// --- Constantes Générales ---
#define TARGET_FPS 66
#define LEVEL_DURATION_SECONDS 50
#define LEVEL_DURATION_FRAMES (LEVEL_DURATION_SECONDS * TARGET_FPS) // on compte le temps en frame et non en seconde
#define MAX_LEVEL 3

// --- Constantes Vaisseau Joueur ---
#define PLAYER_NBFRAMES_IDLE_MOVE 14
#define PLAYER_NBFRAMES_HIT 14
#define PLAYER_NBFRAMES_DEATH 22
#define PLAYER_ANIM_SPEED_MOVE 5
#define PLAYER_ANIM_SPEED_HIT 8
#define PLAYER_ANIM_SPEED_DEATH 10
#define PLAYER_HIT_DURATION (TARGET_FPS / 3)
#define PLAYER_SCALE_FACTOR 0.6f // Taille joueur modifiée précédemment
#define PLAYER_SPEED 4
#define PLAYER_INITIAL_HP 5
#define PLAYER_INVINCIBILITY_DURATION (TARGET_FPS * 2)

// --- Constantes Ennemis ---
#define MAX_ENNEMIS 50
#define ENEMY_NBFRAMES_MOVE 18
#define ENEMY_NBFRAMES_HIT 18
#define ENEMY_NBFRAMES_DEATH 18
#define ENEMY_ANIM_SPEED_MOVE 8
#define ENEMY_ANIM_SPEED_HIT 6
#define ENEMY_ANIM_SPEED_DEATH 1
#define ENEMY_HIT_DURATION 3
// #define ENEMY_HP 2 // Ancienne constante HP unique

// *** NOUVEAU: PV spécifiques par type d'ennemi ***
#define ENEMY_HP_TYPE_0 5
#define ENEMY_HP_TYPE_1 10
#define ENEMY_HP_TYPE_2 7

// *** NOUVEAU: Intervalles de tir spécifiques (en frames) ***
// Plus l'intervalle est grand, plus le tir est lent
#define ENEMY_FIRE_INTERVAL_TYPE_0 (TARGET_FPS * 3)   // Tire toutes les 3 secondes
#define ENEMY_FIRE_INTERVAL_TYPE_1 (TARGET_FPS * 5)   // Tire toutes les 5 secondes (plus lent car plus de PV)
#define ENEMY_FIRE_INTERVAL_TYPE_2 (TARGET_FPS * 2)   // Tire toutes les 2 secondes

#define ENEMY_SPEED_TYPE0 2
#define ENEMY_SPEED_TYPE1 3
#define ENEMY_SPEED_TYPE2 1

// --- Constantes Projectiles Joueur ---
#define MAX_PROJECTILES_JOUEUR 30
#define PROJECTILE_JOUEUR_SPEED 8
#define PROJECTILE_JOUEUR_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.6f)

// *** NOUVEAU: Constantes Projectiles Ennemi ***
#define MAX_PROJECTILES_ENNEMI 50 // Nombre max de projectiles ennemis à l'écran
#define PROJECTILE_ENNEMI_SPEED 5
#define PROJECTILE_ENNEMI_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.5f) // Ajustez si besoin

// --- Constantes Effets Visuels ---
#define MAX_EFFECTS 50
#define IMPACT_NBFRAMES 10
#define IMPACT_ANIM_SPEED 1

// --- Constantes Scrolling ---
#define SCROLL_SPEED 2
#define BACKGROUND_DISPLAY_SCALE 0.5f
// --- Constantes HUD ---
#define HUD_PROGRESS_ANIM_SPEED 2

// --- Constantes Sauvegarde ---
#define MAX_PLAYER_NAME_LEN 30

// --- Énumérations pour les États ---
typedef enum{
    STATE_IDLE,
    STATE_MOVING,
    STATE_HIT,
    STATE_DYING
}PlayerState;
typedef enum{
    ENEMY_STATE_MOVING,
    ENEMY_STATE_HIT,
    ENEMY_STATE_DYING
}EnemyState;
typedef enum {
    PROJECTILE_STATE_FLYING,
    PROJECTILE_STATE_IMPACTING,
    PROJECTILE_STATE_INACTIVE
}ProjectileState;
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_INIT,
    GAME_STATE_PLAYING,
    GAME_STATE_LEVEL_COMPLETE,
    GAME_STATE_GAME_OVER,
    GAME_STATE_GAME_WON,
    GAME_STATE_EXITING
} GameStateEnum;


// --- Structures ---
typedef struct {
    int x, y;
    int w, h;
    int active;
    int health;
    PlayerState state;
    int state_timer;
    BITMAP **current_sprites;
    int current_nb_frames;
    int imgcourante;
    int cptimg;
    int tmpimg;
} Vaisseau;

// *** MODIFIÉ: Structure Ennemi avec timers de tir ***
typedef struct {
    int x, y;
    int type;
    int health; // PV actuels
    int active;
    EnemyState state;
    int state_timer;
    BITMAP **current_sprites;
    int current_nb_frames;
    int imgcourante;
    int cptimg;
    int tmpimg;
    int w, h;
    // Nouveaux champs pour le tir
    int fire_timer;     // Compteur avant le prochain tir
    int fire_interval;  // Délai entre les tirs pour cet ennemi
} Ennemi;

typedef struct {
    int x, y;
    int active;
    ProjectileState state;
    int speed;
    BITMAP *sprite_flying;
    BITMAP **sprites_impact;
    int w, h;
    int impact_w, impact_h;
    int impact_frame;
    int impact_timer;
} Projectile; // Projectile du joueur

// *** NOUVEAU: Structure pour les projectiles ennemis ***
// (Similaire au projectile joueur, mais pourrait diverger plus tard)
typedef struct {
    int x, y;       // Coordonnées ÉCRAN
    int active;     // 1 = actif, 0 = inactif
    int speed;      // Vitesse de déplacement
    BITMAP *sprite; // Pointeur vers le sprite du projectile ennemi (dans GameState)
    int w, h;       // Dimensions
    // Pas d'animation d'impact pour l'instant pour simplifier
} ProjectileEnnemi;


typedef struct {
    int x, y;
    int active;
    int current_frame;
    int frame_timer;
    int anim_speed;
    int nb_frames;
    BITMAP **sprites;
    int w, h;
} Effect;


// --- Structure Principale du Jeu ---
typedef struct GameState {
    GameStateEnum current_game_state;
    int current_level;
    long game_loop_counter;
    char player_name[MAX_PLAYER_NAME_LEN + 1];
    int quit_game;

    BITMAP *buffer;
    BITMAP *decor;
    int scroll_x;
    int scroll_y;
    int couleur_magenta;
    int couleur_blanche;
    int screen_width_allegro;
    int screen_height_allegro;

    Vaisseau joueur;
    Ennemi ennemis[MAX_ENNEMIS];
    Projectile projectiles_joueur[MAX_PROJECTILES_JOUEUR];
    // *** NOUVEAU: Tableau pour les projectiles ennemis ***
    ProjectileEnnemi projectiles_ennemi[MAX_PROJECTILES_ENNEMI];
    Effect effects[MAX_EFFECTS];

    // Sprites Modèles
    BITMAP *sprites_player_idle_move[PLAYER_NBFRAMES_IDLE_MOVE];
    BITMAP *sprites_player_hit[PLAYER_NBFRAMES_HIT];
    BITMAP *sprites_player_death[PLAYER_NBFRAMES_DEATH];

    BITMAP *sprites_enemy0_move[ENEMY_NBFRAMES_MOVE];
    BITMAP *sprites_enemy0_hit[ENEMY_NBFRAMES_HIT];
    BITMAP *sprites_enemy0_death[ENEMY_NBFRAMES_DEATH];
    BITMAP *sprites_enemy1_move[ENEMY_NBFRAMES_MOVE];
    BITMAP *sprites_enemy1_hit[ENEMY_NBFRAMES_HIT];
    BITMAP *sprites_enemy1_death[ENEMY_NBFRAMES_DEATH];
    BITMAP *sprites_enemy2_move[ENEMY_NBFRAMES_MOVE];
    BITMAP *sprites_enemy2_hit[ENEMY_NBFRAMES_HIT];
    BITMAP *sprites_enemy2_death[ENEMY_NBFRAMES_DEATH];

    BITMAP *sprite_projectile_joueur;
    BITMAP *sprites_impact[IMPACT_NBFRAMES];
    // *** NOUVEAU: Sprite pour le projectile ennemi (un seul type pour l'instant) ***
    BITMAP *sprite_projectile_ennemi;


    BITMAP *barre_progression_vide;
    BITMAP *barre_progression_pleine;
    int hud_displayed_level_progress;

    int can_shoot;
    int shoot_timer;

} GameState;


#endif // DEFS_H
