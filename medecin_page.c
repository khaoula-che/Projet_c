#include <gtk/gtk.h>
#include <stdio.h>
#include "bdd.h"
#define NOM_COLUMN 0
#define PRENOM_COLUMN 1

PGconn *connexion_db() {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);
    if (PQstatus(db) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s\n", PQerrorMessage(db));
        PQfinish(db);
        exit(EXIT_FAILURE);
    }
    PQexec(db, "SET client_encoding TO 'UTF8'");
    return db;
}
typedef struct {
    PGconn *db;
    const char *email;
    const char *nom;
    const char *prenom;
    const char *telephone;
    int id_utilisateur;
    int id_medecin;

    GtkWidget *entry_nom;
    GtkWidget *entry_prenom;
    GtkWidget *entry_email;
    GtkWidget *entry_description;
    GtkWidget *entry_adresse;
    GtkWidget *entry_telephone;
    GtkWidget *entry_password;

    GtkWidget *patient_treeview;
    GtkWidget *calendar;
    GtkSpinButton *spin_button_heure;
    GtkSpinButton *spin_button_minute;
    GtkWindow *form_window;
} user;

typedef struct{
    int id_utilisateur;
    char *nom;
    char *prenom;
    char *date_naissance;
    char *telephone;
    char *email;
} patient;

void update_profile(user *userData);
void modify_profil(user *user_data);
void medecin_page(PGconn *db, const char *email);

void get_medecin(PGconn *db, char **nom, char **prenom, int *id_medecin, const char *email_input);
void get_patient(PGconn *db, int id_medecin, GtkListStore *list_store);
void rendez_vous(PGconn *db, int id_medecin, GtkListStore *rv_store);
int get_idPatient(const char *nom, const char *prenom);
void ajouter_rendez_vous(PGconn *conn, int id_patient, int id_medecin, const char *date, int heure, int minute, const char *description, const char *lieu);
void ajouter_patient(PGconn *conn, int id_patient, int id_medecin, const char *nom, const char *prenom);

static void window_close(GtkWidget *widget, gpointer data);
static void save_button_clicked(GtkButton *button, gpointer user_data);
static void modify_button_clicked(GtkButton *button, gpointer user_data);
static void on_profile_button(GtkButton *button, gpointer user_data);
static void on_submit_rv_button_clicked(GtkButton *button, gpointer user_data);
static void on_ajouter_rv_button_clicked(GtkButton *button,gpointer user_data);
static void submit_button_clicked(GtkButton *button, gpointer user_data);
static void on_ajouter_button_clicked(GtkButton *button, gpointer user_data);

void add_column(GtkWidget *view, const char *title, int column_id);
GtkWidget *tree_view(GtkListStore *list_store, const char *column_titles[], int num_columns);
GtkWidget *patient_list(PGconn *db, int id_utilisateur);
GtkWidget *rv_list(PGconn *db, int id_utilisateur);

// Lorsque la fenêtre sera fermée
static void window_close(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}
void get_medecin(PGconn *db, char **nom, char **prenom, int *id_medecin, const char *email_input) {
    const char *query = "SELECT nom, prenom, id_utilisateur, email, telephone FROM utilisateur WHERE email = $1 AND type = 'Medecin'";
    const char *valeur[1] = {email_input};
    PGresult *result = PQexecParams(db, query, 1, NULL, valeur, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
        if (PQntuples(result) > 0) {
            *nom = strdup(PQgetvalue(result, 0, 0));
            *prenom = strdup(PQgetvalue(result, 0, 1));
            *id_medecin = atoi(PQgetvalue(result, 0, 2));
            const char *email = PQgetvalue(result, 0, 3);
            const char *telephone = PQgetvalue(result, 0, 4);

            printf("Nom: %s, Prenom: %s, ID: %d, Email: %s, Telephone: %s\n", *nom, *prenom, *id_medecin, email, telephone);
        } else {
            fprintf(stderr, "Aucun médecin trouvé avec l'email spécifié\n");
        }
    } else {
        fprintf(stderr, "Erreur lors de la récupération des données du médecin : %s\n", PQerrorMessage(db));
    }
    PQclear(result);
}
void get_patient(PGconn *db, int id_medecin, GtkListStore *list_store) {
    const char *query = "SELECT nom, prenom FROM utilisateur WHERE type='Patient' AND id_medecin = $1";
    char id_str[12];
    snprintf(id_str, sizeof(id_str), "%d", id_medecin);
    const char *values[1] = {id_str};
    PGresult *result = PQexecParams(db, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
        for (int i = 0; i < PQntuples(result); i++) {
            const char *nom = PQgetvalue(result, i, 0);
            const char *prenom = PQgetvalue(result, i, 1);
            GtkTreeIter list;
            gtk_list_store_append(list_store, &list);
            gtk_list_store_set(list_store, &list, 0, nom, 1, prenom, -1);
        }
    }
    PQclear(result);
}
void update_profile(user *userData) {
    g_print("In the saving function...\n");

    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);

    // Check the status of the database connection
    if (PQstatus(db) != CONNECTION_OK) {
        g_print("Error: Database connection is not OK: %s\n", PQerrorMessage(db));
        PQfinish(db);  // Close the invalid connection
        return;
    }

    g_print("Email: %s\n", userData->email);
    g_print("New Nom: %s\n", userData->nom);
    g_print("New Prenom: %s\n", userData->prenom);
    g_print("New Numero de Telephone: %s\n", userData->telephone);

    const char *email = userData->email;
    const char *new_nom = userData->nom;
    const char *new_prenom = userData->prenom;
    const char *new_numero_telephone = userData->telephone;

    g_print("Line 197...\n");
    g_print("Start sql...\n");

    const char *update_query = "UPDATE utilisateur SET nom = $1, prenom = $2,  numero_telephone = $3 WHERE email = $4";
    const char *update_values[4] = {new_nom, new_prenom, new_numero_telephone, email};

    g_print("Line 202...\n");

    PGresult *update_result = PQexecParams(db, update_query, 5, NULL, update_values, NULL, NULL, 0);

    // Check the status of the query result
    if (PQresultStatus(update_result) != PGRES_COMMAND_OK) {
        g_print("Error updating profile: %s\n", PQerrorMessage(db));
    } else {
        g_print("Profile updated successfully\n");
    }

    g_print("Line 206 after the sql...\n");

    PQclear(update_result);
    PQfinish(db);  // Close the connection
}
void modify_profil(user *user_data) {
    user *modifyData = (user *)user_data;
    if (modifyData == NULL) {
        g_print("Error: User data is NULL\n");
        return;
    }
    GtkWidget *modify_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(modify_window), "Modifier le profil");
    gtk_window_set_default_size(GTK_WINDOW(modify_window), 400, 400);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(modify_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    GtkWidget *entry_nom = gtk_entry_new();
    GtkWidget *entry_prenom = gtk_entry_new();
    GtkWidget *entry_telephone = gtk_entry_new();
    GtkWidget *entry_password = gtk_entry_new();

    GtkWidget *label_nom = gtk_label_new("Nouveau nom:");
    GtkWidget *label_prenom = gtk_label_new("Nouveau prénom:");
    GtkWidget *label_telephone = gtk_label_new("Nouveau numéro de téléphone:");
    GtkWidget *label_password = gtk_label_new("Nouveau mot de passe:");

    GtkWidget *save_button = gtk_button_new_with_label("Enregistrer");
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_button_clicked), modifyData);

    gtk_grid_attach(GTK_GRID(grid), label_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_nom, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_prenom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_prenom, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_telephone, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_telephone, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), save_button, 0, 4, 2, 1);

    gtk_entry_set_text(GTK_ENTRY(entry_nom), user_data->nom);
    gtk_entry_set_text(GTK_ENTRY(entry_prenom), user_data->prenom);
    gtk_entry_set_text(GTK_ENTRY(entry_telephone), user_data->telephone);
    gtk_entry_set_text(GTK_ENTRY(entry_password), "");

    user_data->entry_nom = entry_nom;
    user_data->entry_prenom = entry_prenom;
    user_data->entry_telephone = entry_telephone;
    user_data->entry_password = entry_password;

    gtk_widget_show_all(modify_window);
}
static void on_profile_button(GtkButton *button, gpointer user_data) {
    user *userData = (user *)user_data;
    if (userData == NULL || userData->db == NULL) {
        g_print("Error: Connexion à la base de données nulle ou invalide\n");
        return;
    }
    const char *nom = userData->nom;
    const char *prenom = userData->prenom;
    const char *email = userData->email;
    const char *telephone = userData->telephone;

    GtkWidget *profile_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(profile_window), "Profile");
    gtk_window_set_default_size(GTK_WINDOW(profile_window), 1000, 900);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(profile_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    GtkWidget *label_nom = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Nom:</span> %s", nom));
    GtkWidget *label_prenom = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Prénom:</span> %s", prenom));
    GtkWidget *label_email = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Email:</span> %s", email));
    GtkWidget *label_telephone = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Téléphone:</span> %s", telephone));
    gtk_label_set_use_markup(GTK_LABEL(label_nom), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_prenom), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_email), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_telephone), TRUE);

    gtk_grid_attach(GTK_GRID(grid), label_email, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_nom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_prenom, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_telephone, 0, 3, 1, 1);

    GtkWidget *modify_button = gtk_button_new_with_label("Modifier");
    g_signal_connect(modify_button, "clicked", G_CALLBACK(modify_button_clicked), user_data);
    gtk_grid_attach(GTK_GRID(grid), modify_button, 0, 5, 1, 1);

    gtk_widget_show_all(profile_window);
}
static void modify_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Redirecting to the modification page...\n");
    modify_profil((user *)user_data);
}
static void save_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Saving modified profile...\n");
    user *modifyData = (user *)user_data;

    GtkWidget *entry_nom = GTK_WIDGET(modifyData->entry_nom);
    GtkWidget *entry_prenom = GTK_WIDGET(modifyData->entry_prenom);
    GtkWidget *entry_telephone = GTK_WIDGET(modifyData->entry_telephone);
    GtkWidget *entry_password = GTK_WIDGET(modifyData->entry_password);

    if (entry_nom == NULL || entry_prenom == NULL || entry_telephone == NULL || entry_password == NULL) {
        g_print("Error: One or more entry fields are NULL\n");
        return;
    }
    const char *new_nom = gtk_entry_get_text(GTK_ENTRY(modifyData->entry_nom));
    const char *new_prenom = gtk_entry_get_text(GTK_ENTRY(modifyData->entry_prenom));
    const char *new_telephone = gtk_entry_get_text(GTK_ENTRY(modifyData->entry_telephone));
    const char *new_password = gtk_entry_get_text(GTK_ENTRY(modifyData->entry_password));

    g_free(modifyData->nom);
    g_free(modifyData->prenom);
    g_free(modifyData->telephone);

    modifyData->nom = g_strdup(new_nom);
    modifyData->prenom = g_strdup(new_prenom);
    modifyData->telephone = g_strdup(new_telephone);

    g_print("Going straight to the saving function...\n");
    update_profile(modifyData);
    gtk_widget_destroy(GTK_WIDGET(button));
}
void add_column(GtkWidget *view, const char *title, int column_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", column_id, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);  // Fix: Pass 'view' instead of 'treeview'
}
// Cette fonction est appelée lorsque le bouton est cliqué.
void button_envoyer_clicked(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    gchar *path_string = gtk_tree_path_to_string(path);
    g_print("Ligne activée : %s\n", path_string);

    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        g_free(path_string);
        return; // Ligne non trouvée
    }

    // Récupération du nom et prénom
    gchar *nom, *prenom;
    gtk_tree_model_get(model, &iter, NOM_COLUMN, &nom, PRENOM_COLUMN, &prenom, -1);
    g_print("Nom : %s, Prénom : %s\n", nom, prenom);

    // Récupération de l'id_utilisateur en utilisant nom et prénom
    int id_utilisateur = get_idPatient(nom, prenom);
    g_print("id_utilisateur: %d\n", id_utilisateur);

    int id_rendezvous;
    gtk_tree_model_get(model, &iter, 6, &id_rendezvous, -1);
    g_print("id_rendezvous : %d\n", id_rendezvous);

    char id_utilisateur_str[12];
    char id_rendezvous_str[12];
    snprintf(id_utilisateur_str, sizeof(id_utilisateur_str), "%d", id_utilisateur);
    snprintf(id_rendezvous_str, sizeof(id_rendezvous_str), "%d", id_rendezvous);

    // Insertion d'une nouvelle entrée dans la table rappel
    PGconn *db = connexion_db();
    if (db != NULL) {
        const char *insert_query = "INSERT INTO rappel (rendez_vous, conseil, meteo,message, id_patient) VALUES ($1, NULL, NULL, 'Rappel de votre rendez-vous', $2)";
        const char *values[2] = {id_rendezvous_str, id_utilisateur_str};
        PGresult *result = PQexecParams(db, insert_query, 2, NULL, values, NULL, NULL, 0);

        if (PQresultStatus(result) != PGRES_COMMAND_OK) {
            g_print("Échec de l'insertion : %s", PQresultErrorMessage(result));
        } else {
            g_print("Rappel envoyé !");
        }
    PQclear(result);
    PQfinish(db);
    // Libération de la mémoire
    g_free(nom);
    g_free(prenom);
    g_free(path_string);
    }
}
void cell_render_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data) {
    // Cette fonction est appelée pour chaque ligne de la vue arborescente pour configurer le rendu de la cellule
    g_object_set(renderer, "text", "Envoyer", NULL);
}
void add_button_column(GtkWidget *treeview) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Envoyer un rappel", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, (GtkTreeCellDataFunc)cell_render_func, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // Connecter le signal "row-activated" pour la vue arborescente
    g_signal_connect(treeview, "row-activated", G_CALLBACK(button_envoyer_clicked), NULL);
}
void cell_render_func_patient(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data) {
    // Cette fonction est appelée pour chaque ligne de la vue arborescente pour configurer le rendu de la cellule
    g_object_set(renderer, "text", "Voir plus", NULL);
}
patient *get_patientDetails(const char *nom, const char *prenom) {
    // Connexion à la base de données
    PGconn *db = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    const char *query = "SELECT id_utilisateur, nom, prenom, date_de_naissance, telephone, email FROM utilisateur WHERE nom = $1 AND prenom = $2";
    const char *param_values[2] = {nom, prenom};

    PGresult *result = PQexecParams(db, query, 2, NULL, param_values, NULL, NULL, 0);

    patient *patient = NULL;
    if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
        patient = malloc(sizeof(*patient));  // Correction ici
        if (patient) {
            patient->id_utilisateur = atoi(PQgetvalue(result, 0, 0));
            patient->nom = g_strdup(PQgetvalue(result, 0, 1));
            patient->prenom = g_strdup(PQgetvalue(result, 0, 2));
            patient->date_naissance = g_strdup(PQgetvalue(result, 0, 3));
            patient->telephone = g_strdup(PQgetvalue(result, 0, 4));
            patient->email = g_strdup(PQgetvalue(result, 0, 5));
        }
    }
    PQclear(result);
    PQfinish(db);
    return patient;
}

void free_patient(patient *patient) {
    if (patient) {
        g_free(patient->nom);
        g_free(patient->prenom);
        g_free(patient->date_naissance);
        g_free(patient->telephone);
        g_free(patient->email);
        free(patient);
    }
}

void button_details_clicked(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Détails du Patient");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 900);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gchar *path_string = gtk_tree_path_to_string(path);
    g_print("Ligne activée : %s\n", path_string);

    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        g_free(path_string);
        return;
    }

    gchar *nom, *prenom;
    gtk_tree_model_get(model, &iter, NOM_COLUMN, &nom, PRENOM_COLUMN, &prenom, -1);
    g_print("Nom : %s, Prénom : %s\n", nom, prenom);

    patient *details = get_patientDetails(nom, prenom);
    if (!details) {
        g_print("Erreur : Détails du patient non trouvés.\n");
        return;
    }

    GtkWidget *label_nom = gtk_label_new(g_strdup_printf("Nom: %s", nom));
    GtkWidget *label_prenom = gtk_label_new(g_strdup_printf("Prénom: %s", prenom));
    GtkWidget *label_email = gtk_label_new(g_strdup_printf("Email: %s", details->email));
    GtkWidget *label_telephone = gtk_label_new(g_strdup_printf("Téléphone: %s", details->telephone));

    gtk_box_pack_start(GTK_BOX(vbox), label_nom, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label_prenom, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label_email, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label_telephone, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    free_patient(details);
}
void add_button_column_details(GtkWidget *treeview) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Details", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, (GtkTreeCellDataFunc)cell_render_func_patient, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // Connecter le signal "row-activated" pour la vue arborescente
    g_signal_connect(treeview, "row-activated", G_CALLBACK(button_details_clicked), NULL);
}
GtkWidget *tree_view(GtkListStore *list_store, const char *column_titles[], int num_columns) {
    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    g_object_unref(list_store);
    for (int i = 0; i < num_columns; i++) {
        add_column(treeview, column_titles[i], i);
    }
    add_button_column_details(treeview);
    return treeview;
}
GtkWidget *tree_view_rv(GtkListStore *list_store, const char *column_titles[], int num_columns) {
    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    g_object_unref(list_store);
    for (int i = 0; i < num_columns; i++) {
        add_column(treeview, column_titles[i], i);
    }
    add_button_column(treeview); // Ajoute la colonne modifiée
    return treeview;
}
// Fonction pour créer la vue de la liste de patients
GtkWidget *patient_list(PGconn *db, int id_utilisateur) {
    GtkListStore *list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    get_patient(db, id_utilisateur, list_store);
    const char *column_titles[] = {"Nom", "Prénom"};
    return tree_view(list_store, column_titles, 2);
}
// Fonction pour créer la vue de la liste de rendez-vous
GtkWidget *rv_list(PGconn *db, int id_utilisateur) {
    GtkListStore *appointment_list_store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    rendez_vous(db, id_utilisateur, appointment_list_store);
    const char *column_titles[] = {"Nom Patient", "Prénom Patient", "Date", "Heure", "Lieu", "Description"};
    return tree_view_rv(appointment_list_store, column_titles, 6);
}

void rendez_vous(PGconn *db, int id_medecin, GtkListStore *rv_store) {
    printf("id_medecin: %d\n", id_medecin);
    const char *query = "SELECT u.nom, u.prenom, rv.date, rv.heure, rv.lieu, rv.description, rv.id_rendezvous FROM rendez_vous rv JOIN utilisateur u ON rv.id_patient = u.id_utilisateur WHERE rv.id_medecin = $1";
    char id_str[12];
    snprintf(id_str, sizeof(id_str), "%d", id_medecin);
    const char *values[1] = {id_str};
    PGresult *result = PQexecParams(db, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
        printf("Nombre de rendez-vous trouvés : %d\n", PQntuples(result));
        for (int i = 0; i < PQntuples(result); i++) {
            const char *nom = PQgetvalue(result, i, 0);
            const char *prenom = PQgetvalue(result, i, 1);
            const char *date = PQgetvalue(result, i, 2);
            const char *heure = PQgetvalue(result, i, 3);
            const char *lieu = PQgetvalue(result, i, 4);
            const char *description = PQgetvalue(result, i, 5);
            int id_rendezvous = atoi(PQgetvalue(result, i, 6));

            printf("Rendez-vous : %s, %s, %s, %s, %s, %s , id_rendez_vous: %d\n", date, heure, lieu, description, nom, prenom, id_rendezvous);

            GtkTreeIter list_rv;
            gtk_list_store_append(rv_store, &list_rv);
            gtk_list_store_set(rv_store, &list_rv, 0, nom, 1, prenom, 2, date, 3, heure, 4, lieu, 5, description, 6,id_rendezvous, -1);
        }
    } else {
        fprintf(stderr, "Erreur d'exécution de la requête : %s\n", PQerrorMessage(db));
    }

    PQclear(result);
}
int get_idPatient(const char *nom, const char *prenom) {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);

    if (nom == NULL || prenom == NULL) {
        fprintf(stderr, "Erreur : Les paramètres nom et prenom ne doivent pas être NULL.\n");
        return -1;
    }
    const char *query = "SELECT id_utilisateur, date_de_naissance, telephone, email FROM utilisateur WHERE nom = $1 AND prenom = $2";
    const char *param_values[2] = {nom, prenom};
    PGresult *result = PQexecParams(db, query, 2, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Erreur lors de la récupération de l'ID de l'utilisateur : %s\n", PQerrorMessage(db));
        PQclear(result);
        return -1;
    }
    int id_patient = -1;
    if (PQntuples(result) > 0) {
        id_patient = atoi(PQgetvalue(result, 0, 0));
    }
    PQclear(result);
    return id_patient;
}
// Fonction callback pour ouvrir le formulaire d'ajout de rendez-vous
static void on_ajouter_rv_button_clicked(GtkButton *button, gpointer user_data) {
    user *userData = (user*)user_data;
    userData->form_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(userData->form_window, "Ajouter Rendez-vous");
    gtk_window_set_default_size(userData->form_window, 1000, 900);

    GtkWidget *form_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(userData->form_window), form_grid);

    GtkWidget *label_nom = gtk_label_new("Nom:");
    userData->entry_nom = gtk_entry_new();
    GtkWidget *label_prenom = gtk_label_new("Prénom:");
    userData->entry_prenom = gtk_entry_new();
    GtkWidget *label_description = gtk_label_new("Description:");
    userData->entry_description = gtk_entry_new();
    GtkWidget *label_adresse = gtk_label_new("Adresse:");
    userData->entry_adresse = gtk_entry_new();
    GtkWidget *label_calendar = gtk_label_new("Date:");
    userData->calendar = gtk_calendar_new();
    GtkWidget *label_heure = gtk_label_new("Heure:");
    userData->spin_button_heure = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 23, 1));
    GtkWidget *label_minute = gtk_label_new("Minute:");
    userData->spin_button_minute = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 59, 1));
    GtkWidget *submit_button = gtk_button_new_with_label("Ajouter");
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_rv_button_clicked), userData);

    gtk_grid_attach(GTK_GRID(form_grid), label_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_nom, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_prenom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_prenom, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_description, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_description, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_adresse, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_adresse, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_calendar, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->calendar, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_heure, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), GTK_WIDGET(userData->spin_button_heure), 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_minute, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), GTK_WIDGET(userData->spin_button_minute), 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), submit_button, 0, 7, 2, 1);

    gtk_widget_show_all(GTK_WIDGET(userData->form_window));
}
static void on_submit_rv_button_clicked(GtkButton *button, gpointer user_data) {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);
    if (PQstatus(db) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s\n", PQerrorMessage(db));
        PQfinish(db);
        exit(EXIT_FAILURE);
    }
    user *userData = (user *)user_data;

    if (db == NULL) {
        fprintf(stderr, "Erreur : Connexion PostgreSQL non initialisée.\n");
        return;
    }
    const char *nom = gtk_entry_get_text(GTK_ENTRY(userData->entry_nom));
    const char *prenom = gtk_entry_get_text(GTK_ENTRY(userData->entry_prenom));
    const char *description = gtk_entry_get_text(GTK_ENTRY(userData->entry_description));
    const char *lieu = gtk_entry_get_text(GTK_ENTRY(userData->entry_adresse));

    // Ajoutez des vérifications pour les widgets GTK
    if (nom == NULL || prenom == NULL) {
        fprintf(stderr, "Erreur : Les widgets GTK ne sont pas correctement initialisés.\n");
        return;
    }
    // Ajoutez une vérification pour les chaînes vides (si nécessaire)
    if (nom[0] == '\0' || prenom[0] == '\0') {
        fprintf(stderr, "Erreur : Les champs Nom et Prénom ne doivent pas être vides.\n");
        return;
    }
    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(userData->calendar), &year, &month, &day);
    int heure = gtk_spin_button_get_value_as_int(userData->spin_button_heure);
    int minute = gtk_spin_button_get_value_as_int(userData->spin_button_minute);

    char dateStr[11];
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", year, month + 1, day);

    int id_patient = get_idPatient(nom, prenom);
    if (id_patient == -1) {
        fprintf(stderr, "Erreur lors de la récupération de l'ID de l'utilisateur.\n");
        return;
    }
    // Obtenir l'ID du médecin
    char *nom_med, *prenom_med, *telephone;
    int id_medecin;
    get_medecin(userData->db, &nom_med, &prenom_med, &id_medecin, userData->email);
    if (id_medecin == -1) {
        fprintf(stderr, "Aucun médecin trouvé avec l'email spécifié\n");
        free(nom_med);
        free(prenom_med);
        return;
    }

    if (id_medecin == -1) {
        fprintf(stderr, "Erreur lors de la récupération de l'ID du médecin.\n");
        return;
    }

    ajouter_rendez_vous(db, id_patient,id_medecin, dateStr, heure, minute, description, lieu);
    gtk_widget_destroy(GTK_WIDGET(userData->form_window));
}
void ajouter_rendez_vous(PGconn *conn, int id_patient, int id_medecin, const char *date, int heure, int minute, const char *description, const char *lieu) {
    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO rendez_vous (id_patient, id_medecin, date, heure, description, lieu) VALUES (%d, %d, '%s', '%02d:%02d', '%s', '%s')", id_patient, id_medecin, date, heure, minute, description, lieu);

    PGresult *result = PQexec(conn, query);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erreur lors de l'ajout du rendez-vous: %s\n", PQerrorMessage(conn));
    } else {
        printf("Rendez-vous ajouté avec succès!\n");
        printf("Détails du rendez-vous:\n");
        printf("ID Patient: %d\n", id_patient);
        printf("ID Médecin: %d\n", id_medecin);
        printf("Date: %s\n", date);
        printf("Heure: %02d:%02d\n", heure, minute);
        printf("Description: %s\n", description);
        printf("Lieu: %s\n", lieu);
    }

    PQclear(result);
}
// Fonction callback pour ouvrir le formulaire d'ajout de patient
static void ajouter_patient_button_clicked(GtkButton *button, gpointer user_data) {
    user *userData = (user *)user_data;
    userData->form_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(userData->form_window, "Ajouter Patient");
    gtk_window_set_default_size(userData->form_window, 1000, 900);

    GtkWidget *form_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(userData->form_window), form_grid);

    GtkWidget *label_nom = gtk_label_new("Nom:");
    userData->entry_nom = gtk_entry_new();
    GtkWidget *label_prenom = gtk_label_new("Prénom:");
    userData->entry_prenom = gtk_entry_new();
    GtkWidget *submit_button = gtk_button_new_with_label("Ajouter");
    g_signal_connect(submit_button, "clicked", G_CALLBACK(submit_button_clicked), userData);

    gtk_grid_attach(GTK_GRID(form_grid), label_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_nom, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_prenom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_prenom, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), submit_button, 0, 2, 2, 1);

    gtk_widget_show_all(GTK_WIDGET(userData->form_window));
}
void ajouter_patient(PGconn *conn, int id_patient, int id_medecin, const char *nom, const char *prenom) {
    char query[512];
    snprintf(query, sizeof(query), "UPDATE utilisateur SET id_medecin = %d WHERE id_utilisateur = %d AND nom = '%s' AND prenom = '%s'", id_medecin, id_patient, nom, prenom);

    PGresult *result = PQexec(conn, query);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erreur lors de la mise à jour du patient: %s\n", PQerrorMessage(conn));
    } else {
        printf("Médecin attribué avec succès pour le patient: Nom: %s, Prénom: %s, ID Médecin: %d\n", nom, prenom, id_medecin);
    }
    PQclear(result);
}
// Fonction callback pour le bouton de soumission du formulaire
static void submit_button_clicked(GtkButton *button, gpointer user_data) {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);
    if (PQstatus(db) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s\n", PQerrorMessage(db));
        PQfinish(db);
        return;
    }
    user *userData = (user *)user_data;
    const char *nom = gtk_entry_get_text(GTK_ENTRY(userData->entry_nom));
    const char *prenom = gtk_entry_get_text(GTK_ENTRY(userData->entry_prenom));

    int id_patient = get_idPatient(nom, prenom);
    if (id_patient == -1) {
        fprintf(stderr, "Erreur lors de la récupération de l'ID de l'utilisateur.\n");
        return;
    }
    int id_medecin;
    char *nom_med, *prenom_med, *telephone;
    get_medecin(db, &nom_med, &prenom_med, &id_medecin, userData->email);

    if (id_medecin == 0) {
        fprintf(stderr, "Aucun médecin trouvé ou erreur de base de données.\n");
    }

    printf("Nom: %s, Prénom: %s, id_patient: %d, id_medecin: %d\n", nom, prenom, id_patient, id_medecin);
    ajouter_patient(db, id_patient, id_medecin, nom, prenom);
    gtk_widget_destroy(GTK_WIDGET(userData->form_window));
}
void medecin_page(PGconn *db, const char *email) {
    const char *query = "SELECT nom, prenom, id_utilisateur, telephone FROM utilisateur WHERE email = $1 AND type = 'Medecin'";
    const char *values[1] = {email};
    PGresult *result = PQexecParams(db, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
        const char *nom = PQgetvalue(result, 0, 0);
        const char *prenom = PQgetvalue(result, 0, 1);
        int id_utilisateur = atoi(PQgetvalue(result, 0, 2));
        const char *telephone = PQgetvalue(result, 0, 3);

        GtkWidget *window, *header_bar, *grid, *label_welcome, *profile_button, *patient_treeview, *appointment_treeview, *label_patient_list, *label_rv_list;
        user *userData = (user*)malloc(sizeof(user));
        userData->db = db;
        userData->email = email;
        userData->nom = nom;
        userData->prenom = prenom;
        userData->telephone = telephone;

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Espace Médecin");
        gtk_window_set_default_size(GTK_WINDOW(window), 1000, 900);
        g_signal_connect(window, "destroy", G_CALLBACK(window_close), NULL);

        header_bar = gtk_header_bar_new();
        gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
        gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Espace Médecin");
        gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

        grid = gtk_grid_new();
        gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);  // Faux pour permettre une distribution inégale de l'espace
        gtk_container_add(GTK_CONTAINER(window), grid);

        char *welcome_message = g_strdup_printf("Bienvenue sur l'espace Medecin, %s %s!", nom, prenom);
        label_welcome = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label_welcome), g_strdup_printf("<span font_desc='Arial 20'>%s</span>", welcome_message));
        g_free(welcome_message);

        // Définit un padding à gauche et à droite pour le label, mais pas d'expansion
        gtk_widget_set_margin_start(label_welcome, 20);
        gtk_widget_set_margin_end(label_welcome, 100);
        gtk_widget_set_hexpand(label_welcome, FALSE);  // Empêche l'expansion horizontale
        gtk_grid_attach(GTK_GRID(grid), label_welcome, 0, 0, 1, 1);

        // Bouton d'ajout de patient
        GtkWidget *add_patient_button = gtk_button_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
        g_signal_connect(add_patient_button, "clicked", G_CALLBACK(ajouter_patient_button_clicked), userData);
        gtk_grid_attach(GTK_GRID(grid), add_patient_button, 1, 0, 1, 1);  // Colonne suivante, même ligne
        // Bouton d'ajout de rendez-vous
        GtkWidget *add_rv_button = gtk_button_new_from_icon_name("appointment-new", GTK_ICON_SIZE_BUTTON);
        g_signal_connect(add_rv_button, "clicked", G_CALLBACK(on_ajouter_rv_button_clicked), userData);
        gtk_grid_attach(GTK_GRID(grid), add_rv_button, 2, 0, 1, 1);  // Colonne suivante, même ligne
        // Bouton de profil
        profile_button = gtk_button_new_from_icon_name("user-info-symbolic", GTK_ICON_SIZE_DIALOG);
        g_signal_connect(profile_button, "clicked", G_CALLBACK(on_profile_button), userData);
        gtk_grid_attach(GTK_GRID(grid), profile_button, 3, 0, 1, 1);  // Colonne suivante, même ligne
        // Liste des patients
        label_patient_list = gtk_label_new("Liste des patients:");
        gtk_grid_attach(GTK_GRID(grid), label_patient_list, 0, 1, 3, 1); // Sur toute la largeur de la grille

        patient_treeview = patient_list(db, id_utilisateur);
        gtk_grid_attach(GTK_GRID(grid), patient_treeview, 0, 2, 3, 1); // Sur toute la largeur de la grille
        // Liste des rendez-vous
        label_rv_list = gtk_label_new("Liste des rendez-vous:");
        gtk_grid_attach(GTK_GRID(grid), label_rv_list, 0, 3, 3, 1); // Sur toute la largeur de la grille

        appointment_treeview = rv_list(db, id_utilisateur);
        gtk_grid_attach(GTK_GRID(grid), appointment_treeview, 0, 4, 3, 1); // Sur toute la largeur de la grille

        gtk_widget_show_all(window);

        PQclear(result);
    } else {
        g_error("Erreur lors de la récupération des données du médecin : %s\n", PQerrorMessage(db));
    }
}