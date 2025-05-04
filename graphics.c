#include "graphics.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

// --- Variables Globales (Définies ici, déclarées extern dans defs.h) ---
// Note: Normalement, elles devraient être dans GameState, mais pour simplifier
// le debug de l'erreur de lien, laissons-les ici TEMPORAIREMENT.
// Une fois que ça compile, on les remettra dans GameState.
// BITMAP *buffer = NULL;
// BITMAP *decor = NULL;
// int scroll_x = 0;
// int scroll_y = 0;
// int couleur_magenta;
// int couleur_blanche;
// float current_scroll_speed = INITIAL_SCROLL_SPEED; // Commenté

// --- Implémentation des Fonctions ---

int initialisation_allegro(GameState *gameState) {
    if (!gameState) return -1;
    int ret;
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

    int requested_w = 800; // Test
    int requested_h = 600; // Test
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
    gameState->screen_width_allegro = SCREEN_W; // Globale Allegro
    gameState->screen_height_allegro = SCREEN_H; // Globale Allegro
    printf("   -> Dimensions écran Allegro réelles: %dx%d\n", gameState->screen_width_allegro, gameState->screen_height_allegro); fflush(stdout);

    printf("7. Tentative create_bitmap() pour le buffer (%dx%d)...\n", gameState->screen_width_allegro, gameState->screen_height_allegro); fflush(stdout);
    gameState->buffer = create_bitmap(gameState->screen_width_allegro, gameState->screen_height_allegro);
    if (!gameState->buffer) { /* ... gestion erreur ... */ exit(EXIT_FAILURE); }
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
        allegro_exit();
        exit(EXIT_FAILURE);
    }
    printf("SUCCES: Bitmap %s chargé (%dx%d).\n", nom_fichier, bmp->w, bmp->h);
    fflush(stdout);
    return bmp;
}

// *** Définition de la fonction de prétraitement ***
void make_white_transparent(GameState *gameState, BITMAP *bmp) {
    if (!gameState) { printf("ERREUR: gameState NULL dans make_white_transparent\n"); fflush(stdout); return; }
    if (!bmp) {
        printf("ERREUR: make_white_transparent appelé avec bmp NULL\n"); fflush(stdout);
        return;
    }
    printf("Traitement transparence blanche->magenta pour bitmap (%p) %dx%d...\n", (void*)bmp, bmp->w, bmp->h); fflush(stdout);

    int pixels_changed = 0;
    // Utiliser les couleurs stockées dans gameState
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
    printf("   -> %d pixels blancs remplacés par magenta.\n", pixels_changed); fflush(stdout);
}
// ------------------------------------------------------------

void mettre_a_jour_scrolling(GameState *gameState) {
    if (!gameState) return;
    gameState->scroll_x += SCROLL_SPEED;
    if (gameState->decor && gameState->decor->w > 0) {
        gameState->scroll_x %= gameState->decor->w;
        if (gameState->scroll_x < 0) gameState->scroll_x += gameState->decor->w;
    } else if (gameState->decor && gameState->scroll_x >= gameState->decor->w) {
         gameState->scroll_x -= gameState->decor->w;
    }
}

void dessiner_decor(GameState *gameState) {
    if (!gameState || !gameState->decor || !gameState->buffer) return;
    BITMAP *dest = gameState->buffer;
    BITMAP *decor_src = gameState->decor;
    int offset_x = gameState->scroll_x;
    int offset_y = gameState->scroll_y;
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;

    if (decor_src->w > 0) { offset_x %= decor_src->w; if (offset_x < 0) offset_x += decor_src->w; }
    else { offset_x = 0; }

    int largeur_partie1 = decor_src->w - offset_x;
    if (largeur_partie1 >= screen_w) { blit(decor_src, dest, offset_x, offset_y, 0, 0, screen_w, screen_h); }
    else {
        if (largeur_partie1 > 0) { blit(decor_src, dest, offset_x, offset_y, 0, 0, largeur_partie1, screen_h); }
        int largeur_partie2 = screen_w - largeur_partie1;
        if (largeur_partie2 > 0) { blit(decor_src, dest, 0, offset_y, largeur_partie1, 0, largeur_partie2, screen_h); }
    }
}

void nettoyer_ressources_graphiques(GameState *gameState) {
     if (!gameState) return;
     printf("Nettoyage ressources graphiques...\n"); fflush(stdout);
    if (gameState->decor) { destroy_bitmap(gameState->decor); gameState->decor = NULL; printf("Decor détruit.\n"); fflush(stdout); }
    if (gameState->buffer) { destroy_bitmap(gameState->buffer); gameState->buffer = NULL; printf("Buffer détruit.\n"); fflush(stdout); }
}
