#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "defs.h"
#include "menu.h"
#include "graphics.h"

// --- Définition de la structure Point ---
typedef struct { int x; int y; } Point;

// --- Menu specific assets (static to this file) ---
static BITMAP *menu_bg_image = NULL;
static BITMAP *menu_your_score_box = NULL;
static BITMAP *menu_fond_map = NULL;
static BITMAP *menu_sprites_niveaux[5] = {NULL}; // 0-3 stars, 4 lock
static BITMAP *menu_btn_idle = NULL;
static BITMAP *menu_btn_selected = NULL;
static FONT *menu_font_global = NULL;

// --- Menu state variables (static to this file) ---
static MenuSubState current_menu_substate = MENU_SUBSTATE_MAIN;
static char menu_player_name_input[MAX_PLAYER_NAME_LEN + 1] = "";
static int menu_player_name_input_idx = 0;
static char menu_current_player_profile_name[MAX_PLAYER_NAME_LEN + 1] = "";
static int menu_level_progress[MAX_LEVEL];
static int map_selected_level_idx = 0;
static int main_menu_current_selection = 0;

static char error_message_line1[100];
static char error_message_line2[100];

static int menu_bg_scroll_x_pos = 0;
static int key_debounce_timer = 0;
#define KEY_DEBOUNCE_FRAMES 5

// proto des fonctions
static void draw_scrolling_background(BITMAP *buffer, GameState *gameState);
static void draw_main_menu_ui(BITMAP *buffer, GameState *gameState);
static void handle_main_menu_input(GameState *gameState);
static void draw_pseudo_input_ui(BITMAP *buffer, GameState *gameState, const char *title);
static void handle_pseudo_input(GameState *gameState);
static void draw_map_selection_ui(BITMAP *buffer, GameState *gameState);
static void handle_map_selection_input(GameState *gameState);
static void draw_options_ui(BITMAP *buffer, GameState *gameState);
static void handle_options_input(GameState *gameState);
static void draw_error_message_ui(BITMAP *buffer, GameState *gameState);
static void handle_error_message_input(GameState *gameState);

static int load_player_progress_menu(const char *player_name, int progress_array[MAX_LEVEL]);
static void save_player_progress_menu(const char *player_name, const int progress_array[MAX_LEVEL]);
static void create_new_player_profile(const char *player_name);
static int key_pressed_allegro(int allegro_key_code);


// --- Public Functions ---

int init_menu_resources(GameState *gameState) {
    printf("Initialisation des ressources du menu...\n");
    fflush(stdout);

    if (gameState->screen_width_allegro <= 0 || gameState->screen_height_allegro <= 0) {
        allegro_message("Menu Error: Screen dimensions not set in gameState.");
        return -1;
    }

    if (!menu_bg_image) menu_bg_image = charger_bitmap_safe("background.bmp");
    if (!menu_your_score_box) menu_your_score_box = charger_bitmap_safe("YourScoreBox.bmp");
    if (!menu_fond_map) menu_fond_map = charger_bitmap_safe("background_map.bmp");
    if (!menu_sprites_niveaux[0]) menu_sprites_niveaux[0] = charger_bitmap_safe("MapLevel_0star.bmp");
    if (!menu_sprites_niveaux[1]) menu_sprites_niveaux[1] = charger_bitmap_safe("MapLevel_1star.bmp");
    if (!menu_sprites_niveaux[2]) menu_sprites_niveaux[2] = charger_bitmap_safe("MapLevel_2star.bmp");
    if (!menu_sprites_niveaux[3]) menu_sprites_niveaux[3] = charger_bitmap_safe("MapLevel_3star.bmp");
    if (!menu_sprites_niveaux[4]) menu_sprites_niveaux[4] = charger_bitmap_safe("MapLevel_Lock.bmp");
    if (!menu_btn_idle) menu_btn_idle = charger_bitmap_safe("OrangeBtn1.bmp");
    if (!menu_btn_selected) menu_btn_selected = charger_bitmap_safe("RedBtn2.bmp");

    menu_font_global = font;

    if (!menu_bg_image || !menu_btn_idle || !menu_btn_selected || !menu_sprites_niveaux[0] || !menu_fond_map || !menu_your_score_box) {
        allegro_message("Erreur: Une ou plusieurs images du menu n'ont pas pu être chargées.");
        return -1;
    }

    main_menu_current_selection = 0;
    menu_player_name_input_idx = 0;
    menu_player_name_input[0] = '\0';

    if (gameState->return_to_map_select) {
        printf("Menu: Retour à la carte des niveaux pour le joueur: %s\n", gameState->player_name);
        fflush(stdout);
        current_menu_substate = MENU_SUBSTATE_MAP_SELECT;
        strncpy(menu_current_player_profile_name, gameState->player_name, MAX_PLAYER_NAME_LEN);
        menu_current_player_profile_name[MAX_PLAYER_NAME_LEN] = '\0';

        if (!load_player_progress_menu(menu_current_player_profile_name, menu_level_progress)) {
            printf("AVERTISSEMENT: Profil '%s' non trouvé lors du retour à la carte. Création d'un nouveau profil.\n", menu_current_player_profile_name);
            create_new_player_profile(menu_current_player_profile_name);
            load_player_progress_menu(menu_current_player_profile_name, menu_level_progress);
        }

        map_selected_level_idx = gameState->current_level; // current_level est 1-indexed après complétion
                                                       // donc pour l'index 0-based, c'est current_level -1
                                                       // Si on veut sélectionner le NIVEAU SUIVANT :
        if (gameState->current_level < MAX_LEVEL) {
             map_selected_level_idx = gameState->current_level; // Sélectionne l'icône du niveau suivant (index 0-based)
        } else {
             map_selected_level_idx = MAX_LEVEL - 1; // Sélectionne le dernier niveau
        }

        if (map_selected_level_idx < 0) map_selected_level_idx = 0; // Sécurité

        gameState->return_to_map_select = 0;
    } else {
        current_menu_substate = MENU_SUBSTATE_MAIN;
        menu_current_player_profile_name[0] = '\0';
        map_selected_level_idx = 0;
    }

    key_debounce_timer = 0;
    printf("Ressources du menu initialisées. Sous-état du menu: %d\n", current_menu_substate);
    fflush(stdout);
    return 0;
}

void deinit_menu_resources() {
    printf("Désinitialisation des ressources du menu...\n");
    fflush(stdout);

    if (menu_bg_image) { destroy_bitmap(menu_bg_image); menu_bg_image = NULL; }
    if (menu_your_score_box) { destroy_bitmap(menu_your_score_box); menu_your_score_box = NULL; }
    if (menu_fond_map) { destroy_bitmap(menu_fond_map); menu_fond_map = NULL; }
    for (int i = 0; i < 5; i++) {
        if (menu_sprites_niveaux[i]) { destroy_bitmap(menu_sprites_niveaux[i]); menu_sprites_niveaux[i] = NULL; }
    }
    if (menu_btn_idle) { destroy_bitmap(menu_btn_idle); menu_btn_idle = NULL; }
    if (menu_btn_selected) { destroy_bitmap(menu_btn_selected); menu_btn_selected = NULL; }

    printf("Ressources du menu désinitialisées.\n");
    fflush(stdout);
}

void run_game_menu(GameState *gameState) {
    if (!gameState || !gameState->buffer) return;

    if (key_debounce_timer > 0) {
        key_debounce_timer--;
    } else {
        switch (current_menu_substate) {
            case MENU_SUBSTATE_MAIN:
                handle_main_menu_input(gameState);
                break;
            case MENU_SUBSTATE_NEW_GAME_PSEUDO:
            case MENU_SUBSTATE_RESUME_GAME_PSEUDO:
                handle_pseudo_input(gameState);
                break;
            case MENU_SUBSTATE_MAP_SELECT:
                handle_map_selection_input(gameState);
                break;
            case MENU_SUBSTATE_OPTIONS:
                handle_options_input(gameState);
                break;
            case MENU_SUBSTATE_SHOW_ERROR:
                handle_error_message_input(gameState);
                break;
        }
    }

    clear_to_color(gameState->buffer, makecol(20, 20, 30));
    draw_scrolling_background(gameState->buffer, gameState);

    switch (current_menu_substate) {
        case MENU_SUBSTATE_MAIN:
            draw_main_menu_ui(gameState->buffer, gameState);
            break;
        case MENU_SUBSTATE_NEW_GAME_PSEUDO:
            draw_pseudo_input_ui(gameState->buffer, gameState, "NOUVELLE PARTIE: ENTREZ PSEUDO");
            break;
        case MENU_SUBSTATE_RESUME_GAME_PSEUDO:
            draw_pseudo_input_ui(gameState->buffer, gameState, "REPRENDRE: ENTREZ PSEUDO");
            break;
        case MENU_SUBSTATE_MAP_SELECT:
            draw_map_selection_ui(gameState->buffer, gameState);
            break;
        case MENU_SUBSTATE_OPTIONS:
            draw_options_ui(gameState->buffer, gameState);
            break;
        case MENU_SUBSTATE_SHOW_ERROR:
            draw_error_message_ui(gameState->buffer, gameState);
            break;
    }
}


void menu_prepare_to_start_game(GameState *gameState, int level_to_load, const char *player_name) {
    printf("Menu: Préparation pour démarrer le niveau %d pour %s\n", level_to_load, player_name);
    fflush(stdout);
    strncpy(gameState->player_name, player_name, MAX_PLAYER_NAME_LEN);
    gameState->player_name[MAX_PLAYER_NAME_LEN] = '\0';
    gameState->current_level = level_to_load;

    deinit_menu_resources();
    gameState->current_game_state = GAME_STATE_LOAD_LEVEL;
}

void menu_update_and_save_progress(GameState *gameState, int completed_level_1_indexed, int stars_earned) {
    if (completed_level_1_indexed < 1 || completed_level_1_indexed > MAX_LEVEL) {
        printf("ERREUR: Tentative de sauvegarde pour un niveau invalide: %d\n", completed_level_1_indexed);
        return;
    }
    if (stars_earned < 0) stars_earned = 0;
    if (stars_earned > 3) stars_earned = 3;

    int current_player_level_progress[MAX_LEVEL];

    if (!load_player_progress_menu(gameState->player_name, current_player_level_progress)) {
        printf("Profil '%s' non trouvé lors de la mise à jour de la progression. Utilisation de la progression par défaut.\n", gameState->player_name);
    }

    if (stars_earned > current_player_level_progress[completed_level_1_indexed - 1]) {
        current_player_level_progress[completed_level_1_indexed - 1] = stars_earned;
    } else if (current_player_level_progress[completed_level_1_indexed - 1] < 1 && stars_earned >=1 ) {
         current_player_level_progress[completed_level_1_indexed - 1] = stars_earned;
    }

    if (completed_level_1_indexed < MAX_LEVEL) {
        if (current_player_level_progress[completed_level_1_indexed] == -1) {
            current_player_level_progress[completed_level_1_indexed] = 0;
            printf("Niveau %d débloqué pour %s.\n", completed_level_1_indexed + 1, gameState->player_name);
        }
    }

    save_player_progress_menu(gameState->player_name, current_player_level_progress);
    printf("Progression mise à jour et sauvegardée pour %s après niveau %d (étoiles: %d).\n",
           gameState->player_name, completed_level_1_indexed, current_player_level_progress[completed_level_1_indexed - 1]);
    fflush(stdout);
}


// --- Static Helper Implementations ---

static void draw_scrolling_background(BITMAP *buffer, GameState *gameState) {
    if (!menu_bg_image) return;
    menu_bg_scroll_x_pos -= 1;
    if (menu_bg_scroll_x_pos <= -menu_bg_image->w) {
        menu_bg_scroll_x_pos += menu_bg_image->w;
    }
    blit(menu_bg_image, buffer, -menu_bg_scroll_x_pos, 0, 0, 0, menu_bg_image->w, menu_bg_image->h);
    blit(menu_bg_image, buffer, -menu_bg_scroll_x_pos + menu_bg_image->w, 0, 0, 0, menu_bg_image->w, menu_bg_image->h);
}

static void draw_main_menu_ui(BITMAP *buffer, GameState *gameState) {
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;
    int btn_w = 300, btn_h = 70;
    int spacing = 20;
    char *main_menu_labels[] = {"Nouvelle Partie", "Reprendre Partie", "Options", "Quitter"};
    int main_menu_options_count = sizeof(main_menu_labels) / sizeof(main_menu_labels[0]);

    int total_h = main_menu_options_count * btn_h + (main_menu_options_count - 1) * spacing;
    int start_y = (screen_h - total_h) / 2;
    int start_x = (screen_w - btn_w) / 2;

    textout_centre_ex(buffer, menu_font_global, "R-TYPE LIKE GAME", screen_w / 2, start_y - 80, makecol(255, 255, 0), -1);

    for (int i = 0; i < main_menu_options_count; i++) {
        BITMAP *current_btn_sprite = (i == main_menu_current_selection) ? menu_btn_selected : menu_btn_idle;
        int y_pos = start_y + i * (btn_h + spacing);
        if (current_btn_sprite) {
            stretch_sprite(buffer, current_btn_sprite, start_x, y_pos, btn_w, btn_h);
        } else {
            rectfill(buffer, start_x, y_pos, start_x + btn_w, y_pos + btn_h, (i == main_menu_current_selection) ? makecol(200,0,0) : makecol(0,0,200));
        }
        textout_centre_ex(buffer, menu_font_global, main_menu_labels[i],
                          start_x + btn_w / 2, y_pos + btn_h / 2 - text_height(menu_font_global) / 2,
                          makecol(255, 255, 255), -1);
    }
}

static void handle_main_menu_input(GameState *gameState) {
    int main_menu_options_count = 4;
    if (key_pressed_allegro(KEY_DOWN)) {
        main_menu_current_selection = (main_menu_current_selection + 1) % main_menu_options_count;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    } else if (key_pressed_allegro(KEY_UP)) {
        main_menu_current_selection = (main_menu_current_selection - 1 + main_menu_options_count) % main_menu_options_count;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    } else if (key_pressed_allegro(KEY_ENTER)) {
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
        switch (main_menu_current_selection) {
            case 0:
                current_menu_substate = MENU_SUBSTATE_NEW_GAME_PSEUDO;
                menu_player_name_input_idx = 0;
                menu_player_name_input[0] = '\0';
                break;
            case 1:
                current_menu_substate = MENU_SUBSTATE_RESUME_GAME_PSEUDO;
                menu_player_name_input_idx = 0;
                menu_player_name_input[0] = '\0';
                break;
            case 2:
                current_menu_substate = MENU_SUBSTATE_OPTIONS;
                break;
            case 3:
                gameState->current_game_state = GAME_STATE_EXITING;
                break;
        }
    } else if (key_pressed_allegro(KEY_ESC)) {
         gameState->current_game_state = GAME_STATE_EXITING;
         key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    }
}


static void draw_pseudo_input_ui(BITMAP *buffer, GameState *gameState, const char *title) {
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;

    textout_centre_ex(buffer, menu_font_global, title, screen_w / 2, screen_h / 2 - 100, makecol(255,255,255), -1);

    if (menu_your_score_box) {
        int box_x = screen_w / 2 - menu_your_score_box->w / 2;
        int box_y = screen_h / 2 - 50;
        draw_sprite(buffer, menu_your_score_box, box_x, box_y);

        char display_text[MAX_PLAYER_NAME_LEN + 2];
        sprintf(display_text, "%s_", menu_player_name_input);
        textout_centre_ex(buffer, menu_font_global, display_text,
                          box_x + menu_your_score_box->w / 2,
                          box_y + menu_your_score_box->h / 2 - text_height(menu_font_global)/2,
                          makecol(255,255,255), -1);
    } else {
        rectfill(buffer, screen_w/2 - 150, screen_h/2 - 25, screen_w/2 + 150, screen_h/2 + 25, makecol(50,50,100));
        char display_text[MAX_PLAYER_NAME_LEN + 2];
        sprintf(display_text, "%s_", menu_player_name_input);
        textout_centre_ex(buffer, menu_font_global, display_text, screen_w / 2, screen_h / 2, makecol(255,255,255), -1);
    }
    textout_centre_ex(buffer, menu_font_global, "ENTREE pour valider, ECHAP pour annuler.", screen_w / 2, screen_h / 2 + 50, makecol(200,200,200), -1);
    textout_centre_ex(buffer, menu_font_global, "Max 30 caracteres. BACKSPACE pour effacer.", screen_w / 2, screen_h / 2 + 80, makecol(200,200,200), -1);
}

static void handle_pseudo_input(GameState *gameState) {
    if (key_pressed_allegro(KEY_ESC)) {
        current_menu_substate = MENU_SUBSTATE_MAIN;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
        return;
    }

    if (keypressed()) {
        int key_allegro = readkey();
        int scancode = key_allegro >> 8;
        int ascii_char = key_allegro & 0xFF;

        if (scancode == KEY_ENTER) {
            if (menu_player_name_input_idx > 0) {
                key_debounce_timer = KEY_DEBOUNCE_FRAMES;
                strncpy(menu_current_player_profile_name, menu_player_name_input, MAX_PLAYER_NAME_LEN);
                menu_current_player_profile_name[MAX_PLAYER_NAME_LEN] = '\0';

                if (current_menu_substate == MENU_SUBSTATE_NEW_GAME_PSEUDO) {
                    if (load_player_progress_menu(menu_current_player_profile_name, menu_level_progress)) {
                        strcpy(error_message_line1, "Ce pseudo existe deja.");
                        strcpy(error_message_line2, "Utilisez 'Reprendre' ou un autre pseudo.");
                        current_menu_substate = MENU_SUBSTATE_SHOW_ERROR;
                    } else {
                        create_new_player_profile(menu_current_player_profile_name);
                        load_player_progress_menu(menu_current_player_profile_name, menu_level_progress);
                        map_selected_level_idx = 0;
                        current_menu_substate = MENU_SUBSTATE_MAP_SELECT;
                    }
                } else {
                    if (load_player_progress_menu(menu_current_player_profile_name, menu_level_progress)) {
                        map_selected_level_idx = 0;
                        current_menu_substate = MENU_SUBSTATE_MAP_SELECT;
                    } else {
                        strcpy(error_message_line1, "Aucune sauvegarde pour ce pseudo.");
                        strcpy(error_message_line2, "Creez une 'Nouvelle Partie'.");
                        current_menu_substate = MENU_SUBSTATE_SHOW_ERROR;
                    }
                }
            }
        } else if (scancode == KEY_BACKSPACE) {
            if (menu_player_name_input_idx > 0) {
                menu_player_name_input_idx--;
                menu_player_name_input[menu_player_name_input_idx] = '\0';
            }
        } else if (ascii_char >= 32 && ascii_char <= 126) {
            if (menu_player_name_input_idx < MAX_PLAYER_NAME_LEN) {
                menu_player_name_input[menu_player_name_input_idx++] = ascii_char;
                menu_player_name_input[menu_player_name_input_idx] = '\0';
            }
        }
    }
}


static void draw_map_selection_ui(BITMAP *buffer, GameState *gameState) {
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;

    if (menu_fond_map) {
        draw_sprite(buffer, menu_fond_map, screen_w/2 - menu_fond_map->w/2, screen_h/2 - menu_fond_map->h/2);
    } else {
        rectfill(buffer, 0, 0, screen_w, screen_h, makecol(30,50,30));
    }

    textprintf_centre_ex(buffer, menu_font_global, screen_w / 2, 40, makecol(255, 255, 255), -1, "Profil : %s", menu_current_player_profile_name);
    textprintf_centre_ex(buffer, menu_font_global, screen_w / 2, screen_h - 60, makecol(200,200,200), -1, "FLECHES G/D pour choisir, ENTREE pour jouer, ECHAP pour menu");

    Point level_icon_positions[MAX_LEVEL] = {
        {screen_w / 2 - 200, screen_h / 2},
        {screen_w / 2 , screen_h / 2 - 50},
        {screen_w / 2 + 200, screen_h / 2}
    };

    for (int i = 0; i < MAX_LEVEL; i++) {
        int star_idx = menu_level_progress[i];
        if (star_idx < -1 || star_idx > 3) star_idx = -1;

        BITMAP *level_sprite_to_draw = menu_sprites_niveaux[star_idx == -1 ? 4 : star_idx];

        if (level_sprite_to_draw) {
            int pos_x = level_icon_positions[i].x - level_sprite_to_draw->w / 2;
            int pos_y = level_icon_positions[i].y - level_sprite_to_draw->h / 2;
            float scale = (i == map_selected_level_idx) ? 1.2f : 1.0f;
            int scaled_w = (int)(level_sprite_to_draw->w * scale);
            int scaled_h = (int)(level_sprite_to_draw->h * scale);

            stretch_sprite(buffer, level_sprite_to_draw,
                           pos_x - (scaled_w - level_sprite_to_draw->w)/2,
                           pos_y - (scaled_h - level_sprite_to_draw->h)/2,
                           scaled_w, scaled_h);
        } else {
            int color = (star_idx == -1) ? makecol(100,100,100) : makecol(0,150,0);
            if (i == map_selected_level_idx) color = makecol(255,255,0);
            rectfill(buffer, level_icon_positions[i].x - 20, level_icon_positions[i].y - 20,
                     level_icon_positions[i].x + 20, level_icon_positions[i].y + 20, color);
            textprintf_ex(buffer, menu_font_global, level_icon_positions[i].x-5, level_icon_positions[i].y-5, makecol(0,0,0), -1, "%d", i+1);
        }
    }
    textprintf_centre_ex(buffer, menu_font_global, screen_w/2, screen_h - 90, makecol(255,255,0), -1, "NIVEAU %d SELECTIONNE", map_selected_level_idx + 1);
}

static void handle_map_selection_input(GameState *gameState) {
    if (key_pressed_allegro(KEY_ESC)) {
        current_menu_substate = MENU_SUBSTATE_MAIN;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
        return;
    }
    if (key_pressed_allegro(KEY_LEFT)) {
        map_selected_level_idx = (map_selected_level_idx - 1 + MAX_LEVEL) % MAX_LEVEL;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    } else if (key_pressed_allegro(KEY_RIGHT)) {
        map_selected_level_idx = (map_selected_level_idx + 1) % MAX_LEVEL;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    } else if (key_pressed_allegro(KEY_ENTER)) {
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
        if (menu_level_progress[map_selected_level_idx] != -1) {
            menu_prepare_to_start_game(gameState, map_selected_level_idx + 1, menu_current_player_profile_name);
        } else {
            strcpy(error_message_line1, "Niveau Verrouille!");
            strcpy(error_message_line2, "Terminez les niveaux precedents.");
            current_menu_substate = MENU_SUBSTATE_SHOW_ERROR;
        }
    }
}

static void draw_options_ui(BITMAP *buffer, GameState *gameState) {
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;
    textout_centre_ex(buffer, menu_font_global, "OPTIONS", screen_w / 2, screen_h / 2 - 50, makecol(255,255,255), -1);
    textout_centre_ex(buffer, menu_font_global, "Options non implementees.", screen_w / 2, screen_h / 2, makecol(200,200,200), -1);
    textout_centre_ex(buffer, menu_font_global, "Appuyez sur ECHAP pour revenir.", screen_w / 2, screen_h / 2 + 50, makecol(200,200,200), -1);
}

static void handle_options_input(GameState *gameState) {
    if (key_pressed_allegro(KEY_ESC) || key_pressed_allegro(KEY_ENTER)) {
        current_menu_substate = MENU_SUBSTATE_MAIN;
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    }
}

static void draw_error_message_ui(BITMAP *buffer, GameState *gameState) {
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;
    rectfill(buffer, screen_w/2 - 250, screen_h/2 - 70, screen_w/2 + 250, screen_h/2 + 70, makecol(30,30,30));
    rect(buffer, screen_w/2 - 250, screen_h/2 - 70, screen_w/2 + 250, screen_h/2 + 70, makecol(200,0,0));
    textout_centre_ex(buffer, menu_font_global, "ERREUR", screen_w / 2, screen_h / 2 - 50, makecol(255,100,100), -1);
    textout_centre_ex(buffer, menu_font_global, error_message_line1, screen_w / 2, screen_h / 2 - 20, makecol(255,255,255), -1);
    textout_centre_ex(buffer, menu_font_global, error_message_line2, screen_w / 2, screen_h / 2 + 0, makecol(255,255,255), -1);
    textout_centre_ex(buffer, menu_font_global, "(Appuyez sur ENTREE ou ECHAP)", screen_w / 2, screen_h / 2 + 30, makecol(180,180,180), -1);
}

static void handle_error_message_input(GameState *gameState) {
     if (key_pressed_allegro(KEY_ESC) || key_pressed_allegro(KEY_ENTER)) {
        if (strstr(error_message_line1, "pseudo") != NULL || strstr(error_message_line2, "pseudo") != NULL) {
             if (strstr(error_message_line1, "existe deja") != NULL) {
                current_menu_substate = MENU_SUBSTATE_NEW_GAME_PSEUDO;
             } else {
                current_menu_substate = MENU_SUBSTATE_RESUME_GAME_PSEUDO;
             }
        } else if (strstr(error_message_line1, "Verrouille") != NULL) {
            current_menu_substate = MENU_SUBSTATE_MAP_SELECT;
        }
        else {
            current_menu_substate = MENU_SUBSTATE_MAIN;
        }
        key_debounce_timer = KEY_DEBOUNCE_FRAMES;
    }
}

// Reverted to sscanf for compatibility with MinGW
static int load_player_progress_menu(const char *player_name, int progress_array[MAX_LEVEL]) {
    FILE *file = fopen(SAVE_FILE_NAME, "r");
    if (!file) {
        printf("Fichier de sauvegarde '%s' non trouvé. Initialisation par défaut.\n", SAVE_FILE_NAME);
        progress_array[0] = 0;
        for (int i = 1; i < MAX_LEVEL; i++) progress_array[i] = -1;
        return 0;
    }

    char line[256];
    char name_in_file[MAX_PLAYER_NAME_LEN + 1];
    int temp_progress[MAX_LEVEL]; // Tableau temporaire pour sscanf
    int found = 0;
    int items_scanned = 0;

    while (fgets(line, sizeof(line), file)) {
        // Construire la chaîne de format pour sscanf en fonction de MAX_LEVEL
        // Cette approche est plus simple que strtok_r et devrait fonctionner avec MinGW
        // mais est moins flexible si MAX_LEVEL change souvent et devient très grand.
        // Pour MAX_LEVEL = 3, c'est gérable.
        if (MAX_LEVEL == 1) {
            items_scanned = sscanf(line, "%s %d", name_in_file, &temp_progress[0]);
        } else if (MAX_LEVEL == 2) {
            items_scanned = sscanf(line, "%s %d %d", name_in_file, &temp_progress[0], &temp_progress[1]);
        } else if (MAX_LEVEL == 3) {
            items_scanned = sscanf(line, "%s %d %d %d", name_in_file, &temp_progress[0], &temp_progress[1], &temp_progress[2]);
        } else {
            // Gérer d'autres cas ou mettre une erreur si MAX_LEVEL est inattendu
            printf("ERREUR: MAX_LEVEL (%d) non géré dans load_player_progress_menu sscanf.\n", MAX_LEVEL);
            items_scanned = 0; // Pour ne pas entrer dans la condition de succès
        }

        if (items_scanned == (MAX_LEVEL + 1) && strcmp(player_name, name_in_file) == 0) {
            for(int i=0; i<MAX_LEVEL; ++i) {
                progress_array[i] = temp_progress[i];
            }
            found = 1;
            break;
        }
    }
    fclose(file);

    if (!found) {
        printf("Profil '%s' non trouvé dans '%s'. Initialisation par défaut.\n", player_name, SAVE_FILE_NAME);
        progress_array[0] = 0;
        for (int i = 1; i < MAX_LEVEL; i++) progress_array[i] = -1;
        return 0;
    }
    printf("Progression chargée pour %s.\n", player_name); fflush(stdout);
    return 1;
}


static void save_player_progress_menu(const char *player_name, const int progress_array[MAX_LEVEL]) {
    FILE *file_in = fopen(SAVE_FILE_NAME, "r");
    FILE *file_out = fopen("savetemp.txt", "w");

    if (!file_out) {
        perror("Erreur ouverture fichier sauvegarde temporaire");
        if (file_in) fclose(file_in);
        return;
    }

    char line[256];
    char name_in_file[MAX_PLAYER_NAME_LEN + 1];
    int found_and_updated = 0;

    if (file_in) {
        while (fgets(line, sizeof(line), file_in)) {
            if (sscanf(line, "%s", name_in_file) == 1 && strcmp(player_name, name_in_file) == 0) {
                fprintf(file_out, "%s", player_name);
                for (int i = 0; i < MAX_LEVEL; i++) {
                    fprintf(file_out, " %d", progress_array[i]);
                }
                fprintf(file_out, "\n");
                found_and_updated = 1;
            } else {
                fprintf(file_out, "%s", line);
            }
        }
        fclose(file_in);
    }

    if (!found_and_updated) {
        fprintf(file_out, "%s", player_name);
        for (int i = 0; i < MAX_LEVEL; i++) {
            fprintf(file_out, " %d", progress_array[i]);
        }
        fprintf(file_out, "\n");
    }

    fclose(file_out);

    if (remove(SAVE_FILE_NAME) != 0 && file_in != NULL) {
         // perror("Erreur suppression ancien fichier de sauvegarde");
    }
    if (rename("savetemp.txt", SAVE_FILE_NAME) != 0) {
         perror("Erreur renommage fichier de sauvegarde temporaire");
    }
    printf("Progression sauvegardee pour %s.\n", player_name); fflush(stdout);
}

static void create_new_player_profile(const char *player_name) {
    int initial_progress[MAX_LEVEL];
    initial_progress[0] = 0;
    for (int i = 1; i < MAX_LEVEL; i++) {
        initial_progress[i] = -1;
    }
    save_player_progress_menu(player_name, initial_progress);
    printf("Nouveau profil cree pour %s. Niveau 1 débloqué.\n", player_name); fflush(stdout);
}

static int key_pressed_allegro(int allegro_key_code) {
    if (key[allegro_key_code]) {
        return 1;
    }
    return 0;
}
