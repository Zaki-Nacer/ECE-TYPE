#include "hud.h"
#include "graphics.h"
#include <stdio.h>
#include <math.h>
#define HUD_PROGRESS_ANIM_SPEED 2

// --- Implémentation des Fonctions ---

void charger_ressources_hud(GameState *gameState) {
    if (!gameState) { /* ... */ return; }
    BITMAP *original_vide = NULL;
    BITMAP *temp_pleine = NULL;

    printf("Chargement ressources HUD...\n"); fflush(stdout);
    original_vide = charger_bitmap_safe("barre_vide.bmp");
    temp_pleine = charger_bitmap_safe("barre_remplie.bmp");

    if (original_vide->w != temp_pleine->w || original_vide->h != temp_pleine->h)
    {
        printf("Redimensionnement de barre_vide (%dx%d) vers (%dx%d)...\n",
               original_vide->w, original_vide->h, temp_pleine->w, temp_pleine->h); fflush(stdout);
        gameState->barre_progression_vide = create_bitmap(temp_pleine->w, temp_pleine->h);
        if (!gameState->barre_progression_vide) { /* ... */ exit(EXIT_FAILURE); }

        // *** CORRIGÉ: Appel correct à stretch_blit (10 arguments) ***
        stretch_blit(original_vide, gameState->barre_progression_vide,
#include "hud.h"
#include "defs.h"
#include "graphics.h"
#include <stdio.h>
#include <math.h>

// --- Implémentation des Fonctions ---

void charger_ressources_hud(GameState *gameState) {
    if (!gameState) { /* ... */ return; }
    BITMAP *original_vide = NULL;
    BITMAP *temp_pleine = NULL;

    printf("Chargement ressources HUD...\n"); fflush(stdout);
    original_vide = charger_bitmap_safe("barre_vide.bmp");
    temp_pleine = charger_bitmap_safe("barre_remplie.bmp");

    if (original_vide->w != temp_pleine->w || original_vide->h != temp_pleine->h)
    {
        printf("Redimensionnement de barre_vide (%dx%d) vers (%dx%d)...\n",
               original_vide->w, original_vide->h, temp_pleine->w, temp_pleine->h); fflush(stdout);
        gameState->barre_progression_vide = create_bitmap(temp_pleine->w, temp_pleine->h);
        if (!gameState->barre_progression_vide) { /* ... */ exit(EXIT_FAILURE); }

        stretch_blit(original_vide, gameState->barre_progression_vide,
                     0, 0, original_vide->w, original_vide->h, // Source
                     0, 0, temp_pleine->w, temp_pleine->h);    // Destination

        destroy_bitmap(original_vide);
        original_vide = NULL;
    } else {
        printf("Les barres HUD ont déjà les mêmes dimensions.\n"); fflush(stdout);
        gameState->barre_progression_vide = original_vide;
        original_vide = NULL;
    }
    gameState->barre_progression_pleine = temp_pleine;

    printf("Ressources HUD chargées et harmonisées.\n"); fflush(stdout);
}

void mettre_a_jour_hud(GameState *gameState) {
     if (!gameState) return;
    if (gameState->hud_displayed_level_progress == -1) { gameState->hud_displayed_level_progress = 0; }
    long target_progress = gameState->game_loop_counter;
    if (target_progress > LEVEL_DURATION_FRAMES) target_progress = LEVEL_DURATION_FRAMES;
    if (target_progress < 0) target_progress = 0;
    if (gameState->hud_displayed_level_progress < target_progress) {
        gameState->hud_displayed_level_progress += HUD_PROGRESS_ANIM_SPEED;
        if (gameState->hud_displayed_level_progress > target_progress) { gameState->hud_displayed_level_progress = (int)target_progress; }
    }
     if (gameState->hud_displayed_level_progress > LEVEL_DURATION_FRAMES) { gameState->hud_displayed_level_progress = LEVEL_DURATION_FRAMES; }
}

void dessiner_barre_progression(GameState *gameState, BITMAP *dest, int x, int y, int value, int max_value) {
    if (!gameState || !dest || !gameState->barre_progression_vide || !gameState->barre_progression_pleine) return;
    BITMAP *barre_vide = gameState->barre_progression_vide;
    BITMAP *barre_pleine = gameState->barre_progression_pleine;
    blit(barre_vide, dest, 0, 0, x, y, barre_vide->w, barre_vide->h);
    int full_width = barre_pleine->w;
    int fill_width = 0;
    if (max_value > 0 && value > 0) { fill_width = (int)((double)value / max_value) * full_width; }
    if (fill_width > full_width) fill_width = full_width;
    if (fill_width < 0) fill_width = 0;
    if (fill_width > 0) { blit(barre_pleine, dest, 0, 0, x, y, fill_width, barre_pleine->h); }
}

void dessiner_hud(GameState *gameState) {
    if (!gameState || !gameState->buffer) return;
    BITMAP *dest = gameState->buffer;
    int barre_x = 10;
    int barre_y = 10;
    dessiner_barre_progression(gameState, dest, barre_x, barre_y, gameState->hud_displayed_level_progress, LEVEL_DURATION_FRAMES);
}















