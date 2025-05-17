#ifndef MENU_H
#define MENU_H

#include "defs.h" // Pour GameState et MAX_PLAYER_NAME_LEN, MAX_LEVEL

// --- Énumération interne pour les sous-états du menu (déplacée de menu.c pour être utilisable ici) ---
// (Si vous préférez la garder statique à menu.c, alors menu_set_active_substate n'est pas nécessaire
// et la logique de init_menu_resources se basera uniquement sur gameState->return_to_map_select)
typedef enum {
    MENU_SUBSTATE_MAIN,
    MENU_SUBSTATE_NEW_GAME_PSEUDO,
    MENU_SUBSTATE_RESUME_GAME_PSEUDO,
    MENU_SUBSTATE_MAP_SELECT,
    MENU_SUBSTATE_OPTIONS,
    MENU_SUBSTATE_SHOW_ERROR
} MenuSubState; // Rendre MenuSubState visible si menu_set_active_substate est utilisé


// --- Prototypes des Fonctions Publiques ---


int init_menu_resources(GameState *gameState);

void deinit_menu_resources();


void run_game_menu(GameState *gameState);


void menu_prepare_to_start_game(GameState *gameState, int level_to_load, const char *player_name);



void menu_update_and_save_progress(GameState *gameState, int completed_level_1_indexed, int stars_earned);


#endif // MENU_H
