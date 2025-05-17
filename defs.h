#ifndef DEFS_H
#define DEFS_H

#include <allegro.h>

// --- Constantes Générales ---
#define TARGET_FPS 66
#define LEVEL_DURATION_SECONDS 50
#define LEVEL_DURATION_FRAMES (LEVEL_DURATION_SECONDS * TARGET_FPS)
#define MAX_LEVEL 3

// --- Constantes Vaisseau Joueur ---
#define PLAYER_NBFRAMES_IDLE_MOVE 14
#define PLAYER_NBFRAMES_HIT 14
#define PLAYER_NBFRAMES_DEATH 22
#define PLAYER_ANIM_SPEED_MOVE 5
#define PLAYER_ANIM_SPEED_HIT 8
#define PLAYER_ANIM_SPEED_DEATH 10
#define PLAYER_HIT_DURATION (TARGET_FPS / 3)
#define PLAYER_SCALE_FACTOR 0.6f
#define PLAYER_SPEED 6
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

// Types d'ennemis
#define ENEMY_TYPE_0 0
#define ENEMY_TYPE_1 1
#define ENEMY_TYPE_2 2
#define ENEMY_TYPE_3 3
#define ENEMY_TYPE_4 4
#define ENEMY_TYPE_5 5
#define ENEMY_TYPE_6 6

#define MAX_ACTIVE_ENEMY_TYPE_4 2

// PV Ennemis
#define ENEMY_HP_TYPE_0 12
#define ENEMY_HP_TYPE_1 22
#define ENEMY_HP_TYPE_2 16
#define ENEMY_HP_TYPE_3 15
#define ENEMY_HP_TYPE_4 30
#define ENEMY_HP_TYPE_5 10
#define ENEMY_HP_TYPE_6 18

// Intervalles de tir (cadence)
#define ENEMY_FIRE_INTERVAL_TYPE_0 (int)(TARGET_FPS * 1.7)
#define ENEMY_FIRE_INTERVAL_TYPE_1 (int)(TARGET_FPS * 0.9)
#define ENEMY_FIRE_INTERVAL_TYPE_2 (int)(TARGET_FPS * 1.1)
#define ENEMY_FIRE_INTERVAL_TYPE_3 (int)(TARGET_FPS * 0.7)
#define ENEMY_FIRE_INTERVAL_TYPE_4 (int)(TARGET_FPS * 1.6)
#define ENEMY_FIRE_INTERVAL_TYPE_5 (TARGET_FPS * 1000)
#define ENEMY_FIRE_INTERVAL_TYPE_6 (int)(TARGET_FPS * 1.1)

// Vitesses des ennemis
#define ENEMY_SPEED_TYPE0 3
#define ENEMY_SPEED_TYPE1 5
#define ENEMY_SPEED_TYPE2 2
#define ENEMY_SPEED_X_TYPE3_WORLD 0
#define ENEMY_SPEED_Y_TYPE3 7
#define ENEMY_SPEED_X_TYPE5 1
#define ENEMY_SPEED_X_TYPE6 3

// --- Constantes Projectiles Joueur ---
#define MAX_PROJECTILES_JOUEUR 30
#define PROJECTILE_JOUEUR_SPEED 12
#define PROJECTILE_JOUEUR_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.6f)

// --- Constantes Projectiles Ennemi ---
#define MAX_PROJECTILES_ENNEMI 70
#define PROJECTILE_ENNEMI_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.5f) // Facteur d'échelle général pour les sprites de projectiles ennemis
#define PROJECTILE_ENNEMI_SPEED_SLOW 3
#define PROJECTILE_ENNEMI_SPEED_MEDIUM 6
#define PROJECTILE_ENNEMI_SPEED_FAST 9
#define PROJECTILE_ENNEMI_SPEED_VIOLET PROJECTILE_ENNEMI_SPEED_FAST

#define PROJECTILE_ENNEMI1_DAMAGE 1 // NOUVEAU
#define PROJECTILE_ENNEMI2_DAMAGE 1 // NOUVEAU (était 0.5, mis à 1 car HP sont int)

#define ENEMY0_WALL_PROJECTILE_COUNT 3
#define ENEMY0_WALL_PROJECTILE_SPREAD 15
#define ENEMY2_SPREAD_FACTOR_VS_HEIGHT 0.5f

// --- Constantes Items ---
#define MAX_ITEMS 10
#define ITEM_SIZE 20
#define ITEM_SCREEN_CLEAR_SPRITE_FILENAME "item_screen_clear.bmp"
#define ITEM_HEALTH_PACK_SPRITE_FILENAME "item_health.bmp"

// --- Constantes Effets Visuels ---
#define MAX_EFFECTS 50
#define IMPACT_NBFRAMES 10
#define IMPACT_ANIM_SPEED 1

// --- Constantes Scrolling ---
#define SCROLL_SPEED 3
#define BACKGROUND_DISPLAY_SCALE 0.5f

// --- Constantes HUD ---
#define HUD_PROGRESS_ANIM_SPEED 2

// --- Constantes Sauvegarde ---
#define MAX_PLAYER_NAME_LEN 30
#define SAVE_FILE_NAME "saves.txt"

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
    ITEM_TYPE_NONE,
    ITEM_TYPE_SCREEN_CLEAR,
    ITEM_TYPE_HEALTH_PACK
}ItemType;

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_LOAD_LEVEL,
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

typedef struct {
    int x, y;
    int type;
    int health;
    int active;
    EnemyState state;
    int state_timer;
    BITMAP **current_sprites;
    int current_nb_frames;
    int imgcourante;
    int cptimg;
    int tmpimg;
    int w, h;
    int fire_timer;
    int fire_interval;
    int speed_y;
    int direction_y;
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
} Projectile;

typedef struct {
    int x, y;
    int active;
    int speed;
    BITMAP *sprite; // Le sprite spécifique utilisé pour ce projectile
    int w, h;
    int damage; // NOUVEAU: Dégâts infligés par ce projectile
} ProjectileEnnemi;

typedef struct {
    int x_world, y_world;
    int w, h;
    int active;
    ItemType type;
    BITMAP *sprite;
} Item;

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
    BITMAP *decor_visuel;
    BITMAP *decor_obstacles;
    int scroll_x;
    int scroll_y;
    int couleur_magenta;
    int couleur_blanche;
    int screen_width_allegro;
    int screen_height_allegro;

    Vaisseau joueur;
    Ennemi ennemis[MAX_ENNEMIS];
    Projectile projectiles_joueur[MAX_PROJECTILES_JOUEUR];
    ProjectileEnnemi projectiles_ennemi[MAX_PROJECTILES_ENNEMI];
    Effect effects[MAX_EFFECTS];
    Item items[MAX_ITEMS];

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

    BITMAP *sprite_projectile_joueur; // Pour "tir_joueur.bmp"
    BITMAP *sprites_impact[IMPACT_NBFRAMES];
    BITMAP *sprite_projectile_ennemi1; // NOUVEAU: Pour "tire_ennemie1.bmp"
    BITMAP *sprite_projectile_ennemi2; // NOUVEAU: Pour "tire_ennemie2.bmp"

    BITMAP *sprite_item_screen_clear;
    BITMAP *sprite_item_health_pack;

    BITMAP *barre_progression_vide;
    BITMAP *barre_progression_pleine;
    int hud_displayed_level_progress;

    int can_shoot;
    int shoot_timer;

    int return_to_map_select;
} GameState;


#endif // DEFS_H
