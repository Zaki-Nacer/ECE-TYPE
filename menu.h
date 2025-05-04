
#ifndef MENU_H
#define MENU_H

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

#endif //MENU_H
