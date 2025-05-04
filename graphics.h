#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "defs.h" // Inclure defs.h pour que GameState soit connu

// --- Prototypes des Fonctions Graphiques ---

// Initialise Allegro et certains champs de gameState (buffer, couleurs)
// Retourne 0 en cas de succès, -1 en cas d'erreur
int initialisation_allegro(GameState *gameState);

// Charge un bitmap en vérifiant les erreurs
BITMAP *charger_bitmap_safe(const char *nom_fichier);

// Remplace les pixels blancs par magenta dans un bitmap
void make_white_transparent(GameState *gameState, BITMAP *bmp);

// Met à jour la position du scrolling dans gameState
void mettre_a_jour_scrolling(GameState *gameState);

// Dessine le décor sur le buffer de destination (contenu dans gameState)
void dessiner_decor(GameState *gameState);

// Nettoie les ressources graphiques globales (buffer, decor) contenues dans gameState
void nettoyer_ressources_graphiques(GameState *gameState);

#endif // GRAPHICS_H
