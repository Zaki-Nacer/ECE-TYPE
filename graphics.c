#include "graphics.h"
#include "defs.h" // Pour GameState et potentiellement BACKGROUND_DISPLAY_SCALE
#include <stdio.h>
#include <stdlib.h>

// --- Implémentation des Fonctions ---

int initialisation_allegro(GameState *gameState) {
    if (!gameState) return -1;
    // int ret; // Variable non utilisée
    printf("1. Tentative allegro_init()...\n"); fflush(stdout);
    if (allegro_init() != 0) { fprintf(stderr, "ERREUR FATALE: allegro_init() a échoué !\n"); return -1; }
    printf("   -> allegro_init() OK.\n"); fflush(stdout);

    printf("2. Tentative install_keyboard()...\n"); fflush(stdout);
    if (install_keyboard() != 0) { fprintf(stderr, "ERREUR FATALE: install_keyboard() a échoué !\n"); return -1; }
    printf("   -> install_keyboard() OK.\n"); fflush(stdout);

    printf("3. Tentative install_mouse()... [COMMENTED OUT]\n"); fflush(stdout);
    /* if (install_mouse() != 0) { ... } */
    printf("   -> install_mouse() SKIPPED.\n"); fflush(stdout);

    printf("4. Tentative install_timer()...\n"); fflush(stdout);
     if (install_timer() != 0) { fprintf(stderr, "ERREUR FATALE: install_timer() a échoué !\n"); return -1; }
    printf("   -> install_timer() OK.\n"); fflush(stdout);

    printf("5. Tentative set_color_depth()...\n"); fflush(stdout);
    set_color_depth(desktop_color_depth());
    printf("   -> set_color_depth() OK.\n"); fflush(stdout);

    int requested_w = 1920;
    int requested_h = 1080;
    printf("6. Tentative set_gfx_mode(%dx%d, WINDOWED)...\n", requested_w, requested_h); fflush(stdout);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, requested_w, requested_h, 0, 0) != 0) {
        printf("   -> GFX_AUTODETECT_WINDOWED (%dx%d) échoué. Tentative GFX_SAFE...\n", requested_w, requested_h); fflush(stdout);
        if (set_gfx_mode(GFX_SAFE, requested_w, requested_h, 0, 0) != 0) {
             allegro_message("ERREUR FATALE: Impossible d'initialiser le mode graphique (%dx%d).", requested_w, requested_h);
             return -1;
        }
         printf("   -> GFX_SAFE OK.\n"); fflush(stdout);
    } else {
        printf("   -> GFX_AUTODETECT_WINDOWED OK.\n"); fflush(stdout);
    }

    // Stocker les dimensions réelles
    gameState->screen_width_allegro = SCREEN_W;
    gameState->screen_height_allegro = SCREEN_H;
    printf("   -> Dimensions écran Allegro réelles: %dx%d\n", gameState->screen_width_allegro, gameState->screen_height_allegro); fflush(stdout);

    printf("7. Tentative create_bitmap() pour le buffer (%dx%d)...\n", gameState->screen_width_allegro, gameState->screen_height_allegro); fflush(stdout);
    gameState->buffer = create_bitmap(gameState->screen_width_allegro, gameState->screen_height_allegro);
    if (!gameState->buffer) {
        allegro_message("ERREUR FATALE: Impossible de créer le buffer graphique !");
        exit(EXIT_FAILURE);
    }
    clear_bitmap(gameState->buffer);
    printf("   -> Buffer créé (%dx%d).\n", gameState->buffer->w, gameState->buffer->h); fflush(stdout);

    printf("8. Définition couleurs...\n"); fflush(stdout);
    gameState->couleur_magenta = makecol(255, 0, 255);
    gameState->couleur_blanche = makecol(255, 255, 255);
    printf("   -> Couleurs OK (Blanc = %d, Magenta = %d).\n", gameState->couleur_blanche, gameState->couleur_magenta); fflush(stdout);

    printf("--- Initialisation Allegro terminée avec succès ---\n"); fflush(stdout);
    return 0;
}


BITMAP *charger_bitmap_safe(const char *nom_fichier) {
    printf("Tentative de chargement bitmap: %s\n", nom_fichier); fflush(stdout);
    BITMAP *bmp = load_bitmap(nom_fichier, NULL);
    if (!bmp) {
        char msg[100];
        sprintf(msg, "ERREUR FATALE: Impossible de charger: %s", nom_fichier);
        allegro_message(msg);
        allegro_exit(); // Quitter Allegro proprement avant de terminer le programme
        exit(EXIT_FAILURE);
    }
    printf("SUCCES: Bitmap %s chargé (%dx%d).\n", nom_fichier, bmp->w, bmp->h);
    fflush(stdout);
    return bmp;
}

void make_white_transparent(GameState *gameState, BITMAP *bmp) {
    if (!gameState) { printf("ERREUR: gameState NULL dans make_white_transparent\n"); fflush(stdout); return; }
    if (!bmp) {
        printf("ERREUR: make_white_transparent appelé avec bmp NULL\n"); fflush(stdout);
        return;
    }
    // printf("Traitement transparence blanche->magenta pour bitmap (%p) %dx%d...\n", (void*)bmp, bmp->w, bmp->h); fflush(stdout); // Un peu verbeux

    int pixels_changed = 0;
    int white_color = gameState->couleur_blanche;
    int magenta_color = gameState->couleur_magenta;

    for (int y = 0; y < bmp->h; y++) {
        for (int x = 0; x < bmp->w; x++) {
            if (getpixel(bmp, x, y) == white_color) {
                putpixel(bmp, x, y, magenta_color);
                pixels_changed++;
            }
        }
    }
    // printf("   -> %d pixels blancs remplacés par magenta.\n", pixels_changed); fflush(stdout); // Un peu verbeux
}

void mettre_a_jour_scrolling(GameState *gameState) {
    if (!gameState) return;
    gameState->scroll_x += SCROLL_SPEED; // SCROLL_SPEED est défini dans defs.h

    // Le modulo pour gameState->scroll_x sera géré dans dessiner_decor
    // car la largeur effective sur laquelle le scroll boucle dépendra du decor_src->w.
    // Si on voulait que scroll_x ne dépasse jamais decor_src->w:
    // if (gameState->decor && gameState->decor->w > 0) {
    //     if (gameState->scroll_x >= gameState->decor->w) {
    //         gameState->scroll_x -= gameState->decor->w;
    //     }
    //     // Gérer aussi le cas où scroll_x deviendrait négatif si SCROLL_SPEED pouvait être négatif
    // }
}

// À placer dans graphics.c
// Cette fonction remplace l'ancienne dessiner_decor
void dessiner_decor_layer(GameState *gameState, BITMAP *decor_a_dessiner) {
    if (!gameState || !decor_a_dessiner || !gameState->buffer) return;

    BITMAP *dest = gameState->buffer;
    BITMAP *decor_src = decor_a_dessiner; // Utilise le bitmap passé en argument
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;

    #ifndef BACKGROUND_DISPLAY_SCALE
        #define BACKGROUND_DISPLAY_SCALE 1.0f
    #endif

    float scale_factor = BACKGROUND_DISPLAY_SCALE;
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    int src_view_w = (int)(screen_w / scale_factor);
    int src_h_to_read = decor_src->h;
    int src_y_to_read_from = 0; // Pas de scrolling vertical complexe pour le moment

    int offset_x = gameState->scroll_x;
    if (decor_src->w > 0) {
        offset_x %= decor_src->w;
        if (offset_x < 0) {
            offset_x += decor_src->w;
        }
    } else {
        return;
    }

    int src_w_available_part1 = decor_src->w - offset_x;

    if (src_w_available_part1 >= src_view_w) {
        stretch_blit(decor_src, dest,
                     offset_x, src_y_to_read_from,
                     src_view_w, src_h_to_read,
                     0, 0,
                     screen_w, screen_h);
    } else {
        int src_w_read_part1 = src_w_available_part1;
        int dest_w_on_screen_part1 = (int)(src_w_read_part1 * scale_factor);

        if (src_w_read_part1 > 0 && dest_w_on_screen_part1 > 0) {
            stretch_blit(decor_src, dest,
                         offset_x, src_y_to_read_from,
                         src_w_read_part1, src_h_to_read,
                         0, 0,
                         dest_w_on_screen_part1, screen_h);
        }

        int src_w_needed_part2 = src_view_w - src_w_read_part1;
        int dest_w_on_screen_part2 = (int)(src_w_needed_part2 * scale_factor);
        if (dest_w_on_screen_part1 + dest_w_on_screen_part2 > screen_w) {
            dest_w_on_screen_part2 = screen_w - dest_w_on_screen_part1;
        }

        if (src_w_needed_part2 > 0 && dest_w_on_screen_part2 > 0) {
            if(src_w_needed_part2 > decor_src->w) src_w_needed_part2 = decor_src->w;

            stretch_blit(decor_src, dest,
                         0, src_y_to_read_from,
                         src_w_needed_part2, src_h_to_read,
                         dest_w_on_screen_part1, 0,
                         dest_w_on_screen_part2, screen_h);
        }
    }
}

// Dans graphics.c
void nettoyer_ressources_graphiques(GameState *gameState) {
    if (!gameState) return;
    printf("Nettoyage ressources graphiques...\n"); fflush(stdout);

    if (gameState->decor_visuel) { // NOUVEAU
        destroy_bitmap(gameState->decor_visuel);
        gameState->decor_visuel = NULL;
        printf("Decor visuel détruit.\n"); fflush(stdout);
    }
    if (gameState->decor_obstacles) { // NOUVEAU
        destroy_bitmap(gameState->decor_obstacles);
        gameState->decor_obstacles = NULL;
        printf("Decor obstacle détruit.\n"); fflush(stdout);
    }

    if (gameState->buffer) {
        destroy_bitmap(gameState->buffer);
        gameState->buffer = NULL;
        printf("Buffer détruit.\n"); fflush(stdout);
    }
}
