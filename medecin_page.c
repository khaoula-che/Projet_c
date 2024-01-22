/*
    SeniorCare
    Created by : CHETIOUI Khaoula
    Description : interface medecin pour gérer les rendez-vous , consulter et mettre à jour les dossiers médicaux des patients, et bien d'autres fonctionnalités.
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include "bdd.h"

// constantes pour les colonnes
// Index des colonnes

#define NOM_COLUMN 0
#define PRENOM_COLUMN 1
#define NOM_MED 0
#define COL_DESCRIPTION 1
#define COL_DOSAGE 2

int id_urgence_stocked;
int id_patient;


static GtkWidget *name_entry;
static GtkWidget *file_chooser;
static GtkWidget *file_window;
static GtkWidget *status_label;

typedef struct {
    PGconn *db;
    const char *email;
    const char *nom;
    const char *prenom;
    const char *telephone;
    const char *mot_de_passe;
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
    int id_medecin;
} patient;

typedef struct {
    user *userData;
    GtkWidget *entry_nom;
    GtkWidget *entry_prenom;
    GtkWidget *entry_date_de_naissance;
    GtkWidget *entry_numero_telephone;
    GtkWidget *entry_password;
} user_data_widgets;

typedef struct {
    char *nom;
    char *description;
    char *dosage;
    char *nom_patient;
    char *prenom_patient;
    int id_patient;

    GtkWidget *entry_nom;
    GtkWidget *entry_description;
    GtkWidget *entry_dosage;
}medicament;

typedef struct {
    int id_rendezvous;
    int id_utilisateur;
} rappel;


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
static void on_ajouter_rv_button_clicked(GtkButton *button, gpointer user_data);
static void submit_button_clicked(GtkButton *button, gpointer user_data);
void on_ajouter_button_clicked(GtkButton *button, gpointer user_data);
void add_column(GtkWidget *view, const char *title, int column_id);
GtkWidget *tree_view(GtkListStore *list_store, const char *column_titles[], int num_columns);
GtkWidget *patient_list(PGconn *db, int id_utilisateur);
GtkWidget *rv_list(PGconn *db, int id_utilisateur);

void delete_patient(GtkWidget *widget, gpointer data);
void add_medecament(GtkWidget *widget, gpointer user_data);
void get_idPatient_med(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
void delete_med_bouton(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
void cell_render_func_med(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);
void add_delete_column(GtkWidget *treeview);
void file_button_clicked(GtkButton *button, gpointer user_data);
void button_details_clicked(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
void delete_rv_clicked(GtkWidget *widget, gpointer user_data);
void rappel_clicked(GtkWidget *widget, gpointer user_data);
void button_active(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
void cell_render_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);
void add_button_column(GtkWidget *treeview);
void cell_render_func_patient(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);
void add_button_column_details(GtkWidget *treeview);
void display_emergency_messages(GtkWidget *grid, PGconn *db, int id_medecin, int row);
void on_delete_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data);
static void ajouter_patient_button_clicked(GtkButton *button, gpointer user_data);
int verify_rv(PGconn *conn, int id_medecin, const char *date, int heure, int minute);

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


void modify_profil(user *userData) {
    if (userData == NULL) {
        g_print("Error: Null user data\n");
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
    GtkWidget *entry_numero_telephone = gtk_entry_new();
    GtkWidget *entry_password = gtk_entry_new();

    GtkWidget *label_nom = gtk_label_new("Nouveau nom:");
    GtkWidget *label_prenom = gtk_label_new("Nouveau prénom:");
    GtkWidget *label_numero_telephone = gtk_label_new("Nouveau numéro de téléphone:");
    GtkWidget *label_password = gtk_label_new("Nouveau mot de passe:");

    GtkWidget *save_button = gtk_button_new_with_label("Enregistrer");

    gtk_grid_attach(GTK_GRID(grid), label_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_nom, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_prenom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_prenom, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_numero_telephone, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_numero_telephone, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), save_button, 0, 4, 2, 1);

    user_data_widgets *udw = g_new0(user_data_widgets, 1);
    udw->userData = userData;
    udw->entry_nom = entry_nom;
    udw->entry_prenom = entry_prenom;
    udw->entry_numero_telephone = entry_numero_telephone;
    udw->entry_password = entry_password;

    gtk_entry_set_text(GTK_ENTRY(entry_nom), userData->nom);
    gtk_entry_set_text(GTK_ENTRY(entry_prenom), userData->prenom);
    gtk_entry_set_text(GTK_ENTRY(entry_numero_telephone), userData->telephone);

    g_signal_connect(save_button, "clicked", G_CALLBACK(save_button_clicked), udw);

    gtk_widget_show_all(modify_window);
}


static void save_button_clicked(GtkButton *button, gpointer data) {
    user_data_widgets *udw = (user_data_widgets *)data;

    g_print("Saving modified profile...\n");

    // Use g_strdup to copy the strings
    const char *new_nom = g_strdup(gtk_entry_get_text(GTK_ENTRY(udw->entry_nom)));
    const char *new_prenom = g_strdup(gtk_entry_get_text(GTK_ENTRY(udw->entry_prenom)));
    const char *new_numero_telephone = g_strdup(gtk_entry_get_text(GTK_ENTRY(udw->entry_numero_telephone)));
    const char *new_password = g_strdup(gtk_entry_get_text(GTK_ENTRY(udw->entry_password)));

    udw->userData->nom = new_nom;
    udw->userData->prenom = new_prenom;
    udw->userData->telephone = new_numero_telephone;
    udw->userData->mot_de_passe = new_password;

    g_print("Going straight to the saving function...\n");
    update_profile(udw->userData);

    gtk_widget_destroy(GTK_WIDGET(button));
    g_free(new_nom);
    g_free(new_prenom);
    g_free(new_numero_telephone);
    g_free(new_password);
}

void update_profile(user *userData) {
    g_print("In the saving function...\n");

    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);


    if (PQstatus(db) != CONNECTION_OK) {
        g_print("Error: Database connection is not OK: %s\n", PQerrorMessage(db));
        PQfinish(db);
        return;
    }

    g_print("Email: %s\n", userData->email);
    g_print("New Nom: %s\n", userData->nom);
    g_print("New Prenom: %s\n", userData->prenom);
    g_print("New Numero de Telephone: %s\n", userData->telephone);
    g_print("New Mot de Passe: %s\n", userData->mot_de_passe);

    const char *email = userData->email;
    const char *new_nom = userData->nom;
    const char *new_prenom = userData->prenom;
    const char *new_numero_telephone = userData->telephone;
    const char *new_mot_de_passe = userData->mot_de_passe;

    const char *update_query = "UPDATE Utilisateur SET nom = $1, prenom = $2, telephone = $3, mot_de_passe = $4 WHERE email = $5";
    const char *update_values[5] = {new_nom, new_prenom, new_numero_telephone, new_mot_de_passe, email};

    PGresult *update_result = PQexecParams(db, update_query, 5, NULL, update_values, NULL, NULL, 0);


    if (PQresultStatus(update_result) != PGRES_COMMAND_OK) {
        g_print("Error updating profile: %s\n", PQerrorMessage(db));
    } else {
        g_print("Profile updated successfully\n");
    }


    PQclear(update_result);
    PQfinish(db);
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

    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), "<span font_desc='17' weight='bold'>Mon profil :</span>");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);

    gtk_widget_set_margin_start(title_label, 10);
    gtk_widget_set_margin_top(title_label, 10);
    gtk_widget_set_margin_bottom(title_label, 15);

    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 2, 1);

    GtkWidget *label_nom = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Nom:</span> %s", nom));
    gtk_widget_set_halign(label_nom, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_nom, 10);

    GtkWidget *label_prenom = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Prénom:</span> %s", prenom));
    gtk_widget_set_halign(label_prenom, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_prenom, 10);
    GtkWidget *label_email = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Email:</span> %s", email));
    gtk_widget_set_halign(label_email, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_email, 10);
    GtkWidget *label_telephone = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Téléphone:</span> %s", telephone));
    gtk_widget_set_halign(label_telephone, GTK_ALIGN_START);

    gtk_widget_set_margin_start(label_telephone, 10);

    gtk_label_set_use_markup(GTK_LABEL(label_nom), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_prenom), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_email), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_telephone), TRUE);

    gtk_grid_attach(GTK_GRID(grid), label_email, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_nom, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_prenom, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_telephone, 0, 4, 1, 1);

    GtkWidget *modify_button = gtk_button_new_with_label("Modifier");
    gtk_widget_set_margin_top(modify_button, 10);

    g_signal_connect(modify_button, "clicked", G_CALLBACK(modify_button_clicked), userData);
    gtk_grid_attach(GTK_GRID(grid), modify_button, 0, 6, 1, 1);

    gtk_widget_show_all(profile_window);
}

static void modify_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Redirecting to the modification page...\n");
    modify_profil((user *)user_data);
}


patient *get_patientDetails(const char *nom, const char *prenom) {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);

    const char *query = "SELECT id_utilisateur, nom, prenom, date_de_naissance, telephone, email, id_medecin FROM utilisateur WHERE nom = $1 AND prenom = $2";
    const char *param_values[2] = {nom, prenom};

    PGresult *result = PQexecParams(db, query, 2, NULL, param_values, NULL, NULL, 0);

    patient *patient = NULL;
    if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
        patient = malloc(sizeof(*patient));
        if (patient) {
            patient->id_utilisateur = atoi(PQgetvalue(result, 0, 0));
            patient->nom = g_strdup(PQgetvalue(result, 0, 1));
            patient->prenom = g_strdup(PQgetvalue(result, 0, 2));
            patient->date_naissance = g_strdup(PQgetvalue(result, 0, 3));
            patient->telephone = g_strdup(PQgetvalue(result, 0, 4));
            patient->email = g_strdup(PQgetvalue(result, 0, 5));
            patient->id_medecin = atoi(PQgetvalue(result, 0, 6));
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


void delete_patient(GtkWidget *widget, gpointer data) {
    PGconn *db = PQconnectdb(data);
    GtkTreeView *treeview = GTK_TREE_VIEW(data);
    GtkTreeModel *model;
    GtkTreeIter iter;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *nom;
        gchar *prenom;
        gtk_tree_model_get(model, &iter, NOM_COLUMN, &nom, PRENOM_COLUMN, &prenom, -1);

        printf("nom: %s", nom);
        patient *details = get_patientDetails(nom, prenom);
        if (details) {
            int id_medecin = details->id_medecin;
            printf("id_medecin: %d\n", id_medecin);
            const char *query = "UPDATE utilisateur SET id_medecin = NULL WHERE nom = $1 AND prenom = $2";
            const char *values[2] = {details->nom, details->prenom};
            PGresult *result = PQexecParams(db, query, 2, NULL, values, NULL, NULL, 0);
            if (PQresultStatus(result) != PGRES_COMMAND_OK) {
                g_print("\nÉchec : %s", PQresultErrorMessage(result));
            } else {
                g_print("\nLe partient a ete supprime!");
            }
            PQclear(result);
            PQfinish(db);

        } else {
            g_print("Patient details not found for %s %s.\n", nom, prenom);
        }

        // Remove the patient from the GtkTreeModel
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        // Free the allocated memory
        g_free(nom);
        g_free(prenom);
    } else {
        g_print("No patient selected for deletion.\n");
    }
}


void add_medecament(GtkWidget *widget, gpointer user_data) {
    medicament *med_info = (medicament *)user_data;
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);
    char *nom, *prenom;

    patient *details = get_patientDetails(med_info->nom_patient, med_info->prenom_patient);
    if (!details) {
        fprintf(stderr, "Patient details not found\n");
        return;
    }
    int id_patient = details->id_utilisateur;
    char id_patient_str[12];
    sprintf(id_patient_str, "%d", id_patient);

    const char *nom_med = gtk_entry_get_text(GTK_ENTRY(med_info->entry_nom));
    const char *description = gtk_entry_get_text(GTK_ENTRY(med_info->entry_description));
    const char *dosage = gtk_entry_get_text(GTK_ENTRY(med_info->entry_dosage));

    const char *query = "INSERT INTO medicament (nom, description, dosage, id_patient) VALUES ($1, $2, $3, $4)";
    const char *values[4] = {nom_med, description, dosage, id_patient_str};

    PGresult *result = PQexecParams(db, query, 4, NULL, values, NULL, NULL, 0);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "\nÉchec de l'ajout du médicament : %s", PQresultErrorMessage(result));
    } else {
        fprintf(stdout, "\nMédicament ajouté avec succès!");
    }

    PQclear(result);
    PQfinish(db);
}


void get_idPatient_med(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    gchar *nom, *prenom;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, NOM_COLUMN, &nom, PRENOM_COLUMN, &prenom, -1);

        patient *details = get_patientDetails(nom, prenom);
        if (!details) {
            g_free(nom);
            g_free(prenom);
            g_print("Erreur : Détails du patient non trouvés.\n");
            return;
        }

        int id_patient = details->id_utilisateur;
        g_print("ID du patient : %d\n", id_patient);



        g_free(nom);
        g_free(prenom);
    }
}

void delete_med_bouton(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);
    gchar *path_string = gtk_tree_path_to_string(path);
    g_print("Ligne activée : %s\n", path_string);

    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        char *nom_med, *description_med, *dosage_med;

        gtk_tree_model_get(model, &iter,
                           NOM_MED, &nom_med,          // NOM_MED est l'index de la colonne pour le nom
                           COL_DESCRIPTION, &description_med, // COL_DESCRIPTION pour la description
                           COL_DOSAGE, &dosage_med,    // COL_DOSAGE pour le dosage
                           -1);

        g_print("Nom du médicament : %s, Description : %s, Dosage : %s\n", nom_med, description_med, dosage_med);
        g_print("ID du patient : %d\n", id_patient);

        char query[256];
        snprintf(query, sizeof(query), "DELETE FROM medicament WHERE id_patient = %d AND nom = '%s'", id_patient,nom_med);
        PGresult *res = PQexec(db, query);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Échec de la suppression du médicament : %s\n", PQerrorMessage(db));
        } else {
            printf("Medicament supprime avec succès.\n");
        }

        PQclear(res);
        PQfinish(db);

        g_free(nom_med);
        g_free(description_med);
        g_free(dosage_med);

    } else {
        g_print("Impossible de récupérer l'itérateur pour la ligne sélectionnée.\n");
    }

    g_free(path_string);
}

void cell_render_func_med(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data) {
    g_object_set(renderer, "text", "Supprimer", NULL);
}


void add_delete_column(GtkWidget *treeview){
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Supprimer le medecament", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, (GtkTreeCellDataFunc)cell_render_func_med, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    g_signal_connect(treeview, "row-activated", G_CALLBACK( delete_med_bouton), NULL);


}


void file_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("ID du patient : %d\n", id_patient);

    const gchar *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));

    GFile *source_file = g_file_new_for_path(filename);
    gboolean source_exists = g_file_query_exists(source_file, NULL);
    GFile *destination_folder = g_file_new_for_path("docs");

    if (!g_file_query_exists(destination_folder, NULL)) {
        if (!g_file_make_directory_with_parents(destination_folder, NULL, NULL)) {
            g_print("Error lors de la creation .\n");
            return;
        }
    }

    GFile *destination_file = g_file_get_child(destination_folder, g_strconcat(name, ".pdf", NULL));
    gchar *save_path = g_file_get_path(destination_file);

    g_print("File will be saved to: %s\n", save_path);
    g_print("Checking filename and save_path before g_file_copy:\n");
    g_print("filename: %s\n", filename);
    g_print("save_path: %s\n", save_path);

    GError *error = NULL;
    gboolean success = g_file_copy(source_file, destination_file, G_FILE_COPY_NONE, NULL, NULL, NULL, &error);

    if (success) {
        printf("File uploaded successfully\n");

        if (status_label != NULL) {
            gtk_label_set_text(GTK_LABEL(status_label), "File uploaded successfully!");
        }

        PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

        if (PQstatus(conn) == CONNECTION_OK) {
            char query[256];
            snprintf(query, sizeof(query), "INSERT INTO files (file_name, id_patient) VALUES ('%s', %d)",
                     g_path_get_basename(save_path), id_patient);

            PGresult *result = PQexec(conn, query);

            if (PQresultStatus(result) == PGRES_COMMAND_OK) {
                g_print("File information inserted into the database successfully.\n");
            } else {
                g_print("Error: Failed to insert file information into the database.\n");
            }

            PQclear(result);
            PQfinish(conn);

        } else {
            g_print("Error: Failed to connect to the database.\n");
        }

    } else {
        g_print("Error: Failed to copy the file. %s\n", error ? error->message : "Unknown error");

        if (status_label != NULL) {
            gtk_label_set_text(GTK_LABEL(status_label), "Error uploading file.");
        }

        g_error_free(error);
    }

    g_object_unref(source_file);
    g_object_unref(destination_folder);
    g_object_unref(destination_file);

    g_free(name);
    g_free(filename);
    g_free(save_path);


}


void button_details_clicked(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);

    // Création de la fenêtre principale
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Détails du Patient");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 900);

    // Création d'un GtkNotebook
    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    // extraction des détails du patient
    gchar *path_string = gtk_tree_path_to_string(path);
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        g_free(path_string);
        return;
    }

    gchar *nom, *prenom;
    gtk_tree_model_get(model, &iter, NOM_COLUMN, &nom, PRENOM_COLUMN, &prenom, -1);
    patient *details = get_patientDetails(nom, prenom);

    if (!details) {
        g_free(nom);
        g_free(prenom);
        g_print("Erreur : Détails du patient non trouvés.\n");
        return;
    }

    id_patient = details->id_utilisateur;

    g_free(nom);
    g_free(prenom);


    GtkWidget *info_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *label_titre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_titre), "<span font_desc='17' weight='bold'>Détails du patient :</span>");
    gtk_label_set_xalign(GTK_LABEL(label_titre), 0.0);
    gtk_widget_set_margin_top(label_titre, 15);
    gtk_widget_set_margin_start(label_titre, 10);
    gtk_box_pack_start(GTK_BOX(info_page), label_titre, FALSE, FALSE, 5);

    GtkWidget *label_nom = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label_nom), g_strdup_printf("Nom: %s", details->nom));
    gtk_label_set_xalign(GTK_LABEL(label_nom), 0.0); // Alignement à gauche
    gtk_widget_set_margin_start(label_nom, 10);
    gtk_box_pack_start(GTK_BOX(info_page), label_nom, FALSE, FALSE, 5);

    GtkWidget *label_prenom = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label_prenom), g_strdup_printf("Prénom: %s", details->prenom));
    gtk_label_set_xalign(GTK_LABEL(label_prenom), 0.0); // Alignement à gauche
    gtk_widget_set_margin_start(label_prenom, 10);
    gtk_box_pack_start(GTK_BOX(info_page), label_prenom, FALSE, FALSE, 5);

    GtkWidget *label_email = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label_email), g_strdup_printf("Email: %s", details->email));
    gtk_label_set_xalign(GTK_LABEL(label_email), 0.0);
    gtk_widget_set_margin_start(label_email, 10);
    gtk_box_pack_start(GTK_BOX(info_page), label_email, FALSE, FALSE, 5);

    GtkWidget *label_telephone = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label_telephone), g_strdup_printf("Téléphone: %s", details->telephone));
    gtk_label_set_xalign(GTK_LABEL(label_telephone), 0.0);
    gtk_widget_set_margin_start(label_telephone, 10);
    gtk_box_pack_start(GTK_BOX(info_page), label_telephone, FALSE, FALSE, 5);

    GtkWidget *delete_button = gtk_button_new_with_label("Supprimer le patient");
    gtk_widget_set_margin_start(delete_button, 10);
    gtk_widget_set_size_request(delete_button, 100, 30);
    gtk_box_pack_start(GTK_BOX(info_page), delete_button, FALSE, FALSE, 5);
    g_signal_connect_data(G_OBJECT(delete_button), "clicked", G_CALLBACK(delete_patient), treeview, NULL, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), info_page, gtk_label_new("Informations"));

    GtkWidget *medicament_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *med_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(med_title), "<span font_desc='17' weight='bold'>Ajouter un médicament :</span>");
    gtk_label_set_xalign(GTK_LABEL(med_title), 0.0);
    gtk_widget_set_margin_start(med_title, 10);
    gtk_widget_set_margin_top(med_title, 20);
    gtk_widget_set_margin_bottom(med_title, 10);
    gtk_box_pack_start(GTK_BOX(medicament_page), med_title, FALSE, FALSE, 0);

    // Création d'un GtkGrid pour organiser les éléments
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(medicament_page), grid, TRUE, TRUE, 0);

    medicament *med_info = malloc(sizeof(medicament));
    med_info->nom_patient = details->nom;
    med_info->prenom_patient = details->prenom;

    // Champ pour le nom du médicament
    med_info->entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(med_info->entry_nom), "Nom du médicament");
    gtk_widget_set_margin_start(med_info->entry_nom, 10);
    gtk_widget_set_size_request(med_info->entry_nom, 250, 30);
    gtk_grid_attach(GTK_GRID(grid), med_info->entry_nom, 10, 0, 1, 1);

    med_info->entry_description = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(med_info->entry_description), "Description");
    gtk_widget_set_margin_start(med_info->entry_description, 10);
    gtk_widget_set_size_request(med_info->entry_description, 250, 30);
    gtk_grid_attach(GTK_GRID(grid), med_info->entry_description, 10, 1, 1, 1);

    med_info->entry_dosage = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(med_info->entry_dosage), "Dosage");
    gtk_widget_set_margin_start(med_info->entry_dosage, 10);
    gtk_widget_set_size_request(med_info->entry_dosage, 250, 30);
    gtk_grid_attach(GTK_GRID(grid), med_info->entry_dosage, 10, 2, 1, 1);

    // Bouton pour soumettre le formulaire
    GtkWidget *submit_button = gtk_button_new_with_label("Ajouter");
    gtk_widget_set_margin_start(submit_button, 10);
    gtk_widget_set_size_request(submit_button, 250, 30);
    g_signal_connect(G_OBJECT(submit_button), "clicked", G_CALLBACK(add_medecament), med_info);
    gtk_grid_attach(GTK_GRID(grid), submit_button, 10, 3, 1, 1);
    gtk_widget_set_margin_bottom(submit_button, 0);


    GtkWidget *list_med_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_med_title), "<span font_desc='17' weight='bold'>La liste des médicaments :</span>");
    gtk_label_set_xalign(GTK_LABEL(list_med_title), 0.0);
    gtk_widget_set_margin_start(list_med_title, 10);
    gtk_widget_set_margin_top(list_med_title, 10);
    gtk_widget_set_margin_bottom(list_med_title, 10);
    gtk_box_pack_start(GTK_BOX(medicament_page), list_med_title, FALSE, FALSE, 0);

    // Récupération et affichage de la liste des médicaments
    int id_patient = details->id_utilisateur;
    char query[256];
    snprintf(query, sizeof(query), "SELECT nom, description, dosage FROM medicament WHERE id_patient = %d", id_patient);
    PGresult *res = PQexec(db, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Échec de la requête SELECT : %s", PQerrorMessage(db));
    } else {
        printf("Nombre de médicaments récupérés : %d\n", PQntuples(res));
    }
    med_info->id_patient = id_patient;
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter2;

    int rows = PQntuples(res);

    for (int i = 0; i < rows; i++) {
        medicament med;
        med.nom = g_strdup(PQgetvalue(res, i, 0));
        med.description = g_strdup(PQgetvalue(res, i, 1));
        med.dosage = g_strdup(PQgetvalue(res, i, 2));

        gtk_list_store_append(store, &iter2);
        gtk_list_store_set(store, &iter2, 0, med.nom, 1, med.description, 2, med.dosage, -1);

        g_free(med.nom);
        g_free(med.description);
        g_free(med.dosage);
    }

    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_box_pack_start(GTK_BOX(medicament_page), scrolled_window, TRUE, TRUE, 0);

    // Colonne pour le nom du médicament
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();  // Crée un cell renderer pour le texte
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    // Colonne pour la description
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    // Colonne pour le dosage
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Dosage", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    add_delete_column(tree_view);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), medicament_page, gtk_label_new("Médicament"));

    // page messagerie
    GtkWidget *message_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), message_page, gtk_label_new("Messagerie"));

    // Document page
    GtkWidget *doc_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    g_print("In display add documents...\n");

    GtkWidget *fic_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(fic_title), "<span font_desc='17' weight='bold'>Ajouter un document :</span>");
    gtk_label_set_xalign(GTK_LABEL(fic_title), 0.0);
    gtk_widget_set_margin_start(fic_title, 10);
    gtk_widget_set_margin_top(fic_title, 20);
    gtk_widget_set_margin_bottom(fic_title, 10);
    gtk_box_pack_start(GTK_BOX(doc_page), fic_title, FALSE, FALSE, 0);

    GtkWidget *doc_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(doc_grid), 5);
    gtk_box_pack_start(GTK_BOX(doc_page), doc_grid, FALSE, FALSE, 0);

    GtkWidget *doc_name_label = gtk_label_new("Nom du fichier:");
    gtk_widget_set_margin_start(doc_name_label, 10);
    gtk_grid_attach(GTK_GRID(doc_grid), doc_name_label, 0, 0, 1, 1);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_widget_set_margin_start(name_entry, 10);
    gtk_grid_attach(GTK_GRID(doc_grid), name_entry, 1, 0, 1, 1);

    GtkWidget *doc_file_label = gtk_label_new("Ajouter le fichier:");
    gtk_widget_set_margin_start(doc_file_label, 10);
    gtk_grid_attach(GTK_GRID(doc_grid), doc_file_label, 0, 1, 1, 1);

    GtkWidget *file_chooser = gtk_file_chooser_button_new("Envoyer ", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_widget_set_margin_start(file_chooser , 10);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser), g_get_home_dir());
    gtk_grid_attach(GTK_GRID(doc_grid), file_chooser, 1, 1, 1, 1);

    GtkWidget *doc_submit_button = gtk_button_new_with_label("Envoyer");
    gtk_widget_set_margin_start(doc_submit_button, 10);
    g_signal_connect(doc_submit_button, "clicked", G_CALLBACK(file_button_clicked), user_data);
    gtk_grid_attach(GTK_GRID(doc_grid), doc_submit_button, 0, 2, 2, 1);

    GtkWidget *list_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_title), "<span font_desc='17' weight='bold'>La liste des documents :</span>");
    gtk_label_set_xalign(GTK_LABEL(list_title), 0.0);
    gtk_widget_set_margin_start(list_title, 10);
    gtk_widget_set_margin_top(list_title, 20);
    gtk_widget_set_margin_bottom(list_title, 10);

    gtk_box_pack_start(GTK_BOX(doc_page), list_title, FALSE, FALSE, 0);


    const char *query2 = "SELECT file_name FROM files WHERE id_patient = %d";
    char formatted_query[256];
    snprintf(formatted_query, sizeof(formatted_query), query2, id_patient);

    PGresult *result = PQexec(db, formatted_query);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Échec de la requête SELECT : %s", PQerrorMessage(db));
    } else {
        printf("Nombre de documents récupérés : %d\n", PQntuples(result));
    }

    GtkListStore *store2 = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter iter3;

    int rows2 = PQntuples(result);
    for (int i = 0; i < rows2; i++) {
        const gchar *nom_fichier = PQgetvalue(result, i, 0);

        // Ajouter le nom du fichier au modèle de liste
        gtk_list_store_append(store2, &iter3);
        gtk_list_store_set(store2, &iter3, 0, nom_fichier, -1);
    }

    GtkWidget *tree_view2 = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store2));
    GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("Nom du fichier", renderer2, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view2), column2);

// Ajouter la vue de liste à la page de documents
    gtk_box_pack_start(GTK_BOX(doc_page), tree_view2, TRUE, TRUE, 0);

    PQclear(result);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), doc_page, gtk_label_new("Documents"));

    gtk_widget_show_all(window);
}

void add_column(GtkWidget *view, const char *title, int column_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", column_id, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
}


void delete_rv_clicked(GtkWidget *widget, gpointer user_data) {
    rappel *data = (rappel *)user_data;
    if (!data) return;

    const char *data_connexion = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data_connexion);

    if (PQstatus(db) == CONNECTION_BAD) {
        fprintf(stderr, "Connexion à la base de données échouée : %s\n", PQerrorMessage(db));
        PQfinish(db);
        return;
    }
    char query_rappel[256];
    snprintf(query_rappel, sizeof(query_rappel), "DELETE FROM rappel WHERE rendez_vous = %d", data->id_rendezvous);
    PGresult *result_rappel = PQexec(db, query_rappel);

    if (PQresultStatus(result_rappel) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Échec de la suppression dans rappel : %s\n", PQerrorMessage(db));
        PQclear(result_rappel);
        PQfinish(db);
        free(data);
        return;
    }
    PQclear(result_rappel);

    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM rendez_vous WHERE id_rendezvous = %d", data->id_rendezvous);

    PGresult *result = PQexec(db, query);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Échec de la suppression du rendez-vous : %s\n", PQerrorMessage(db));
    } else {
        printf("Rendez-vous supprimé avec succès.\n");
    }
    PQclear(result);
    PQfinish(db);
    free(data);
}

void rappel_clicked(GtkWidget *widget, gpointer user_data) {
    rappel *data = (rappel *)user_data;
    if (!data) return;

    const char *data_connexion = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data_connexion);

    if (db != NULL) {
        char id_rendezvous_str[12], id_utilisateur_str[12];
        snprintf(id_utilisateur_str, sizeof(id_utilisateur_str), "%d", data->id_utilisateur);
        snprintf(id_rendezvous_str, sizeof(id_rendezvous_str), "%d", data->id_rendezvous);

        const char *values[2] = {id_rendezvous_str, id_utilisateur_str};
        const char *insert_query = "INSERT INTO rappel (rendez_vous, conseil, meteo, message, id_patient) VALUES ($1, NULL, NULL, 'Rappel de votre rendez-vous', $2)";
        PGresult *result = PQexecParams(db, insert_query, 2, NULL, values, NULL, NULL, 0);

        if (PQresultStatus(result) != PGRES_COMMAND_OK) {
            g_print("Échec de l'insertion : %s", PQresultErrorMessage(result));
        } else {
            g_print("Rappel envoyé !");
        }
        PQclear(result);
        PQfinish(db);
    } else {
        g_print("Échec de la connexion à la base de données.");
    }
    free(data);
}

// lorsque le bouton est cliqué.
void button_active(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    gchar *path_string = gtk_tree_path_to_string(path);
    g_print("Ligne activée : %s\n", path_string);

    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        g_free(path_string);
        return;
    }

    rappel *data = malloc(sizeof(rappel));
    if (!data) return;

    gchar *nom, *prenom;
    gtk_tree_model_get(model, &iter, NOM_COLUMN, &nom, PRENOM_COLUMN, &prenom, -1);

    data->id_utilisateur = get_idPatient(nom, prenom);
    gtk_tree_model_get(model, &iter, 6, &data->id_rendezvous, -1);

    g_free(nom);
    g_free(prenom);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Actions sur le Rendez-vous");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *btn_send_reminder = gtk_button_new_with_label("Envoie de rappel");
    g_signal_connect(btn_send_reminder, "clicked", G_CALLBACK(rappel_clicked), data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_send_reminder, TRUE, TRUE, 0);

    GtkWidget *btn_delete_rv = gtk_button_new_with_label("Supprimer le rendez-vous");
    g_signal_connect(btn_delete_rv, "clicked", G_CALLBACK(delete_rv_clicked), data);
    gtk_box_pack_start(GTK_BOX(vbox), btn_delete_rv, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
}

void cell_render_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data) {
    // appelée pour chaque ligne de la vue arborescente pour configurer le rendu de la cellule
    g_object_set(renderer, "text", "Voir", NULL);
}


void add_button_column(GtkWidget *treeview) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Action", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, (GtkTreeCellDataFunc)cell_render_func, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // Connecter le signal "row-activated" pour la vue arborescente
    g_signal_connect(treeview, "row-activated", G_CALLBACK(button_active), NULL);}


void cell_render_func_patient(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data) {
    // Cette fonction est appelée pour chaque ligne de la vue arborescente pour configurer le rendu de la cellule
    g_object_set(renderer, "text", "Voir plus", NULL);
}

void add_button_column_details(GtkWidget *treeview) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Details", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, (GtkTreeCellDataFunc)cell_render_func_patient, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
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
    add_button_column(treeview); //ajoute la colonne envoyer
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
        int num_tuples = PQntuples(result);

        if (num_tuples == 0) {
            printf("Aucun rendez-vous trouvé.\n");
            printf("Nombre de rendez-vous trouvés : %d\n", num_tuples);
            GtkTreeIter iter;
            gtk_list_store_append(rv_store, &iter);
            gtk_list_store_set(rv_store, &iter, 0, "Aucun rendez-vous trouvé", 1, "", 2, "", 3, "", 4, "", 5, "", 6, 0, -1);

        } else {
            for (int i = 0; i < num_tuples; i++) {
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
    gtk_window_set_default_size(userData->form_window, 600, 900);

    GtkWidget *form_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(userData->form_window), form_grid);


    GtkWidget *form_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(form_title), "<span font_desc='17' weight='bold'>Formulaire d'ajout de rendez-vous :</span>");
    gtk_widget_set_halign(form_title, GTK_ALIGN_START);
    gtk_widget_set_margin_start(form_title, 10);
    gtk_widget_set_margin_top(form_title, 10);
    gtk_widget_set_margin_bottom(form_title, 15);
    gtk_grid_attach(GTK_GRID(form_grid), form_title, 0, 0, 2, 1);

    GtkWidget *label_nom = gtk_label_new("Nom:");
    gtk_widget_set_halign(label_nom, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_nom, 10);
    userData->entry_nom = gtk_entry_new();

    GtkWidget *label_prenom = gtk_label_new("Prénom:");
    gtk_widget_set_halign(label_prenom, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_prenom, 10);
    userData->entry_prenom = gtk_entry_new();

    GtkWidget *label_description = gtk_label_new("Description:");
    gtk_widget_set_halign(label_description, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_description, 10);
    userData->entry_description = gtk_entry_new();

    GtkWidget *label_adresse = gtk_label_new("Adresse:");
    gtk_widget_set_halign(label_adresse, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_adresse, 10);
    userData->entry_adresse = gtk_entry_new();

    GtkWidget *label_calendar = gtk_label_new("Date:");
    gtk_widget_set_halign(label_calendar, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_calendar, 10);
    userData->calendar = gtk_calendar_new();

    GtkWidget *label_heure = gtk_label_new("Heure:");
    gtk_widget_set_halign(label_heure, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_heure, 10);

    userData->spin_button_heure = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 23, 1));

    GtkWidget *label_minute = gtk_label_new("Minute:");
    gtk_widget_set_halign(label_minute, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_minute, 10);

    userData->spin_button_minute = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 59, 1));
    GtkWidget *submit_button = gtk_button_new_with_label("Ajouter");
    gtk_widget_set_margin_top(submit_button, 20);

    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_rv_button_clicked), userData);

    gtk_grid_attach(GTK_GRID(form_grid), label_nom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_nom, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), label_prenom, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_prenom, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), label_description, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_description, 1, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), label_adresse, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_adresse, 1, 4, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), label_calendar, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), userData->calendar, 1, 5, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), label_heure, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), GTK_WIDGET(userData->spin_button_heure), 1, 6, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), label_minute, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), GTK_WIDGET(userData->spin_button_minute), 1, 7, 1, 1);

    gtk_grid_attach(GTK_GRID(form_grid), submit_button, 0, 8, 2, 1);

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


    if (nom == NULL || prenom == NULL) {
        fprintf(stderr, "Erreur : Les widgets GTK ne sont pas correctement initialisés.\n");
        return;
    }

    if (nom[0] == '\0' || prenom[0] == '\0') {

        fprintf(stderr, "Erreur : Les champs Nom et prenom ne doivent pas être vides.\n");
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

    int id_medecin;
    char *nom_med, *prenom_med, *telephone;

    get_medecin(db, &nom_med, &prenom_med, &id_medecin, userData->email);

    if (id_medecin == 0) {
        fprintf(stderr, "Aucun médecin trouvé ou erreur de base de données.\n");
    }

    if (id_medecin == -1) {
        fprintf(stderr, "Erreur lors de la récupération de l'id du médecin.\n");
        return;
    }

    // Construire la chaîne de date et d'heure
    char dateTimeStr[16];
    snprintf(dateTimeStr, sizeof(dateTimeStr), "%04d-%02d-%02d %02d:%02d", year, month + 1, day, heure, minute);

    if (verify_rv(db, id_medecin, dateTimeStr, heure, minute) > 0) {
        fprintf(stderr, "Erreur : La date et l'heure sont déjà prises.\n");
        return;

    } else {
        ajouter_rendez_vous(db, id_patient, id_medecin, dateStr, heure, minute, description, lieu);
        gtk_widget_destroy(GTK_WIDGET(userData->form_window));

    }

}


int verify_rv(PGconn *conn, int id_medecin, const char *date, int heure, int minute) {
        char query[256];
        snprintf(query, sizeof(query),
                 "SELECT COUNT(*) FROM rendez_vous WHERE id_medecin = %d AND date = '%s' AND heure = '%02d:%02d'",
                 id_medecin, date, heure, minute);

        PGresult *result = PQexec(conn, query);

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            fprintf(stderr, "Erreur lors de la vérification du rendez-vous : %s\n", PQerrorMessage(conn));
            PQclear(result);
            return -1; // Erreur lors de l'exécution de la requête
        }

        int count = atoi(PQgetvalue(result, 0, 0));
        PQclear(result);
        return count > 0; // renvoie 1 si le rendez-vous est pris, 0 sinon

}


void ajouter_rendez_vous(PGconn *conn, int id_patient, int id_medecin, const char *date, int heure, int minute, const char *description, const char *lieu) {
    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO rendez_vous (id_patient, id_medecin, date, heure, description, lieu) VALUES (%d, %d, '%s', '%02d:%02d', '%s', '%s')", id_patient, id_medecin, date, heure, minute, description, lieu);

    PGresult *result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Erreur lors de l'ajout du rendez-vous: %s\n", PQerrorMessage(conn));
    }
    else {
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
    gtk_window_set_default_size(userData->form_window, 500, 400);

    GtkWidget *form_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(userData->form_window), form_grid);


    GtkWidget *form_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(form_title), "<span font_desc='17' weight='bold'>Formulaire d'ajout de patient :</span>");
    gtk_widget_set_halign(form_title, GTK_ALIGN_START);
    gtk_widget_set_margin_start(form_title, 10);
    gtk_widget_set_margin_top(form_title, 10);
    gtk_widget_set_margin_bottom(form_title, 15);
    gtk_grid_attach(GTK_GRID(form_grid), form_title, 0, 0, 2, 1);

    GtkWidget *label_nom = gtk_label_new("Nom:");
    userData->entry_nom = gtk_entry_new();

    GtkWidget *label_prenom = gtk_label_new("Prénom:");
    userData->entry_prenom = gtk_entry_new();

    GtkWidget *submit_button = gtk_button_new_with_label("Ajouter");
    g_signal_connect(submit_button, "clicked", G_CALLBACK(submit_button_clicked), userData);

    gtk_grid_attach(GTK_GRID(form_grid), label_nom, 0, 1, 1, 1);

    gtk_widget_set_halign(label_nom, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label_nom, 10);
    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_nom, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_prenom, 0, 2, 1, 1);

    gtk_widget_set_margin_start(label_prenom, 10);
    gtk_widget_set_halign(label_prenom, GTK_ALIGN_START);

    gtk_grid_attach(GTK_GRID(form_grid), userData->entry_prenom, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), submit_button, 0, 3, 2, 1);

    gtk_widget_set_margin_start(submit_button, 10);
    gtk_widget_set_margin_top(submit_button, 10);

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
        fprintf(stderr, "Erreur lors de la récupération de l'id de l'utilisateur.\n");
        return;
    }
    int id_medecin;
    char *nom_med, *prenom_med, *telephone;

    get_medecin(db, &nom_med, &prenom_med, &id_medecin, userData->email);

    if (id_medecin == 0) {
        fprintf(stderr, "Aucun medecin trouvé\n");
    }

    printf("Nom: %s, Prénom: %s, id_patient: %d, id_medecin: %d\n", nom, prenom, id_patient, id_medecin);
    ajouter_patient(db, id_patient, id_medecin, nom, prenom);



    gtk_widget_destroy(GTK_WIDGET(userData->form_window));

}


void on_delete_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data) {
    int id_urgence_stocked = GPOINTER_TO_INT(user_data);
    PGconn *db = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        // Retrieve the ID of the urgency from the model
        int id_urgence_model;
        gtk_tree_model_get(model, &iter, 0, &id_urgence_model, -1);
        printf("ID urgence from model: %d\n", id_urgence_model);



        char *query = g_strdup_printf("DELETE FROM urgence WHERE id_urgence = %d", id_urgence_stocked);
        PGresult *result = PQexec(db, query);


        if (PQresultStatus(result) != PGRES_COMMAND_OK) {
            g_error("Échec de la suppression de l'urgence : %s\n", PQerrorMessage(db));
        }

        else {

            gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

            PQclear(result);
            g_free(query);
        }
    }
}


// Fonction pour afficher les incidents d'urgence
void display_emergency_messages(GtkWidget *grid,PGconn *db, int id_medecin, int row) {
    const char *query = "SELECT urgence.id_urgence, urgence.date_incident, utilisateur.nom, utilisateur.prenom "
                        "FROM urgence "
                        "JOIN utilisateur ON urgence.id_patient = utilisateur.id_utilisateur "
                        "WHERE urgence.id_medecin = $1 AND utilisateur.id_medecin = $1";


    char *id_medecin_str = g_strdup_printf("%d", id_medecin);
    const char *values[1] = {id_medecin_str};

    PGresult *result = PQexecParams(db, query, 1, NULL, values, NULL, NULL, 0);

    g_free(id_medecin_str);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        g_error("Échec de la requête SELECT : %s\n", PQerrorMessage(db));
        return;
    }

    int rows = PQntuples(result);

    if (rows == 0) {
        // Aucune urgence, afficher un message
        GtkWidget *label_no_emergency = gtk_label_new("Aucune urgence enregistrée.");
        gtk_grid_attach(GTK_GRID(grid), label_no_emergency, 0, row, 1, 1);
        PQclear(result);
        return;
    }

    // Creation d'un GtkListStore avec trois colonnes
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeIter iter;

    for (int i = 0; i < rows; i++) {
        int id_urgence = atoi(PQgetvalue(result, i, 0)); // Convertir en int
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, id_urgence, // ID de l'urgence
                           1, PQgetvalue(result, i, 1), // Date d'incident
                           2, PQgetvalue(result, i, 2), // Nom
                           3, PQgetvalue(result, i, 3), // Prénom
                           4, "Supprimer", // Texte du bouton
                           -1);

        id_urgence_stocked = id_urgence;

    }

    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view), -1, "Date d'incident", gtk_cell_renderer_text_new(), "text", 1, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view), -1, "Prénom", gtk_cell_renderer_text_new(), "text", 2, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view), -1, "Nom", gtk_cell_renderer_text_new(), "text", 3, NULL);

    GtkCellRenderer *renderer_delete = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column_delete = gtk_tree_view_column_new_with_attributes("Supprimer", renderer_delete, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_delete);

    g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_delete_clicked), GINT_TO_POINTER(id_urgence_stocked));



    gtk_grid_attach(GTK_GRID(grid), tree_view, 0, row, 3, 1);


    g_object_unref(store);
    PQclear(result);
}
void add_column_rappels(GtkWidget *view, const char *title, int column_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", column_id, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
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

        user *userData = (user*)malloc(sizeof(user));

        userData->db = db;
        userData->email = email;
        userData->nom = nom;
        userData->prenom = prenom;
        userData->telephone = telephone;

        // Creation de la fenêtre principale
        GtkWidget *grid, *patient_treeview, *appointment_treeview;

        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Espace Médecin");
        gtk_window_set_default_size(GTK_WINDOW(window), 1000, 900);
        g_signal_connect(window, "destroy", G_CALLBACK(window_close), NULL);

        // Creation et configuration de la barre de titre
        GtkWidget *header_bar = gtk_header_bar_new();
        gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
        gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Espace Médecin");
        gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

        // Création des grilles pour l'organisation de la fenêtre
        GtkWidget *grid_main = gtk_grid_new();
        GtkWidget *grid_top = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(window), grid_main);
        gtk_grid_attach(GTK_GRID(grid_main), grid_top, 0, 0, 1, 1);

        // configuration du message de bienvenue
        char *welcome_message = g_strdup_printf("Bienvenue sur l'espace Medecin, %s %s!", nom, prenom);
        GtkWidget *label_welcome = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label_welcome), g_strdup_printf("<span font_desc='Arial 20'>%s</span>", welcome_message));
        g_free(welcome_message);

        gtk_widget_set_hexpand(label_welcome, TRUE);
        gtk_widget_set_margin_start(label_welcome, 20);
        gtk_widget_set_margin_end(label_welcome, 20);
        gtk_grid_attach(GTK_GRID(grid_top), label_welcome, 0, 0, 1, 1);

        GtkWidget *add_patient_button = gtk_button_new();
        GtkImage *add_patient_image = gtk_image_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_MENU);
        gtk_button_set_image(GTK_BUTTON(add_patient_button), GTK_WIDGET(add_patient_image));
        g_signal_connect(add_patient_button, "clicked", G_CALLBACK(ajouter_patient_button_clicked), userData);
        gtk_widget_set_margin_end(add_patient_button, 0);
        gtk_grid_attach(GTK_GRID(grid_top), add_patient_button, 1, 0, 1, 1);

        GtkWidget *add_rv_button = gtk_button_new();
        GtkImage *add_rv_image = gtk_image_new_from_icon_name("appointment-new", GTK_ICON_SIZE_MENU);
        gtk_button_set_image(GTK_BUTTON(add_rv_button), GTK_WIDGET(add_rv_image));
        g_signal_connect(add_rv_button, "clicked", G_CALLBACK(on_ajouter_rv_button_clicked), userData);
        gtk_widget_set_margin_end(add_rv_button, 0);
        gtk_grid_attach(GTK_GRID(grid_top), add_rv_button, 2, 0, 1, 1);

        GtkWidget *profile_button = gtk_button_new();
        GtkImage *profile_image = gtk_image_new_from_icon_name("user-info-symbolic", GTK_ICON_SIZE_MENU);
        gtk_button_set_image(GTK_BUTTON(profile_button), GTK_WIDGET(profile_image));
        g_signal_connect(profile_button, "clicked", G_CALLBACK(on_profile_button), userData);
        gtk_widget_set_margin_end(profile_button, 0);
        gtk_grid_attach(GTK_GRID(grid_top), profile_button, 3, 0, 1, 1);


        GtkWidget *label_emergency_list = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label_emergency_list), "<span font_desc='17' weight='bold'>Liste des urgences :</span>");
        gtk_widget_set_margin_top(label_emergency_list, 15);
        gtk_widget_set_margin_bottom(label_emergency_list, 10);
        gtk_grid_attach(GTK_GRID(grid_main), label_emergency_list, 0, 1, 1, 1);
        display_emergency_messages(grid_main, db, id_utilisateur, 2);

        GtkWidget *label_patient_list = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label_patient_list), "<span font_desc='17' weight='bold'>Liste des patients :</span>");
        gtk_widget_set_margin_top(label_patient_list, 15);
        gtk_widget_set_margin_bottom(label_patient_list, 10);
        gtk_grid_attach(GTK_GRID(grid_main), label_patient_list, 0, 3, 1, 1);
        patient_treeview = patient_list(db, id_utilisateur);
        gtk_grid_attach(GTK_GRID(grid_main), patient_treeview, 0, 4, 1, 1);

        GtkWidget *label_rv_list = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label_rv_list), "<span font_desc='17' weight='bold'>Liste des rendez-vous :</span>");
        gtk_widget_set_margin_top(label_rv_list, 15);
        gtk_widget_set_margin_bottom(label_rv_list, 10);
        gtk_grid_attach(GTK_GRID(grid_main), label_rv_list, 0, 5, 1, 1);
        appointment_treeview = rv_list(db, id_utilisateur);
        gtk_grid_attach(GTK_GRID(grid_main), appointment_treeview, 0, 6, 1, 1);


        PQclear(result);
        gtk_widget_show_all(window);
    } else {
        g_error("Erreur lors de la récupération des données du médecin : %s\n", PQerrorMessage(db));
    }
}