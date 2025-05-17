#ifndef DEFS_H
#define DEFS_H

#include <allegro.h>

// --- Constantes Générales ---
#define TARGET_FPS 60 // Un peu plus standard
#define LEVEL_DURATION_SECONDS 50 // Durée d'un niveau en secondes (pas utilisé directement)
#define LEVEL_DURATION_FRAMES (LEVEL_DURATION_SECONDS * TARGET_FPS) // Calculé si besoin
#define MAX_LEVEL 3

// --- Vaisseau Joueur ---
#define PLAYER_NBFRAMES_IDLE_MOVE 14
#define PLAYER_NBFRAMES_HIT 14
#define PLAYER_NBFRAMES_DEATH 22
#define PLAYER_ANIM_SPEED_MOVE 5
#define PLAYER_ANIM_SPEED_HIT 8
#define PLAYER_ANIM_SPEED_DEATH 10
#define PLAYER_HIT_DURATION (TARGET_FPS / 3) // Temps où le joueur est touché
#define PLAYER_SCALE_FACTOR 0.6f
#define PLAYER_SPEED 6
#define PLAYER_INITIAL_HP 5
#define PLAYER_INVINCIBILITY_DURATION (TARGET_FPS * 2) // Temps d'invincibilité après un coup

// --- Ennemis ---
#define MAX_ENNEMIS 50
#define ENEMY_NBFRAMES_MOVE 18
#define ENEMY_NBFRAMES_HIT 18
#define ENEMY_NBFRAMES_DEATH 18
#define ENEMY_ANIM_SPEED_MOVE 8
#define ENEMY_ANIM_SPEED_HIT 6
#define ENEMY_ANIM_SPEED_DEATH 1 // Animation de mort rapide
#define ENEMY_HIT_DURATION 3 // Frames où l'ennemi montre qu'il est touché

// Types d'ennemis
#define ENEMY_TYPE_0 0
#define ENEMY_TYPE_1 1
#define ENEMY_TYPE_2 2
#define ENEMY_TYPE_3 3 // Celui qui bouge en Y
#define ENEMY_TYPE_4 4 // L'immobile qui tire
#define ENEMY_TYPE_5 5 // Celui qui ne tire pas et drop un item
#define ENEMY_TYPE_6 6 // Le violet

#define MAX_ACTIVE_ENEMY_TYPE_4 2 // Limite pour le type 4 à l'écran

// Points de vie des Ennemis
#define ENEMY_HP_TYPE_0 12
#define ENEMY_HP_TYPE_1 22
#define ENEMY_HP_TYPE_2 16
#define ENEMY_HP_TYPE_3 15
#define ENEMY_HP_TYPE_4 30
#define ENEMY_HP_TYPE_5 10 // Facile à tuer
#define ENEMY_HP_TYPE_6 18

// Cadence de tir (plus la valeur est haute, plus c'est lent)
#define ENEMY_FIRE_INTERVAL_TYPE_0 (int)(TARGET_FPS * 2.5)
#define ENEMY_FIRE_INTERVAL_TYPE_1 (int)(TARGET_FPS * 1.5)
#define ENEMY_FIRE_INTERVAL_TYPE_2 (int)(TARGET_FPS * 1.8)
#define ENEMY_FIRE_INTERVAL_TYPE_3 (int)(TARGET_FPS * 1.0) // Rapide
#define ENEMY_FIRE_INTERVAL_TYPE_4 (int)(TARGET_FPS * 2.0)
#define ENEMY_FIRE_INTERVAL_TYPE_5 (TARGET_FPS * 1000)      // Ne tire quasi jamais
#define ENEMY_FIRE_INTERVAL_TYPE_6 (int)(TARGET_FPS * 1.5)

// Vitesses des ennemis
#define ENEMY_SPEED_TYPE0 3
#define ENEMY_SPEED_TYPE1 5
#define ENEMY_SPEED_TYPE2 2
#define ENEMY_SPEED_X_TYPE3_WORLD 0 // Se déplace avec le scroll + son mouvement Y
#define ENEMY_SPEED_Y_TYPE3 7
#define ENEMY_SPEED_X_TYPE5 1 // Un peu lent
#define ENEMY_SPEED_X_TYPE6 3

// --- Projectiles Joueur ---
#define MAX_PROJECTILES_JOUEUR 30
#define PROJECTILE_JOUEUR_SPEED 12
#define PROJECTILE_JOUEUR_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.6f)

// --- Projectiles Ennemi ---
#define MAX_PROJECTILES_ENNEMI 80
#define PROJECTILE_ENNEMI_SCALE_FACTOR (PLAYER_SCALE_FACTOR * 0.5f)
#define PROJECTILE_ENNEMI_SPEED_SLOW 3
#define PROJECTILE_ENNEMI_SPEED_MEDIUM 6
#define PROJECTILE_ENNEMI_SPEED_FAST 9
#define PROJECTILE_ENNEMI_SPEED_VIOLET PROJECTILE_ENNEMI_SPEED_FAST
#define PROJECTILE_ENNEMI1_DAMAGE 1
#define PROJECTILE_ENNEMI2_DAMAGE 1 // Pour certains ennemis

#define ENEMY0_WALL_PROJECTILE_COUNT 3 // Nombre de tirs pour l'ennemi 0
#define ENEMY0_WALL_PROJECTILE_SPREAD 15 // Écart entre les tirs
#define ENEMY2_SPREAD_FACTOR_VS_HEIGHT 0.5f // Pour l'écart des tirs de l'ennemi 2

// --- Items ---
#define MAX_ITEMS 10
#define ITEM_SIZE 20 // Taille pour la collision, pas forcément le sprite
#define ITEM_SCREEN_CLEAR_SPRITE_FILENAME "item_screen_clear.bmp"
#define ITEM_HEALTH_PACK_SPRITE_FILENAME "item_health.bmp"

// --- Boss ---
#define BOSS_INITIAL_HP 200
#define BOSS_HP_PHASE_2_TRIGGER (BOSS_INITIAL_HP / 2) // Seuil pour la phase 2
#define BOSS_PHASE1_ATTACK_COUNT 5 // Nombre d'attaques "mur" en phase 1
#define BOSS_PHASE1_ATTACK_INTERVAL (TARGET_FPS * 2)
#define BOSS_PHASE1_PROJECTILE_SPEED PROJECTILE_ENNEMI_SPEED_MEDIUM
#define BOSS_PHASE1_PROJECTILE_DAMAGE 1
#define BOSS_PHASE1_WALL_GAP_SIZE 3 // Taille du trou dans le mur de projectiles
#define BOSS_PHASE2_ATTACK_INTERVAL (int)(TARGET_FPS * 1.5)
#define BOSS_PHASE2_PROJECTILE_SPEED PROJECTILE_ENNEMI_SPEED_MEDIUM
#define BOSS_PHASE2_PROJECTILE_DAMAGE 1


// --- Effets Visuels ---
#define MAX_EFFECTS 50
#define IMPACT_NBFRAMES 10 // Nombre de frames pour l'animation d'impact
#define IMPACT_ANIM_SPEED 1 // Vitesse de l'animation d'impact

// --- Scrolling ---
#define SCROLL_SPEED 3
#define BACKGROUND_DISPLAY_SCALE 0.5f // Pour un effet de parallaxe ou juste pour adapter la taille

// --- HUD (Interface) ---
#define HUD_PROGRESS_ANIM_SPEED 2

// --- Sauvegarde ---
#define MAX_PLAYER_NAME_LEN 30
#define SAVE_FILE_NAME "sauvegarde.dat" // Un nom un peu plus classique

// --- Énumérations ---
typedef enum { STATE_IDLE, STATE_MOVING, STATE_HIT, STATE_DYING } PlayerState;
typedef enum { ENEMY_STATE_MOVING, ENEMY_STATE_HIT, ENEMY_STATE_DYING } EnemyState;
typedef enum { PROJECTILE_STATE_FLYING, PROJECTILE_STATE_IMPACTING, PROJECTILE_STATE_INACTIVE } ProjectileState;
typedef enum { ITEM_TYPE_NONE, ITEM_TYPE_SCREEN_CLEAR, ITEM_TYPE_HEALTH_PACK } ItemType;
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_LOAD_LEVEL,
    GAME_STATE_PLAYING,
    GAME_STATE_LEVEL_COMPLETE,
    GAME_STATE_GAME_OVER,
    GAME_STATE_GAME_WON,
    GAME_STATE_EXITING
} GameStateEnum;

typedef enum {
    BOSS_PHASE_NONE,        // Pas encore là ou vaincu
    BOSS_PHASE_1_ATTACKING, // Phase 1: attaque et invulnérable
    BOSS_PHASE_2_VULNERABLE // Phase 2: peut être touché, attaque plus simple
} BossPhase;

// --- Structures ---
typedef struct {
    int x, y, w, h, active;
    int health;
    PlayerState state;
    int state_timer; // Pour gérer la durée d'un état (ex: invincibilité, hit)
    int current_nb_frames; // Nombre de frames de l'animation actuelle
    int imgcourante; // Frame actuelle de l'animation
    int cptimg; // Compteur pour la vitesse d'animation
    int tmpimg; // Seuil pour la vitesse d'animation (ancien nom, un peu obscur)
    BITMAP **current_sprites; // Tableau des sprites pour l'animation actuelle
} Vaisseau;

typedef struct {
    int x, y, w, h, active;
    int type;
    int health;
    EnemyState state;
    int state_timer;
    int current_nb_frames;
    int imgcourante;
    int cptimg;
    int tmpimg;
    int fire_timer; // Compteur avant le prochain tir
    int fire_interval; // Délai entre les tirs
    int speed_y; // Vitesse verticale (pour type 3)
    int direction_y; // Direction verticale (pour type 3)
    BITMAP **current_sprites;
} Ennemi;

typedef struct {
    int x, y, w, h, active;
    int speed;
    ProjectileState state; // Pour gérer l'impact
    BITMAP *sprite_flying;
    // Pour l'animation d'impact (si on en fait une spécifique au projectile)
    BITMAP **sprites_impact;
    int impact_w, impact_h;
    int impact_frame, impact_timer;
} Projectile;

typedef struct {
    int x, y, w, h, active;
    int speed;
    int damage; // Dégâts du projectile
    BITMAP *sprite;
} ProjectileEnnemi;

typedef struct {
    int x_world, y_world, w, h, active; // Coordonnées dans le monde
    ItemType type;
    BITMAP *sprite;
} Item;

typedef struct {
    int x, y, w, h;
    int active;
    int health;
    int max_health; // Pour la barre de vie
    BossPhase current_phase;
    int phase1_attack_counter;
    int phase1_attack_timer;
    int phase2_attack_timer;
    int has_triggered_mid_phase_switch; // Pour le changement de phase à mi-vie
    BITMAP *sprite_phase1;
    BITMAP *sprite_phase2;
    BITMAP *current_sprite; // Sprite actuel du boss
} Boss;

typedef struct {
    int x, y, w, h, active;
    int current_frame, frame_timer, anim_speed, nb_frames;
    BITMAP **sprites; // Tableau de sprites pour l'effet
} Effect;

// --- Structure Principale du Jeu ---
typedef struct GameState {
    GameStateEnum current_game_state;
    int current_level;
    long game_loop_counter; // Compteur de la boucle principale, pour le temps
    char player_name[MAX_PLAYER_NAME_LEN + 1];
    int quit_game; // Drapeau pour quitter le jeu

    // Allegro et graphismes
    BITMAP *buffer; // Double buffering
    BITMAP *decor_visuel; // Image de fond
    BITMAP *decor_obstacles; // Image pour les collisions avec le décor (si besoin)
    int scroll_x; // Position du scrolling horizontal
    int scroll_y_dummy; // Scrolling vertical (pas vraiment utilisé ici, mais pourrait l'être)
    int couleur_magenta; // Pour la transparence
    int couleur_blanche;
    int screen_width_allegro;
    int screen_height_allegro;

    // Entités du jeu
    Vaisseau joueur;
    Ennemi ennemis[MAX_ENNEMIS];
    Projectile projectiles_joueur[MAX_PROJECTILES_JOUEUR];
    ProjectileEnnemi projectiles_ennemi[MAX_PROJECTILES_ENNEMI];
    Effect effects[MAX_EFFECTS];
    Item items[MAX_ITEMS];
    Boss boss; // Le boss

    // Sprites pré-chargés
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
    // Ajouter d'autres types d'ennemis si animés

    BITMAP *sprite_projectile_joueur;
    BITMAP *sprites_impact[IMPACT_NBFRAMES]; // Animation d'impact générique
    BITMAP *sprite_projectile_ennemi1; // Projectile ennemi standard
    BITMAP *sprite_projectile_ennemi2; // Autre projectile ennemi

    BITMAP *sprite_item_screen_clear;
    BITMAP *sprite_item_health_pack;

    // HUD
    BITMAP *barre_progression_vide;
    BITMAP *barre_progression_pleine;
    int hud_displayed_level_progress; // Pour l'animation de la barre

    // Contrôles joueur
    int can_shoot; // Pour la cadence de tir du joueur
    int shoot_timer;

    // Logique de jeu
    int return_to_map_select; // Si on doit retourner au menu de sélection
    int boss_fight_active; // Si le combat contre le boss est en cours

} GameState;


#endif // DEFS_H
