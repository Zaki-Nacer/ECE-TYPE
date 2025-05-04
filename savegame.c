#include "savegame.h"
#include "defs.h" // Pour MAX_PLAYER_NAME_LEN (même si pas directemt utilisé ici)
#include <stdio.h>
#include <string.h> // Pour strlen, strcpy, strcat

// Fonction pour construire le nom du fichier de sauvegarde
// Attention: simple, ne gère pas les caractères spéciaux dans le nom.
void get_save_filename(const char *playerName, char *filename_buffer, size_t buffer_size) {
    // On limite la longueur du nom pour éviter les dépassements
    strncpy(filename_buffer, playerName, buffer_size - 5); // -5 pour ".sav" et le nul
    filename_buffer[buffer_size - 5] = '\0'; // Assurer la terminaison nulle
    strcat(filename_buffer, ".sav");
}

// Sauvegarde le niveau atteint
int save_progress(const char *playerName, int level) {
    if (!playerName || playerName[0] == '\0') {
        fprintf(stderr, "Erreur sauvegarde: Nom de joueur invalide.\n");
        return -1;
    }
    if (level < 1) {
        fprintf(stderr, "Erreur sauvegarde: Niveau invalide (%d).\n", level);
        return -1; // Ne pas sauvegarder un niveau invalide
    }

    char filename[MAX_PLAYER_NAME_LEN + 5]; // Assez pour nom + ".sav" + '\0'
    get_save_filename(playerName, filename, sizeof(filename));

    printf("Sauvegarde du niveau %d pour le joueur '%s' dans %s...\n", level, playerName, filename);
    fflush(stdout);

    FILE *f = fopen(filename, "w"); // Ouvre en mode écriture (écrase l'ancien)
    if (f == NULL) {
        perror("Erreur ouverture fichier sauvegarde en écriture");
        fprintf(stderr, "Impossible de sauvegarder la progression pour %s.\n", playerName);
        return -1;
    }

    // Écrit simplement le numéro du niveau dans le fichier
    fprintf(f, "%d", level);

    fclose(f);
    printf("Sauvegarde réussie.\n"); fflush(stdout);
    return 0;
}

// Charge le niveau atteint
int load_progress(const char *playerName) {
     if (!playerName || playerName[0] == '\0') {
        fprintf(stderr, "Erreur chargement: Nom de joueur invalide.\n");
        return 1; // Retourne niveau 1 par défaut
    }

    char filename[MAX_PLAYER_NAME_LEN + 5];
    get_save_filename(playerName, filename, sizeof(filename));

    printf("Tentative de chargement pour le joueur '%s' depuis %s...\n", playerName, filename);
    fflush(stdout);

    FILE *f = fopen(filename, "r"); // Ouvre en mode lecture
    if (f == NULL) {
        // perror("Info: Fichier sauvegarde non trouvé"); // Pas une erreur fatale
        printf("Aucune sauvegarde trouvée pour '%s'. Début au niveau 1.\n", playerName);
        fflush(stdout);
        return 1; // Pas de sauvegarde, commencer au niveau 1
    }

    int loaded_level = 1; // Défaut si lecture échoue
    if (fscanf(f, "%d", &loaded_level) == 1) {
        // Lecture réussie
        if (loaded_level < 1 || loaded_level > MAX_LEVEL) { // Vérifier validité
             printf("Niveau sauvegardé (%d) invalide. Début au niveau 1.\n", loaded_level);
             fflush(stdout);
             loaded_level = 1;
        } else {
            printf("Sauvegarde trouvée. Niveau chargé: %d\n", loaded_level);
            fflush(stdout);
        }
    } else {
        // Erreur de lecture ou fichier vide
        printf("Erreur lecture fichier sauvegarde ou fichier vide. Début au niveau 1.\n");
        fflush(stdout);
        loaded_level = 1;
    }

    fclose(f);
    return loaded_level;
}
