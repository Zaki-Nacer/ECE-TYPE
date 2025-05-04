#include <allegro.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

// Déclaration des images utilisées
BITMAP *buffer;
BITMAP *btn_idle;
BITMAP *btn_selected;
BITMAP *bg_image;
BITMAP *your_score_box;
BITMAP *sprites_niveaux[5]; // Sprites pour 0, 1, 2, 3 étoiles et niveau verrouillé
BITMAP *fond_map;

// Structure pour stocker la position des niveaux
typedef struct {
    int x, y;
} Position;

// Coordonnées des 3 niveaux
Position positions_niveaux[3] = {
    {100, 400},
    {700, 600},
    {1200, 500}
};

int niveau_selectionne = 0; // Niveau actuellement sélectionné
int bg_scroll_x = 0;        // Position du fond qui défile

// Éléments du menu principal
char *menu_labels[] = {"Nouvelle Partie", "Reprendre Partie", "Options", "Quitter"};
int menu_index = 0;
int menu_size = 4;
int key_pressed = 0;

// Déclarations des fonctions
void init();
void deinit();
void afficher_menu();
void demander_pseudo_nouvelle();
void demander_pseudo_reprise();
void afficher_options();
void sauvegarder_niveau(const char *pseudo, int niveau);
void afficher_fond_defilant();
void afficher_map(const char *pseudo);
void afficher_message_erreur(const char* ligne1, const char* ligne2);
void jouer_niveau(int index, const char *pseudo);

// Fonction principale
int main() {
    init();

    while (!key[KEY_ESC]) {
        clear(buffer);
        afficher_fond_defilant();
        afficher_menu();

        // Navigation avec les flèches
        if (!key_pressed) {
            if (key[KEY_DOWN]) {
                menu_index = (menu_index + 1) % menu_size;
                key_pressed = 1;
            }
            if (key[KEY_UP]) {
                menu_index = (menu_index - 1 + menu_size) % menu_size;
                key_pressed = 1;
            }
        }

        if (!key[KEY_DOWN] && !key[KEY_UP]) {
            key_pressed = 0;
        }

        // Choix avec la touche Entrée
        if (key[KEY_ENTER]) {
            switch(menu_index) {
                case 0: demander_pseudo_nouvelle(); break;
                case 1: demander_pseudo_reprise(); break;
                case 2: afficher_options(); break;
                case 3: deinit(); exit(EXIT_SUCCESS); break;
            }
        }

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        rest(10);
    }

    deinit();
    return 0;
}
END_OF_MAIN()

// Initialisation des ressources
void init() {
    allegro_init();
    install_keyboard();
    set_color_depth(32);
    set_color_conversion(COLORCONV_TOTAL);
    set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 1920, 1080, 0, 0);
    clear_to_color(screen, makecol(0, 0, 0));

    buffer = create_bitmap(SCREEN_W, SCREEN_H);

    // Chargement des images
    bg_image = load_bitmap("background.bmp", NULL);
    your_score_box = load_bitmap("YourScoreBox.bmp", NULL);
    fond_map = load_bitmap("background_map.bmp", NULL);
    sprites_niveaux[0] = load_bitmap("MapLevel_0star.bmp", NULL);
    sprites_niveaux[1] = load_bitmap("MapLevel_1star.bmp", NULL);
    sprites_niveaux[2] = load_bitmap("MapLevel_2star.bmp", NULL);
    sprites_niveaux[3] = load_bitmap("MapLevel_3star.bmp", NULL);
    sprites_niveaux[4] = load_bitmap("MapLevel_Lock.bmp", NULL);
    btn_idle = load_bitmap("OrangeBtn1.bmp", NULL);
    btn_selected = load_bitmap("RedBtn2.bmp", NULL);

    for (int i = 0; i < 5; i++) {
        if (!sprites_niveaux[i]) {
            allegro_message("Erreur de chargement d’un sprite de niveau !");
            exit(EXIT_FAILURE);
        }
    }

    if (!bg_image || !your_score_box || !fond_map || !btn_idle || !btn_selected) {
        allegro_message("Erreur de chargement d’une image !");
        exit(EXIT_FAILURE);
    }

    set_trans_blender(0, 0, 0, 255);
}

// Libération des ressources
void deinit() {
    destroy_bitmap(buffer);
    destroy_bitmap(btn_idle);
    destroy_bitmap(btn_selected);
    destroy_bitmap(bg_image);
    destroy_bitmap(your_score_box);
    destroy_bitmap(fond_map);
    for (int i = 0; i < 5; i++) {
        destroy_bitmap(sprites_niveaux[i]);
    }
}

// Affiche le menu principal avec les boutons
void afficher_menu() {
    int btn_largeur = 300;
    int btn_hauteur = 100;
    int espace = 40;
    int hauteur_totale = menu_size * btn_hauteur + (menu_size - 1) * espace;
    int start_y = (SCREEN_H - hauteur_totale) / 2;
    int start_x = (SCREEN_W - btn_largeur) / 2;

    for (int i = 0; i < menu_size; i++) {
        int y = start_y + i * (btn_hauteur + espace);
        BITMAP *btn = (i == menu_index) ? btn_selected : btn_idle;
        stretch_sprite(buffer, btn, start_x, y, btn_largeur, btn_hauteur);
        textprintf_centre_ex(buffer, font, start_x + btn_largeur / 2, y + btn_hauteur / 3,
                             (i == menu_index) ? makecol(255, 255, 255) : makecol(0, 0, 0),
                             -1, "%s", menu_labels[i]);
    }
}
// Saisie du pseudo pour créer une nouvelle partie
void demander_pseudo_nouvelle() {
    char pseudo[21] = "";
    int index = 0;
    int done = 0;

    while (!done) {
        clear(buffer);
        afficher_fond_defilant();

        // Titre
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, 150, makecol(255, 255, 255), -1, "Votre pseudo");

        // Boîte pour entrer le pseudo
        int box_x = (SCREEN_W - your_score_box->w) / 2;
        int box_y = 250;
        draw_sprite(buffer, your_score_box, box_x, box_y);

        // Affiche le pseudo en cours de saisie
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, box_y + your_score_box->h / 2 - 8, makecol(255, 255, 255), -1, "%s_", pseudo);

        // Instructions
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, box_y + your_score_box->h + 40, makecol(180, 180, 180), -1, "(Validez avec ENTREE)");

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        // Lecture des touches
        if (keypressed()) {
            int ch = readkey() & 0xff;

            if (ch == 8 && index > 0) { // Retour arrière
                index--;
                pseudo[index] = '\0';
            } else if (ch >= 32 && ch <= 126 && index < 20) {
                pseudo[index++] = ch;
                pseudo[index] = '\0';
            } else if (ch == 13 && index > 0) {
                // Vérifie si le pseudo existe déjà
                FILE *f = fopen("saves.txt", "r");
                int existe = 0;
                if (f) {
                    char ligne[100], p[30];
                    while (fgets(ligne, sizeof(ligne), f)) {
                        if (sscanf(ligne, "%s", p) == 1 && strcmp(p, pseudo) == 0) {
                            existe = 1;
                            break;
                        }
                    }
                    fclose(f);
                }

                if (existe) {
                    afficher_message_erreur("Ce pseudo existe déjà !", "Allez dans \"Reprendre une partie\"");
                    return;
                } else {
                    // Nouveau joueur → sauvegarde initiale
                    f = fopen("saves.txt", "a");
                    if (f) {
                        fprintf(f, "%s 0 -1 -1\n", pseudo); // Débloque seulement le niveau 1
                        fclose(f);
                    }
                    done = 1;
                }
            }
        }

        rest(10);
    }

    while (key[KEY_ENTER]) rest(10); // Attendre relâchement
    afficher_map(pseudo); // Afficher la carte
}

// Saisie du pseudo pour charger une sauvegarde existante
void demander_pseudo_reprise() {
    char pseudo[21] = "";
    int index = 0;
    int done = 0;

    while (!done) {
        clear(buffer);
        afficher_fond_defilant();

        // Titre
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, 150, makecol(255, 255, 255), -1, "Pseudo sauvegardé");

        int box_x = (SCREEN_W - your_score_box->w) / 2;
        int box_y = 250;
        draw_sprite(buffer, your_score_box, box_x, box_y);

        textprintf_centre_ex(buffer, font, SCREEN_W / 2, box_y + your_score_box->h / 2 - 8, makecol(255, 255, 255), -1, "%s_", pseudo);
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, box_y + your_score_box->h + 40, makecol(180, 180, 180), -1, "(Validez avec ENTREE)");

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

        if (keypressed()) {
            int ch = readkey() & 0xff;

            if (ch == 8 && index > 0) {
                index--;
                pseudo[index] = '\0';
            } else if (ch >= 32 && ch <= 126 && index < 20) {
                pseudo[index++] = ch;
                pseudo[index] = '\0';
            } else if (ch == 13 && index > 0) {
                FILE *f = fopen("saves.txt", "r");
                int existe = 0;
                char ligne[100], p[30];
                while (f && fgets(ligne, sizeof(ligne), f)) {
                    if (sscanf(ligne, "%s", p) == 1 && strcmp(p, pseudo) == 0) {
                        existe = 1;
                        break;
                    }
                }
                if (f) fclose(f);

                if (existe) {
                    while (key[KEY_ENTER]) rest(10);
                    afficher_map(pseudo);
                    done = 1;
                } else {
                    afficher_message_erreur("Aucune sauvegarde trouvée !", "Vérifiez le pseudo ou lancez une nouvelle partie");
                    return;
                }
            }
        }

        rest(10);
    }
}

// Options à venir
void afficher_options() {
    clear(buffer);
    textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), -1, "Options à venir...");
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    readkey(); // Attente d'une touche
}

// Écriture d'une nouvelle sauvegarde
void sauvegarder_niveau(const char *pseudo, int niveau) {
    FILE *f = fopen("saves.txt", "a");
    if (f != NULL) {
        fprintf(f, "%s %d %d %d\n", pseudo, 0, -1, -1);
        fclose(f);
    }
}

// Affichage du fond qui défile horizontalement
void afficher_fond_defilant() {
    bg_scroll_x += 1;
    if (bg_scroll_x >= bg_image->w)
        bg_scroll_x = 0;

    int x1 = -bg_scroll_x;
    int x2 = x1 + bg_image->w;

    draw_sprite(buffer, bg_image, x1, 0);
    draw_sprite(buffer, bg_image, x2, 0);
}

int charger_sauvegarde(const char *pseudo, int progression[3]) {
    FILE *f = fopen("saves.txt", "r");
    if (!f) return 0;

    char ligne[100];
    char p[30];
    int n1, n2, n3;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (sscanf(ligne, "%s %d %d %d", p, &n1, &n2, &n3) == 4) {
            if (strcmp(p, pseudo) == 0) {
                progression[0] = n1;
                progression[1] = n2;
                progression[2] = n3;
                fclose(f);
                return 1;
            }
        }
    }

    fclose(f);
    return 0; // Pseudo non trouvé
}

void afficher_map(const char *pseudo) {
    int progression[3] = {0, -1, -1}; // Niveau 1 débloqué, les autres bloqués
    int niveau_selectionne = 0;
    int key_cooldown = 0;

    // Si aucune sauvegarde, en créer une par défaut
    if (!charger_sauvegarde(pseudo, progression)) {
        FILE *f = fopen("saves.txt", "a");
        if (f) {
            fprintf(f, "%s 0 -1 -1\n", pseudo);
            fclose(f);
        }
    }

    while (!key[KEY_ESC]) {
        clear(buffer);
        draw_sprite(buffer, fond_map, 0, 0);

        // Affiche le nom du joueur en haut
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, 40, makecol(255, 255, 255), -1, "Profil : %s", pseudo);

        // Navigation clavier gauche/droite
        if (!key_cooldown) {
            if (key[KEY_LEFT] && niveau_selectionne > 0) {
                niveau_selectionne--;
                key_cooldown = 1;
            }
            if (key[KEY_RIGHT] && niveau_selectionne < 2) {
                niveau_selectionne++;
                key_cooldown = 1;
            }

            // Lancer un niveau si débloqué
            if (key[KEY_ENTER]) {
                if (progression[niveau_selectionne] >= 0) {
                    jouer_niveau(niveau_selectionne, pseudo);
                } else {
                    afficher_message_erreur("Niveau verrouillé !", "Terminez le précédent pour le débloquer");
                }
            }
        }

        // Remet le cooldown à 0 si aucune flèche n'est pressée
        if (!key[KEY_LEFT] && !key[KEY_RIGHT]) {
            key_cooldown = 0;
        }

        // Affichage des niveaux
        for (int i = 0; i < 3; i++) {
            int etat = progression[i];
            if (etat < 0 || etat > 3) etat = 4; // verrouillé

            int x = positions_niveaux[i].x;
            int y = positions_niveaux[i].y;

            if (i == niveau_selectionne) {
                // Niveau sélectionné : affiché en plus grand
                int new_w = sprites_niveaux[etat]->w * 1.3;
                int new_h = sprites_niveaux[etat]->h * 1.3;
                stretch_sprite(buffer, sprites_niveaux[etat],
                               x - (new_w - sprites_niveaux[etat]->w) / 2,
                               y - (new_h - sprites_niveaux[etat]->h) / 2,
                               new_w, new_h);
            } else {
                draw_sprite(buffer, sprites_niveaux[etat], x, y);
            }
        }

        // Instructions à l'écran
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H - 80, makecol(180, 180, 180), -1,
                             "Utilisez GAUCHE/DROITE pour choisir un niveau");

        textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H - 50, makecol(180, 180, 180), -1,
                             "Appuyez sur ECHAP pour revenir au menu");

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        rest(15);
    }
}

void afficher_message_erreur(const char* ligne1, const char* ligne2) {
    while (!keypressed()) {
        clear(buffer);
        afficher_fond_defilant();

        textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2 - 30, makecol(255, 100, 100), -1, ligne1);
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), -1, ligne2);
        textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2 + 40, makecol(180, 180, 180), -1,
                             "(Appuyez sur une touche pour revenir)");

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        rest(10);
    }

    readkey(); // vide la touche pressée
}

void jouer_niveau(int index, const char *pseudo) {
    clear(buffer);
    afficher_fond_defilant();

    // Message de chargement
    char msg[100];
    sprintf(msg, "Chargement du niveau %d pour %s...", index + 1, pseudo);

    textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), -1, msg);

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    rest(1000);

}
