#ifndef DEFS_H
#define DEFS_H

#include <allegro.h>

// --- Constantes Générales ---
#define TARGET_FPS 66
#define LEVEL_DURATION_SECONDS 50
#define LEVEL_DURATION_FRAMES (LEVEL_DURATION_SECONDS * TARGET_FPS)
#define MAX_LEVEL 3

// --- Constantes Vaisseau Joueur ---
#define PLAYER_NBFRAMES_IDLE_MOVE 4 // Nombre total de frames (idle=0, move=1+)
#define PLAYER_NBFRAMES_HIT 2       // Nombre frames anim HIT
#define PLAYER_NBFRAMES_DEATH 6     // Nombre frames anim DEATH
#define PLAYER_ANIM_SPEED_MOVE 5
#define PLAYER_ANIM_SPEED_HIT 8
#define PLAYER_ANIM_SPEED_DEATH 10
#define PLAYER_HIT_DURATION (TARGET_FPS / 3) // Durée état HIT (env 1/3 sec)
#define PLAYER_SCALE_FACTOR 0.375f
#define PLAYER_SPEED 4
#define PLAYER_INITIAL_HP 5 // Points de vie initiaux
#define PLAYER_INVINCIBILITY_DURATION (TARGET_FPS * 2) // 2 secondes d'invincibilité après HIT

// --- Constantes Ennemis ---
#define MAX_ENNEMIS 50
#define ENEMY_NBFRAMES_MOVE 4
#define ENEMY_NBFRAMES_HIT 2
#define ENEMY_NBFRAMES_DEATH 5 // *** CORRIGÉ: Nombre de frames pour l'animation de mort ***
#define ENEMY_ANIM_SPEED_MOVE 8
#define ENEMY_ANIM_SPEED_HIT 6
#define ENEMY_ANIM_SPEED_DEATH 10
#define ENEMY_HIT_DURATION (TARGET_FPS / 4) // Durée état HIT ennemi
#define ENEMY_HP 2 // Points de vie des ennemis (uniforme pour simplifier)
#define ENEMY_SPEED_TYPE0 2
#define ENEMY_SPEED_TYPE1 3
#define ENEMY_SPEED_TYPE2 1

// --- Constantes Projectiles Joueur ---
#define MAX_PROJECTILES_JOUEUR 30
#define PROJECTILE_JOUEUR_SPEED 8
#define PROJECTILE_JOUEUR_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.6f)

// --- Constantes Effets Visuels ---
#define MAX_EFFECTS 50
#define IMPACT_NBFRAMES 4
#define IMPACT_ANIM_SPEED 4

// --- Constantes Scrolling ---
#define SCROLL_SPEED 2

// --- Constantes HUD ---
#define HUD_PROGRESS_ANIM_SPEED 2

// --- Constantes Sauvegarde ---
#define MAX_PLAYER_NAME_LEN 30

// --- Énumérations pour les États ---
typedef enum { STATE_IDLE, STATE_MOVING, STATE_HIT, STATE_DYING } PlayerState;
typedef enum { ENEMY_STATE_MOVING, ENEMY_STATE_HIT, ENEMY_STATE_DYING } EnemyState;
// *** MODIFIÉ: Ajout état projectile ***
typedef enum { PROJECTILE_STATE_FLYING, PROJECTILE_STATE_IMPACTING, PROJECTILE_STATE_INACTIVE } ProjectileState; // Ajout INACTIVE
typedef enum { GAME_STATE_MENU, GAME_STATE_INIT, GAME_STATE_PLAYING, GAME_STATE_LEVEL_COMPLETE, GAME_STATE_GAME_OVER, GAME_STATE_GAME_WON, GAME_STATE_EXITING } GameStateEnum;


// --- Structures ---
typedef struct {
    int x, y;
    int w, h;
    int active;
    int health;
    PlayerState state;
    int state_timer; // Timer pour HIT et invincibilité

    BITMAP **current_sprites;
    int current_nb_frames;
    int imgcourante;
    int cptimg;
    int tmpimg; // Vitesse de l'animation actuelle
} Vaisseau;

typedef struct {
    int x, y; // Coordonnées MONDE
    int type;
    int health;
    int active; // 1 = actif, 0 = inactif/mort
    EnemyState state;
    int state_timer; // Timer pour état HIT

    BITMAP **current_sprites; // Pointeur vers le tableau de sprites actuel (dans GameState)
    int current_nb_frames; // Nombre de frames de l'animation actuelle
    int imgcourante; // Index de la frame actuelle
    int cptimg; // Compteur pour la vitesse d'animation
    int tmpimg; // Vitesse de l'animation actuelle (délai entre frames)
    int w, h; // Dimensions (basées sur la frame actuelle)
} Ennemi;

// *** MODIFIÉ: Structure Projectile avec état et animation d'impact ***
typedef struct {
    int x, y; // Coordonnées ECRAN
    int active; // 1 = actif, 0 = inactif
    ProjectileState state; // État actuel du projectile
    int speed; // Vitesse de déplacement (si FLYING)

    // Sprites (pointeurs vers GameState)
    BITMAP *sprite_flying;
    BITMAP **sprites_impact; // Tableau pour l'animation d'impact

    // Dimensions
    int w, h; // Dimensions en vol
    int impact_w, impact_h; // Dimensions de l'impact

    // Animation d'impact
    int impact_frame; // Frame actuelle de l'impact
    int impact_timer; // Timer pour l'animation d'impact
} Projectile;

typedef struct {
    int x, y; // Coordonnées ECRAN
    int active;
    int current_frame;
    int frame_timer;
    int anim_speed;
    int nb_frames;
    BITMAP **sprites; // Pointeur vers le tableau de sprites (dans GameState)
    int w, h;
} Effect;


// --- Structure Principale du Jeu ---
typedef struct GameState {
    // État général
    GameStateEnum current_game_state;
    int current_level;
    long game_loop_counter;
    char player_name[MAX_PLAYER_NAME_LEN + 1];
    int quit_game;

    // Ressources Graphiques
    BITMAP *buffer;
    BITMAP *decor;
    int scroll_x; // Position gauche du décor visible dans le monde
    int scroll_y;
    int couleur_magenta;
    int couleur_blanche;
    int screen_width_allegro;
    int screen_height_allegro;

    // Entités du jeu
    Vaisseau joueur;
    Ennemi ennemis[MAX_ENNEMIS];
    Projectile projectiles_joueur[MAX_PROJECTILES_JOUEUR];
    Effect effects[MAX_EFFECTS];

    // Sprites Modèles (redimensionnés et traités, stockés ici)
    // Joueur
    BITMAP *sprites_player_idle_move[PLAYER_NBFRAMES_IDLE_MOVE];
    BITMAP *sprites_player_hit[PLAYER_NBFRAMES_HIT];
    BITMAP *sprites_player_death[PLAYER_NBFRAMES_DEATH];
    // Ennemis (Type 0)
    BITMAP *sprites_enemy0_move[ENEMY_NBFRAMES_MOVE];
    BITMAP *sprites_enemy0_hit[ENEMY_NBFRAMES_HIT];
    // *** CORRIGÉ: Utilisation de la bonne constante pour la taille du tableau death ***
    BITMAP *sprites_enemy0_death[ENEMY_NBFRAMES_DEATH];
    // Ennemis (Type 1)
    BITMAP *sprites_enemy1_move[ENEMY_NBFRAMES_MOVE];
    BITMAP *sprites_enemy1_hit[ENEMY_NBFRAMES_HIT];
    BITMAP *sprites_enemy1_death[ENEMY_NBFRAMES_DEATH];
    // Ennemis (Type 2)
    BITMAP *sprites_enemy2_move[ENEMY_NBFRAMES_MOVE];
    BITMAP *sprites_enemy2_hit[ENEMY_NBFRAMES_HIT];
    BITMAP *sprites_enemy2_death[ENEMY_NBFRAMES_DEATH];
    // Projectile Joueur
    BITMAP *sprite_projectile_joueur; // Sprite pour l'état FLYING
    BITMAP *sprites_impact[IMPACT_NBFRAMES]; // Sprites pour l'animation d'impact (utilisé aussi par Effect)

    // HUD
    BITMAP *barre_progression_vide;
    BITMAP *barre_progression_pleine;
    int hud_displayed_level_progress; // Pour l'animation de la barre

    // Contrôles et Timers
    int can_shoot; // Flag pour limiter la cadence de tir
    int shoot_timer; // Compteur pour la cadence de tir

} GameState;


#endif // DEFS_H
