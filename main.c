#include <openssl/sha.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include "medecin_page.h"
#include "patient_page.h"
#include "bdd.h"

// Déclaration des widgets globaux
GtkWidget *menu_window;
GtkWidget *stack;
GtkWidget *box;
GtkWidget *login_box;
GtkWidget *register_box;
GtkWidget *login_button_menu;
GtkWidget *register_button_menu;
GtkWidget *email_login;
GtkWidget *password_login;
GtkWidget *login_button;
GtkWidget *nom_register;
GtkWidget *prenom_register;
GtkWidget *email_register;
GtkWidget *numero_register;
GtkWidget *password_register;
GtkWidget *register_button;
GtkWidget *type_user;
GtkWidget *birth_date;
GtkWidget *type_combobox;

void insert_user(PGconn *db, const char *nom, const char *prenom, const char *password, const char *email, const char *type, const char *date_de_naissance, const char *numero_telephone) {
    const char *query = "INSERT INTO utilisateur (nom, prenom, mot_de_passe, email, type, date_de_naissance, telephone) VALUES ($1, $2, $3, $4, $5, $6, $7)";
    const char *Values[7] = {nom, prenom, password, email, type, date_de_naissance, numero_telephone};
    PGresult *result = PQexecParams(db, query, 7, NULL, Values, NULL, NULL, 0);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erreur d'inscription : %s\n", PQresultErrorMessage(result));
    } else {
        printf("Inscription réalisée avec succès\n");
    }
    PQclear(result);
}
void success(PGconn *db, const char *email, const char *type) {
    if (strcmp(type, "Medecin") == 0) {
        char *nom, *prenom;
        int id_medecin;
        get_medecin(db, &nom, &prenom, &id_medecin, email);
        medecin_page(db, email);
    }
    if (strcmp(type, "Patient") == 0) {
        display_patient_page(db, email);
    } else {
        fprintf(stderr, "Type d'utilisateur non reconnu : %s\n", type);
    }
}

void verify_user(PGconn *db, const char *password, const char *email) {
    const char *query = "SELECT mot_de_passe, type FROM utilisateur WHERE email = $1";
    const char *values[1] = {email};
    PGresult *result = PQexecParams(db, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
        const char *password_stocked = PQgetvalue(result, 0, 0);
        const char *type = PQgetvalue(result, 0, 1);

        if (strcmp(password, password_stocked) == 0) {
            printf("\nConnexion réussie\n");
            success(db,email,type);
        } else {
            printf("\nMot de passe incorrect\n");
        }
    } else {
        printf("\nUtilisateur introuvable\n");
    }
    PQclear(result);
}

void login_click(GtkButton *button, gpointer user_data) {
    const char *email = gtk_entry_get_text(GTK_ENTRY(email_login));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_login));
    PGconn *db = connexion_db();
    if (db != NULL) {
        verify_user(db, password, email);
        PQfinish(db);
    } else {
        printf("erreur\n");
    }
}

void register_click(GtkButton *button, gpointer user_data) {
    const char *nom = gtk_entry_get_text(GTK_ENTRY(nom_register));
    const char *prenom = gtk_entry_get_text(GTK_ENTRY(prenom_register));
    const char *email = gtk_entry_get_text(GTK_ENTRY(email_register));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_register));
    const char *numero_telephone = gtk_entry_get_text(GTK_ENTRY(numero_register));
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combobox));
    char date_de_naissance[11];
    const char *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(type_combobox));

    if (index == 2) { //si patient est sélectionné
        guint year, month, day;
        gtk_calendar_get_date(GTK_CALENDAR(birth_date), &year, &month, &day);
        snprintf(date_de_naissance, sizeof(date_de_naissance), "%04d-%02d-%02d", year, month + 1, day);
    } else {
        strcpy(date_de_naissance, "");
    }
    PGconn *db = connexion_db();

    if (type != NULL) {
        PGconn *db = connexion_db();
        if (db != NULL) {
            if (strcmp(type, "Medecin") == 0) {
                // Insérer un médecin avec date de naissance NULL
                insert_user(db, nom, prenom, password, email, type, NULL, numero_telephone);
                success(db,email,type);
            } else {
                if (strlen(date_de_naissance) == 0) {
                    printf("Veuillez saisir une date de naissance valide.");
                } else {
                    insert_user(db, nom, prenom, password, email, type, date_de_naissance, numero_telephone);
                }
            }
            PQfinish(db);
        } else {
            printf("erreur");
        }
    } else {
        printf("Veuillez sélectionner le type d'utilisateur");
    }
}

// fonction appelée lorsque le type est sélectionné
void type_combobox_change(GtkComboBox *combobox, gpointer user_data) {
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox)); // récupérer l'index de l'élément sélectionné
    if (index == 2) { //si patient est sélectionné
        gtk_widget_set_visible(birth_date, TRUE);
    } else {
        gtk_widget_set_visible(birth_date, FALSE);
    }
}

void login_user() {
    login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *space1 = gtk_label_new(NULL);
    gtk_widget_set_size_request(space1, -1, 180);
    gtk_box_pack_start(GTK_BOX(login_box), space1, FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(login_box), grid, TRUE, FALSE, 0);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);

    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), "<span font_desc='20' weight='bold'>Connexion</span>");
    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 1, 1);

    // Ajout d'un espace entre le label et les champs de saisie
    GtkWidget *spacing_label = gtk_label_new(NULL);
    gtk_widget_set_size_request(spacing_label, -1, 20);
    gtk_grid_attach(GTK_GRID(grid), spacing_label, 0, 1, 1, 1);

    email_login = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(email_login), "Email");
    gtk_widget_set_size_request(email_login, 250, 30);
    gtk_grid_attach(GTK_GRID(grid), email_login, 0, 2, 1, 1);

    password_login = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_login), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(password_login), FALSE);
    gtk_widget_set_size_request(password_login, 250, 30);
    gtk_grid_attach(GTK_GRID(grid), password_login, 0, 3, 1, 1);

    login_button = gtk_button_new_with_label("Se connecter");
    gtk_widget_set_size_request(login_button, 250, 30);
    gtk_grid_attach(GTK_GRID(grid), login_button, 0, 4, 1, 1);
    g_signal_connect(login_button, "clicked", G_CALLBACK(login_click), NULL);

    gtk_stack_add_named(GTK_STACK(stack), login_box, "login");
}

void register_user() {
    register_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(register_box), FALSE);  // Assure une taille non homogène des enfants

    GtkWidget *title_label = gtk_label_new("Inscription");
    gtk_label_set_markup(GTK_LABEL(title_label), "<span font_desc='20' weight='bold'>Inscription</span>");
    gtk_box_pack_start(GTK_BOX(register_box), title_label, FALSE, FALSE, 0);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(register_box), grid, TRUE, TRUE, 0);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);  // Alignement horizontal au centre
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);  // Alignement vertical au centre

    // Création des widgets pour le formulaire d'inscription
    type_combobox = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(type_combobox), NULL, "Type d'utilisateur");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(type_combobox), NULL, "Medecin");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(type_combobox), NULL, "Patient");
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combobox), 0);

    nom_register = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(nom_register), "Nom");

    prenom_register = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(prenom_register), "Prénom");

    email_register = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(email_register), "Email");

    numero_register = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(numero_register), "Numéro de téléphone");

    password_register = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_register), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(password_register), FALSE);

    birth_date = gtk_calendar_new();
    gtk_widget_set_visible(birth_date, FALSE);

    register_button = gtk_button_new_with_label("S'inscrire");

    // Ajout des widgets au grid
    gtk_grid_attach(GTK_GRID(grid), type_combobox, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), nom_register, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), prenom_register, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), email_register, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), numero_register, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_register, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), birth_date, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), register_button, 0, 7, 1, 1);

    g_signal_connect(register_button, "clicked", G_CALLBACK(register_click), NULL);
    gtk_stack_add_named(GTK_STACK(stack), register_box, "register");
    g_signal_connect(type_combobox, "changed", G_CALLBACK(type_combobox_change), "register");
}

// Fonction affichant le menu
void switch_to_login(GtkButton *button, gpointer stack) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "login");
}

void switch_to_register(GtkButton *button, gpointer stack) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "register");
}
// Fonction pour afficher le menu
void show_menu() {
    menu_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(menu_window), "SeniorCare");
    gtk_window_set_default_size(GTK_WINDOW(menu_window), 1000, 900);

    // Utilisation d'un GtkEventBox pour appliquer un style CSS
    GtkWidget *event_box = gtk_event_box_new();
    gtk_widget_set_name(event_box, "blue-background"); // Nom pour le style CSS
    gtk_container_add(GTK_CONTAINER(menu_window), event_box);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(event_box), box);  // Ajout à event_box au lieu de menu_window

    stack = gtk_stack_new();
    gtk_box_pack_start(GTK_BOX(box), stack, TRUE, TRUE, 0);

    // Utilisation d'un GtkFixed pour positionner les boutons
    GtkWidget *button_container = gtk_fixed_new();
    gtk_box_pack_start(GTK_BOX(box), button_container, TRUE, TRUE, 0);

    // Bouton "Connexion"
    login_button_menu = gtk_button_new_with_label("Connexion");
    g_signal_connect(login_button_menu, "clicked", G_CALLBACK(switch_to_login), stack);
    gtk_fixed_put(GTK_FIXED(button_container), login_button_menu, 430, 0);
    gtk_widget_set_size_request(login_button_menu, 150, 40);

    // Bouton "Inscription"
    register_button_menu = gtk_button_new_with_label("Inscription");
    g_signal_connect(register_button_menu, "clicked", G_CALLBACK(switch_to_register), stack);
    gtk_fixed_put(GTK_FIXED(button_container), register_button_menu, 430, 50);
    gtk_widget_set_size_request(register_button_menu, 150, 40);

    login_user();
    register_user();

    // Appliquer le style CSS pour le fond bleu
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, " #blue-background { background-color: #37C1BE; }\n", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(event_box);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_widget_show_all(menu_window);
}

void show_home_page() {
    GtkWidget *home_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(home_window), "SeniorCare");
    gtk_window_set_default_size(GTK_WINDOW(home_window), 1000, 900);
    g_signal_connect(home_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Conteneur principal avec un fond bleu
    GtkWidget *event_box = gtk_event_box_new();
    gtk_widget_set_name(event_box, "blue-background"); // Nom pour le style CSS
    gtk_container_add(GTK_CONTAINER(home_window), event_box);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(event_box), vbox);

    GtkWidget *space1 = gtk_label_new(NULL);
    gtk_widget_set_size_request(space1, -1, 180);
    gtk_box_pack_start(GTK_BOX(vbox), space1, FALSE, FALSE, 0);

    GtkWidget *title_label = gtk_label_new("Senior Care");
    gtk_label_set_markup(GTK_LABEL(title_label), "<span font_desc='20' weight='bold'>Senior Care</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);

    GtkWidget *legend_label = gtk_label_new("Bienvenue à SeniorCare, votre assistant de soins personnalisé.");
    gtk_label_set_markup(GTK_LABEL(legend_label), "<span font_desc='15'>Bienvenue à SeniorCare, votre assistant de soins personnalisé.</span>");
    gtk_box_pack_start(GTK_BOX(vbox), legend_label, FALSE, FALSE, 0);

    // un espaceentre le label et le bouton
    GtkWidget *space = gtk_label_new(NULL);
    gtk_widget_set_size_request(space, -1, 80);
    gtk_box_pack_start(GTK_BOX(vbox), space, FALSE, FALSE, 0);

    // Bouton "Démarrer"
    GtkWidget *start_button = gtk_button_new_with_label("Démarrer");
    g_signal_connect(start_button, "clicked", G_CALLBACK(show_menu), NULL);

    GtkWidget *button_container = gtk_fixed_new();  // Utilisation d'un conteneur fixe
    gtk_container_add(GTK_CONTAINER(vbox), button_container);

    // position et  taille du bouton dans le conteneur fixe
    gtk_fixed_put(GTK_FIXED(button_container), start_button, 420, 30);
    gtk_widget_set_size_request(start_button, 150, 40);  //la largeur et la hauteur

    // Appliquer le style CSS pour le fond bleu
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider," #blue-background { background-color: #37C1BE; }\n",-1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(event_box);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_widget_show_all(home_window);
}
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "fr_FR.UTF-8");
    gtk_init(&argc, &argv);
    show_home_page();
    gtk_main();
    return 0;
}
