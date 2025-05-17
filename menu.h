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

/**
 * @brief Initialise les ressources graphiques et l'état du menu.
 * @param gameState Pointeur vers l'état global du jeu.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int init_menu_resources(GameState *gameState);

/**
 * @brief Libère les ressources allouées pour le menu.
 */
void deinit_menu_resources();

/**
 * @brief Exécute la logique et le dessin du menu principal du jeu.
 * Gère les entrées utilisateur et les transitions entre les sous-états du menu.
 * Modifie gameState->current_game_state si une option de jeu est sélectionnée.
 * @param gameState Pointeur vers l'état global du jeu.
 */
void run_game_menu(GameState *gameState);

/**
 * @brief Prépare la transition du menu vers le jeu.
 * Configure gameState avec le nom du joueur et le niveau à charger.
 * Change gameState->current_game_state à GAME_STATE_LOAD_LEVEL.
 * @param gameState Pointeur vers l'état global du jeu.
 * @param level_to_load Numéro du niveau à charger (1-indexed).
 * @param player_name Nom du joueur.
 */
void menu_prepare_to_start_game(GameState *gameState, int level_to_load, const char *player_name);


/**
 * @brief Met à jour la progression du joueur après avoir terminé un niveau et sauvegarde.
 * @param gameState Pointeur vers l'état global du jeu (contient le nom du joueur).
 * @param completed_level_1_indexed Le numéro du niveau qui vient d'être terminé (1-indexed).
 * @param stars_earned Le nombre d'étoiles obtenues pour ce niveau (ex: 1 pour simple complétion).
 */
void menu_update_and_save_progress(GameState *gameState, int completed_level_1_indexed, int stars_earned);


#endif // MENU_H
