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

void dessiner_decor(GameState *gameState) {
    if (!gameState || !gameState->decor || !gameState->buffer) return;

    BITMAP *dest = gameState->buffer;
    BITMAP *decor_src = gameState->decor;
    int screen_w = gameState->screen_width_allegro;
    int screen_h = gameState->screen_height_allegro;

    // --- IMPORTANT ---
    // Définissez BACKGROUND_DISPLAY_SCALE dans votre fichier defs.h
    // Exemple pour un "dézoom x2" (voir deux fois plus de contenu) :
    // #define BACKGROUND_DISPLAY_SCALE 0.5f
    // Pour un affichage normal (pas de zoom/dézoom) :
    // #define BACKGROUND_DISPLAY_SCALE 1.0f
    // Pour un "zoom x2" (voir le contenu deux fois plus gros) :
    // #define BACKGROUND_DISPLAY_SCALE 2.0f
    #ifndef BACKGROUND_DISPLAY_SCALE
        // Valeur par défaut si non défini, pour éviter les erreurs de compilation.
        // Mettez la valeur souhaitée dans defs.h !
        #define BACKGROUND_DISPLAY_SCALE 1.0f
        // printf("WARN: BACKGROUND_DISPLAY_SCALE non défini, utilisation de 1.0f par défaut.\n");
    #endif

    float scale_factor = BACKGROUND_DISPLAY_SCALE;
    if (scale_factor <= 0.0f) { // Sécurité pour éviter division par zéro ou scale invalide
        scale_factor = 1.0f;
    }

    // Largeur et hauteur de la portion de l'image source que nous voulons afficher à l'écran.
    // Si scale_factor = 0.5 (dézoom x2), nous lisons une portion 2x plus large/haute de la source.
    int src_view_w = (int)(screen_w / scale_factor);
    int src_view_h = (int)(screen_h / scale_factor); // On utilise la pleine hauteur de la source, et on la scale à screen_h

    // On utilisera toujours la pleine hauteur de l'image de décor source et on la scalera à la hauteur de l'écran.
    // Cela est typique pour les fonds qui remplissent tout l'écran verticalement.
    int src_h_to_read = decor_src->h;


    // offset_x est la position de départ (coordonnée X) dans decor_src pour lire notre vue.
    // Elle boucle sur la largeur totale de decor_src.
    int offset_x = gameState->scroll_x;
    if (decor_src->w > 0) {
        offset_x %= decor_src->w;
        if (offset_x < 0) { // Gérer le cas où scroll_x pourrait devenir négatif (si SCROLL_SPEED < 0)
            offset_x += decor_src->w;
        }
    } else { // Pas de décor ou décor de largeur nulle
        clear_bitmap(dest); // Optionnel: effacer le buffer si pas de décor
        return;
    }

    // Pour l'instant, on ne gère pas de scrolling vertical complexe via scroll_y pour le scaling.
    // On prendra toujours depuis y=0 de la source pour la hauteur src_h_to_read.
    int src_y_to_read_from = 0;


    // Combien de pixels de largeur de l'image source sont disponibles à partir de offset_x jusqu'à la fin de l'image.
    int src_w_available_part1 = decor_src->w - offset_x;

    if (src_w_available_part1 >= src_view_w) {
        // Cas simple : toute la vue (src_view_w) est disponible en un seul morceau dans decor_src.
        stretch_blit(decor_src, dest,
                     offset_x, src_y_to_read_from,   // Source: (x,y)
                     src_view_w, src_h_to_read,      // Source: (largeur, hauteur) à lire
                     0, 0,                           // Destination: (x,y) sur le buffer
                     screen_w, screen_h);            // Destination: (largeur, hauteur) -> tout l'écran
    } else {
        // Cas complexe : la vue est coupée en deux par la fin de decor_src, il faut faire un raccord.

        // Partie 1: de offset_x jusqu'à la fin de decor_src.
        int src_w_read_part1 = src_w_available_part1;
        // Combien de largeur sur l'écran cette première partie va occuper après scaling.
        int dest_w_on_screen_part1 = (int)(src_w_read_part1 * scale_factor);

        if (src_w_read_part1 > 0 && dest_w_on_screen_part1 > 0) {
            stretch_blit(decor_src, dest,
                         offset_x, src_y_to_read_from,       // Source: (x,y)
                         src_w_read_part1, src_h_to_read,    // Source: (largeur, hauteur)
                         0, 0,                               // Destination: (x,y)
                         dest_w_on_screen_part1, screen_h);  // Destination: (largeur, hauteur)
        }


        // Partie 2: du début de decor_src pour combler ce qui manque à l'écran.
        // Combien de largeur de la source il nous manque pour compléter src_view_w.
        int src_w_needed_part2 = src_view_w - src_w_read_part1;
        // Combien de largeur il reste à remplir sur l'écran.
        // int dest_w_on_screen_part2 = screen_w - dest_w_on_screen_part1;
        // Il est plus précis de recalculer basé sur src_w_needed_part2 et scale_factor,
        // mais s'assurer que ça ne dépasse pas screen_w - dest_w_on_screen_part1.
        int dest_w_on_screen_part2 = (int)(src_w_needed_part2 * scale_factor);
        if (dest_w_on_screen_part1 + dest_w_on_screen_part2 > screen_w) { // Ajustement pour coller parfaitement
            dest_w_on_screen_part2 = screen_w - dest_w_on_screen_part1;
        }


        if (src_w_needed_part2 > 0 && dest_w_on_screen_part2 > 0) {
             // S'assurer qu'on ne lit pas plus que ce que la source peut offrir pour la partie 2 (même si c'est depuis le début)
            if(src_w_needed_part2 > decor_src->w) src_w_needed_part2 = decor_src->w;

            stretch_blit(decor_src, dest,
                         0, src_y_to_read_from,              // Source: (x,y) -> on repart du début de decor_src
                         src_w_needed_part2, src_h_to_read,  // Source: (largeur, hauteur)
                         dest_w_on_screen_part1, 0,          // Destination: (x,y) -> à côté de la partie 1
                         dest_w_on_screen_part2, screen_h);  // Destination: (largeur, hauteur)
        }
    }
}


void nettoyer_ressources_graphiques(GameState *gameState) {
     if (!gameState) return;
     printf("Nettoyage ressources graphiques...\n"); fflush(stdout);
    if (gameState->decor) {
        destroy_bitmap(gameState->decor);
        gameState->decor = NULL;
        printf("Decor détruit.\n"); fflush(stdout);
    }
    if (gameState->buffer) {
        destroy_bitmap(gameState->buffer);
        gameState->buffer = NULL;
        printf("Buffer détruit.\n"); fflush(stdout);
    }
}
