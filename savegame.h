#ifndef SAVEGAME_H
#define SAVEGAME_H

// Sauvegarde le niveau atteint pour le joueur donné.
// Retourne 0 en cas de succès, -1 en cas d'erreur.
int save_progress(const char *playerName, int level);

// Charge le niveau atteint pour le joueur donné.
// Retourne le niveau lu (>= 1), ou 1 si aucune sauvegarde n'est trouvée ou en cas d'erreur.
int load_progress(const char *playerName);

#endif // SAVEGAME_H
