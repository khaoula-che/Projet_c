/*
    SeniorCare
    Created by : MOUGNI Asma
    Description : Interface patient dans laquelle se fait la prise de rendez-vous, la consultation des rendez-vous, et bien d'autres fonctionnalitées.
*/
// En-têtes système
#include <stdio.h>
#include <unistd.h>

// En-têtes GTK, Glib, CURL, gettext, JSON-GLib
#include <gtk/gtk.h>
#include <glib.h>
#include <curl/curl.h>
#include <libintl.h>
#include <json-glib/json-glib.h>
#include <pango/pango.h>

// En-tête personnalisé
#include "bdd.h"

// Directive de traduction pour la localisation
#define _(String) gettext(String)


GtkWidget *treeview;
GtkWidget *profile_window;
GtkWidget *entry_nom;
GtkWidget *entry_prenom;
GtkWidget *entry_date_de_naissance;
GtkWidget *entry_city;
GtkWidget *entry_numero_telephone;
GtkWidget *entry_password;
static GtkWidget *name_entry;
static GtkWidget *file_chooser;
static GtkWidget *file_window;
static GtkWidget *status_label;
static GtkWidget *widget;
static int year, month, day;


typedef struct {
    PGconn *db;
    const char *email;
    const char *nom;
    const char *city;
    const char *prenom;
    const char *date_de_naissance;
    const char *telephone;
    const char *mot_de_passe;
    int id_patient;
    int is_doctor;
    GtkWidget *heure_entry;
} UserData;

typedef struct {
    GtkWidget *window;
    GtkWidget *date_entry;
    GtkWidget *hours_spin;
    GtkWidget *heure_entry;
    GtkWidget *minutes_spin;
    GtkWidget *calendar;
    GtkWidget *lieu_entry;
    GtkWidget *description_entry;
    GtkWidget *medecin_combobox;
    PGresult *medecin_result;

    UserData *user_data;
} RendezVousFormData;

typedef struct {
    UserData *user_data;
    int rendezvous_id;
    gchar *date;
    int id_medecin;
} DeleteButtonData;




void delete_rendezvous(GtkButton *button, gpointer user_data);
static int global_rendezvous_id = -1;
void displayFormForRendezVous(GtkButton *button, UserData *user_data);
void submitRendezVousForm(GtkButton *button, RendezVousFormData *form_data);
void modify_profile(UserData *userData);
void update_profile_in_database(UserData *userData);
static void on_save_button_clicked(GtkButton *button, gpointer user_data);
static void on_modify_button_clicked(GtkButton *button, gpointer user_data);
static void on_profile_button_clicked(GtkButton *button, gpointer user_data);
void patient(PGconn *db, const char *email, GtkListStore *list_store);
void rendezvous(PGconn *db, int id_patient, GtkListStore *rv_store);
void column(GtkWidget *view, const char *title, int column_id);
GtkWidget *view(GtkListStore *list_store, const char *column_titles[], int num_columns);
GtkWidget *list(PGconn *db, int id_utilisateur);
void display_patient_page(PGconn *db, const char *email);
void updateWeatherIcon(GtkWidget *weatherDisplay);
static void delete_file(GtkButton *button, gpointer userData);
gboolean isTimeWithinRange(GtkWidget *hours_spin, GtkWidget *minutes_spin);
gboolean hasExistingAppointment(GtkComboBoxText *medecin_combobox, PGresult *medecin_result, const gchar *selected_date_str, const char *heure_minutes, PGconn *connexion, int id_medecin);
const char *getSelectedDoctorID(PGresult *medecin_result, const gchar *selected_doctor_name);
static void on_submit_button_clicked(GtkButton *button, gpointer user_data);
static void display_documents(GtkButton *button, gpointer user_data);
static void display_add_document_form(GtkButton *button, gpointer user_data);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
static void on_delete_account_button_clicked(GtkButton *button, gpointer user_data);
void insertEmergency(int id_patient, int id_medecin);
void display_emergency_contact_info();
int getPatientID(UserData *user_data);

GtkWidget *create_treeview() {
    GtkWidget *treeview = gtk_tree_view_new();
    return treeview;
}

GtkWidget *create_message() {
    GtkWidget *message = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(message), FALSE);
    return message;
}

static void window_close_patient(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void message_box_patient() {

}






void on_emergency_button_clicked(GtkButton *button, const char *email) {
    // Déclaration de la requête SQL pour récupérer l'ID de l'utilisateur et du médecin associé
    const char *sql = "SELECT id_utilisateur, id_medecin FROM Utilisateur WHERE email = $1";

    // Connexion à la base de données PostgreSQL
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    // Vérification de l'état de la connexion à la base de données
    if (PQstatus(conn) != CONNECTION_OK) {
        g_print("Database connection error: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // Exécution de la requête SQL avec la valeur de l'e-mail en tant que paramètre
    const char *values[1] = {email};
    PGresult *result = PQexecParams(conn, sql, 1, NULL, values, NULL, NULL, 0);

    // Vérification du statut de la requête SQL
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        g_print("Query execution error: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return;
    }

    // Extraction de l'ID du patient et du médecin à partir du résultat
    int id_patient = atoi(PQgetvalue(result, 0, 0));
    int id_medecin = atoi(PQgetvalue(result, 0, 1));

    // Nettoyage des ressources (libération de la mémoire)
    PQclear(result);
    PQfinish(conn);

    // Appel de la fonction d'insertion pour enregistrer l'urgence dans la base de données
    insertEmergency(id_patient, id_medecin);
}

void insertEmergency(int id_patient, int id_medecin) {
    // Affichage d'un message pour indiquer le début de l'insertion d'un enregistrement d'urgence
    g_print("Inserting Emergency Record:\n");

    // Connexion à la base de données PostgreSQL
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    // Déclaration de la requête SQL pour l'insertion d'un enregistrement d'urgence
    const char *query = "INSERT INTO Urgence (id_patient, id_medecin) VALUES ($1, $2)";

    // Conversion des valeurs d'identifiants en chaînes de caractères
    const char *values[2] = {g_strdup_printf("%d", id_patient), g_strdup_printf("%d", id_medecin)};

    // Exécution de la requête SQL d'insertion
    PGresult *insertResult = PQexecParams(conn, query, 2, NULL, values, NULL, NULL, 0);

    // Vérification du statut de la requête d'insertion
    if (PQresultStatus(insertResult) != PGRES_COMMAND_OK) {
        // En cas d'erreur, affichage du message d'erreur
        g_print("Query execution error: %s\n", PQerrorMessage(conn));
    } else {
        // Affichage du message de réussite si l'insertion est réussie
        g_print("Emergency Record Inserted Successfully!\n");
    }

    // Libération des ressources (mémoire) allouées pour la requête d'insertion
    PQclear(insertResult);

    // Fermeture de la connexion à la base de données
    PQfinish(conn);

    // Appel de la fonction pour afficher les informations de contact d'urgence
    display_emergency_contact_info(); // Vous pouvez choisir d'appeler cette fonction ici ou à un endroit approprié
}


void display_emergency_contact_info() {
    // Récupération des informations de contact d'urgence depuis la base de données
    // Remplacez ceci par votre logique de requête de base de données réelle
    const char *emergency_contact_number = "123-456-7890"; // Remplacez par le numéro réel

    // Création d'une boîte de dialogue avec les informations de contact d'urgence
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Vous avez signalé une urgence. Cela sera immédiatement notifié à votre médecin traitant.\nAppelez au plus vite ce numéro :\n%s",
        emergency_contact_number
    );

    // Exécution du dialogue et attente de la réponse de l'utilisateur
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Destruction de la boîte de dialogue pour libérer la mémoire
    gtk_widget_destroy(dialog);
}



void displayWeatherMessage(const char *weatherCode) {
    // Création d'une étiquette GTK pour afficher le message météo
    GtkWidget *messageLabel = gtk_label_new(NULL);

    // Sélection du message en fonction du code météo
    if (strcmp(weatherCode, "01d") == 0) { // Ciel dégagé (jour)
        gtk_label_set_text(GTK_LABEL(messageLabel), _("☀️ C'est une journée ensoleillée ! Profitez du temps et hydratez-vous régulièrement."));
    } else if (strcmp(weatherCode, "10d") == 0 || strcmp(weatherCode, "10n") == 0 || strcmp(weatherCode, "09d") == 0) { // Pluie
        gtk_label_set_text(GTK_LABEL(messageLabel), _("🌧️ Il pleut ! N'oubliez pas votre parapluie."));
    } else if (strcmp(weatherCode, "01n") == 0) { // Ciel dégagé (nuit)
        gtk_label_set_text(GTK_LABEL(messageLabel), _("🌙 Ciel dégagé la nuit. Profitez de la vue !"));
    } else if (strcmp(weatherCode, "13d") == 0) { // Neige
        gtk_label_set_text(GTK_LABEL(messageLabel), _("❄️ Il neige ! Restez au chaud et faites attention en cas de sorties."));
    } else if (strcmp(weatherCode, "50d") == 0) { // Brume ou brouillard
        gtk_label_set_text(GTK_LABEL(messageLabel), _("🌁 C'est brumeux. Conduisez prudemment !"));
    } else if (strcmp(weatherCode, "50n") == 0) { // Brume ou brouillard (nuit)
        gtk_label_set_text(GTK_LABEL(messageLabel), _("🌁 C'est brumeux et il fait nuit. Restez vigilant !"));
    } else {
        gtk_label_set_text(GTK_LABEL(messageLabel), _("🌦️ Découvrez la météo !"));
    }

    // Configuration de la taille de la police pour l'étiquette
    PangoFontDescription *fontDesc = pango_font_description_new();
    pango_font_description_set_size(fontDesc, 20 * PANGO_SCALE);  // Ajustez la taille de la police selon vos besoins
    gtk_widget_override_font(messageLabel, fontDesc);
    pango_font_description_free(fontDesc);

    // Création d'une fenêtre GTK pour afficher le message météo
    GtkWidget *messageWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(messageWindow), "Weather Message");
    gtk_window_set_default_size(GTK_WINDOW(messageWindow), 300, 100);

    // Création d'une grille GTK pour organiser les éléments de la fenêtre
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(messageWindow), grid);

    // Ajout de l'étiquette à la grille
    gtk_grid_attach(GTK_GRID(grid), messageLabel, 0, 0, 1, 1);

    // Affichage de tous les éléments de la fenêtre
    gtk_widget_show_all(messageWindow);
}






size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Récupération du widget GTKImage depuis le pointeur utilisateur
    GtkWidget *weatherImage = GTK_WIDGET(userp);

    // Conversion des données téléchargées en chaîne JSON
    const char *jsonString = (const char *)contents;
    JsonParser *parser = json_parser_new();
    GError *error = NULL;
    json_parser_load_from_data(parser, jsonString, strlen(jsonString), &error);

    // Vérification des erreurs lors de l'analyse JSON
    if (error != NULL) {
        g_warning("Error parsing JSON: %s", error->message);
        g_error_free(error);
        return 0;
    }

    // Extraction de l'objet racine JSON
    JsonObject *rootObject = json_node_get_object(json_parser_get_root(parser));

    // Extraction du code d'icône météo à partir du JSON
    const char *iconCode = NULL;

    if (json_object_has_member(rootObject, "weather")) {
        JsonArray *weatherArray = json_object_get_array_member(rootObject, "weather");

        if (json_array_get_length(weatherArray) > 0) {
            JsonObject *firstWeatherEntry = json_array_get_object_element(weatherArray, 0);

            if (json_object_has_member(firstWeatherEntry, "icon")) {
                iconCode = json_object_get_string_member(firstWeatherEntry, "icon");

                // Affichage du message météo en fonction du code d'icône
                displayWeatherMessage(iconCode);
            } else {
                g_warning("Missing 'icon' key in the first 'weather' entry.");
            }
        } else {
            g_warning("Empty 'weather' array.");
        }
    } else {
        g_warning("Missing 'weather' key in the JSON response.");
    }

    // Téléchargement de l'icône météo depuis une URL et enregistrement en local
    if (iconCode != NULL) {
        char iconUrl[256];
        snprintf(iconUrl, sizeof(iconUrl), "http://openweathermap.org/img/w/%s.png", iconCode);

        g_print("Constructed Icon URL: %s\n", iconUrl);

        // Définition du chemin local pour sauvegarder l'icône
        const char *localImagePath = "weather_icon.png";
        FILE *localImageFile = fopen(localImagePath, "wb");

        // Téléchargement de l'icône avec cURL
        if (localImageFile) {
            CURL *curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, iconUrl);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, localImageFile);

                CURLcode res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    g_warning("Failed to download image from URL: %s", curl_easy_strerror(res));
                }

                curl_easy_cleanup(curl);
            } else {
                g_warning("Failed to initialize CURL for downloading image.");
            }

            fclose(localImageFile);
        } else {
            g_warning("Failed to open local image file for writing.");
        }

        // Vérification de l'existence du fichier local de l'icône
        if (access(localImagePath, F_OK) != -1) {
            // Chargement de l'icône à partir du fichier local avec GdkPixbuf
            GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(localImagePath, NULL);
            if (pixbuf != NULL) {
                // Redimensionnement de l'icône à une taille spécifiée
                GdkPixbuf *resizedPixbuf = gdk_pixbuf_scale_simple(pixbuf, 100, 100, GDK_INTERP_BILINEAR);

                // Affichage de l'icône dans le widget GTKImage
                gtk_image_set_from_pixbuf(GTK_IMAGE(weatherImage), resizedPixbuf);

                // Libération de la mémoire associée au GdkPixbuf
                g_object_unref(pixbuf);
                g_object_unref(resizedPixbuf);
            } else {
                g_warning("Failed to load Pixbuf from file: %s", localImagePath);
            }
        } else {
            g_warning("Local image file is not accessible: %s", localImagePath);
        }
    } else {
        g_warning("Unable to extract 'icon' code from JSON.");
    }

    // Libération de la mémoire associée au parseur JSON
    g_object_unref(parser);

    // Retourne le nombre total d'octets traités
    return size * nmemb;
}



void makeOpenWeatherMapRequest(const char *apiKey, const char *city, GtkWidget *weatherImage) {
    CURL *curl;
    CURLcode res;

    // Initialisation globale de cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialisation de l'objet cURL
    curl = curl_easy_init();
    if (curl) {
        // Construction de l'URL pour la requête OpenWeatherMap
        char url[256];
        snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", city, apiKey);

        // Configuration des options de la requête cURL
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, weatherImage);

        // Exécution de la requête cURL
        res = curl_easy_perform(curl);

        // Vérification des erreurs lors de l'exécution de la requête cURL
        if (res != CURLE_OK) {
            g_warning("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Libération des ressources de l'objet cURL
        curl_easy_cleanup(curl);
    }

    // Nettoyage global de cURL
    curl_global_cleanup();
}





void modify_profile(UserData *userData) {
    // Vérification de la validité des données de l'utilisateur et de la connexion à la base de données
    if (userData == NULL || userData->db == NULL) {
        g_print("Error: Null or invalid database connection\n");
        return;
    }

    // Création d'une fenêtre GTK pour modifier le profil
    GtkWidget *modify_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(modify_window), "Modifier le profil");
    gtk_window_set_default_size(GTK_WINDOW(modify_window), 400, 400);

    // Création d'une grille GTK pour organiser les éléments de la fenêtre
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(modify_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    // Création des widgets d'entrée et d'étiquette pour les champs de modification
    entry_nom = gtk_entry_new();
    entry_prenom = gtk_entry_new();
    entry_date_de_naissance = gtk_calendar_new();
    entry_numero_telephone = gtk_entry_new();
    entry_password = gtk_entry_new();
    entry_city = gtk_entry_new();

    GtkWidget *label_nom = gtk_label_new("Nouveau nom:");
    GtkWidget *label_prenom = gtk_label_new("Nouveau prénom:");
    GtkWidget *label_date_de_naissance = gtk_label_new("Nouvelle date de naissance:");
    GtkWidget *label_numero_telephone = gtk_label_new("Nouveau numéro de téléphone:");
    GtkWidget *label_ville = gtk_label_new("Ville:");
    GtkWidget *label_password = gtk_label_new("Nouveau mot de passe:");

    GtkWidget *save_button = gtk_button_new_with_label("Enregistrer");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_button_clicked), userData);

    // Attachement des widgets à la grille
    gtk_grid_attach(GTK_GRID(grid), label_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_nom, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_prenom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_prenom, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_date_de_naissance, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_date_de_naissance, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_numero_telephone, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_numero_telephone, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_ville, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_city, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), save_button, 0, 6, 2, 1);

    // Remplissage des champs d'entrée avec les données actuelles de l'utilisateur
    gtk_entry_set_text(GTK_ENTRY(entry_nom), userData->nom);
    gtk_entry_set_text(GTK_ENTRY(entry_prenom), userData->prenom);

    // Conversion de la chaîne de date_de_naissance en entiers pour le calendrier
    int year, month, day;
    sscanf(userData->date_de_naissance, "%d-%d-%d", &year, &month, &day);
    gtk_calendar_select_month(GTK_CALENDAR(entry_date_de_naissance), month - 1, year);
    gtk_calendar_select_day(GTK_CALENDAR(entry_date_de_naissance), day);

    gtk_entry_set_text(GTK_ENTRY(entry_numero_telephone), userData->telephone);

    // Affichage de la fenêtre
    gtk_widget_show_all(modify_window);
}

static void on_save_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Saving modified profile...\n");

    // Vérification de la validité des champs d'entrée
    if (entry_nom == NULL || entry_prenom == NULL || entry_date_de_naissance == NULL ||
        entry_numero_telephone == NULL || entry_password == NULL) {
        g_print("Error: One or more entry fields are NULL\n");
        return;
    }

    // Récupération des nouvelles valeurs des champs d'entrée
    const char *new_nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    const char *new_prenom = gtk_entry_get_text(GTK_ENTRY(entry_prenom));

    gchar *selected_date_str;
    gtk_calendar_get_date(GTK_CALENDAR(entry_date_de_naissance), &year, &month, &day);
    selected_date_str = g_strdup_printf("%d-%02d-%02d", year, month + 1, day);

    const char *new_date_de_naissance = selected_date_str;
    const char *new_numero_telephone = gtk_entry_get_text(GTK_ENTRY(entry_numero_telephone));
    const char *new_city = gtk_entry_get_text(GTK_ENTRY(entry_city));
    const char *new_password = gtk_entry_get_text(GTK_ENTRY(entry_password));

    // Mise à jour des données utilisateur avec les nouvelles valeurs
    UserData *userData = (UserData *)user_data;
    userData->nom = new_nom;
    userData->prenom = new_prenom;
    userData->date_de_naissance = new_date_de_naissance;
    userData->telephone = new_numero_telephone;
    userData->city = new_city;
    userData->mot_de_passe = new_password;

    // Appel de la fonction pour sauvegarder les modifications dans la base de données
    g_print("Going straight to the saving function...\n");
    update_profile_in_database(userData);

    // Libération de la mémoire allouée pour la chaîne de date
    g_free(selected_date_str);

    // Destruction du widget bouton pour fermer la fenêtre de modification
    gtk_widget_destroy(GTK_WIDGET(button));
}

void update_profile_in_database(UserData *userData) {
    g_print("In the saving function...\n");

    // Connexion à la base de données PostgreSQL
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);

    // Vérification de l'état de la connexion
    if (PQstatus(db) != CONNECTION_OK) {
        g_print("Error: Database connection is not OK: %s\n", PQerrorMessage(db));
        PQfinish(db);
        return;
    }

    // Récupération des nouvelles valeurs des champs
    const char *email = userData->email;
    const char *new_nom = userData->nom;
    const char *new_prenom = userData->prenom;
    const char *new_date_de_naissance = userData->date_de_naissance;
    const char *new_numero_telephone = userData->telephone;
    const char *new_city = userData->city;
    const char *new_mot_de_passe = userData->mot_de_passe;

    // Construction de la requête de mise à jour
    const char *update_query = "UPDATE Utilisateur SET nom = $1, prenom = $2, date_de_naissance = $3, telephone = $4, city = $5, mot_de_passe = $6 WHERE email = $7";
    const char *update_values[7] = {new_nom, new_prenom, new_date_de_naissance, new_numero_telephone, new_city, new_mot_de_passe, email};

    // Exécution de la requête de mise à jour
    PGresult *update_result = PQexecParams(db, update_query, 7, NULL, update_values, NULL, NULL, 0);

    // Vérification du résultat de la requête
    if (PQresultStatus(update_result) != PGRES_COMMAND_OK) {
        g_print("Error updating profile: %s\n", PQerrorMessage(db));
    } else {
        g_print("Profile updated successfully\n");
    }

    // Libération des ressources
    PQclear(update_result);
    PQfinish(db);
}



static void on_modify_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Redirecting to the modification page...\n");

    modify_profile((UserData *)user_data);
}

static void on_profile_button_clicked(GtkButton *button, gpointer user_data) {
    UserData *userData = (UserData *)user_data;

    // Vérification de la validité des données utilisateur
    if (userData == NULL || userData->db == NULL) {
        g_print("Error: Null or invalid database connection\n");
        return;
    }

    // Connexion à la base de données PostgreSQL
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);

    // Vérification de l'état de la connexion
    if (PQstatus(db) != CONNECTION_OK) {
        g_print("Error: Database connection is not OK: %s\n", PQerrorMessage(db));
        PQfinish(db);
        return;
    }

    // Récupération des données du profil depuis la base de données
    const char *email = userData->email;
    const char *query = "SELECT nom, prenom, telephone, city, date_de_naissance FROM Utilisateur WHERE email = $1";
    const char *params[1] = {email};
    PGresult *result = PQexecParams(db, query, 1, NULL, params, NULL, NULL, 0);

    // Extraction des données du résultat
    const char *nom = PQgetvalue(result, 0, PQfnumber(result, "nom"));
    const char *prenom = PQgetvalue(result, 0, PQfnumber(result, "prenom"));
    const char *telephone = PQgetvalue(result, 0, PQfnumber(result, "telephone"));
    const char *city = PQgetvalue(result, 0, PQfnumber(result, "city"));
    const char *date_de_naissance = PQgetvalue(result, 0, PQfnumber(result, "date_de_naissance"));

    // Création et affichage de la fenêtre de profil
    GtkWidget *profile_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(profile_window), "Profil");
    gtk_window_set_default_size(GTK_WINDOW(profile_window), 500, 500);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(profile_window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    GtkWidget *label_nom = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Nom:</span> %s", nom));
    GtkWidget *label_prenom = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Prénom:</span> %s", prenom));
    GtkWidget *label_date_de_naissance = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Date de naissance:</span> %s", date_de_naissance));
    GtkWidget *label_email = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Email:</span> %s", email));
    GtkWidget *label_telephone = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Téléphone:</span> %s", telephone));
    GtkWidget *label_ville = gtk_label_new(g_strdup_printf("<span weight='bold' font_size='larger'>Ville:</span> %s", city));

    gtk_label_set_use_markup(GTK_LABEL(label_nom), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_prenom), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_date_de_naissance), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_email), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_telephone), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_ville), TRUE);

    gtk_grid_attach(GTK_GRID(grid), label_email, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_nom, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_prenom, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_date_de_naissance, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_telephone, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_ville, 0, 5, 1, 1);

    GtkWidget *modify_button = gtk_button_new_with_label("Modifier");
    g_signal_connect(modify_button, "clicked", G_CALLBACK(on_modify_button_clicked), user_data);
    gtk_grid_attach(GTK_GRID(grid), modify_button, 0, 6, 1, 1);

    GtkWidget *emergency_button = gtk_button_new_with_label("Signaler une urgence");
    g_signal_connect(emergency_button, "clicked", G_CALLBACK(on_emergency_button_clicked), email);
    gtk_grid_attach(GTK_GRID(grid), emergency_button, 0, 7, 1, 1);

    GtkWidget *delete_account_button = gtk_button_new_with_label("Supprimer mon compte");
    g_signal_connect(delete_account_button, "clicked", G_CALLBACK(on_delete_account_button_clicked), userData);
    gtk_grid_attach(GTK_GRID(grid), delete_account_button, 0, 8, 1, 1);


    PQclear(result);

    gtk_widget_show_all(profile_window);
    PQfinish(db);
}


static void on_delete_account_button_clicked(GtkButton *button, gpointer user_data) {
    UserData *userData = (UserData *)user_data;

    // S'assurer que userData n'est pas NULL et que la connexion à la base de données est valide
    if (userData == NULL || userData->db == NULL) {
        g_print("Erreur : Connexion à la base de données NULL ou invalide\n");
        return;
    }

    // Récupérer l'e-mail de l'utilisateur (userData->email est déjà utilisé dans la fonction)
    const char *email = userData->email;

    // Effectuer les actions pour supprimer le compte utilisateur
    const char *delete_query = "DELETE FROM Utilisateur WHERE email = $1";
    const char *params[1] = {email};
    PGresult *delete_result = PQexecParams(userData->db, delete_query, 1, NULL, params, NULL, NULL, 0);

    // Vérifier le statut du résultat de l'exécution de la requête DELETE
    if (PQresultStatus(delete_result) != PGRES_COMMAND_OK) {
        g_print("Erreur lors de l'exécution de la requête DELETE : %s\n", PQresultErrorMessage(delete_result));
        PQclear(delete_result);
        return;
    }

    // Libérer le résultat et terminer la connexion à la base de données
    PQclear(delete_result);
    PQfinish(userData->db);

}





static void delete_file(GtkButton *button, gpointer user_data) {
    UserData *data = (UserData *)user_data;

    // Récupération de l'ID du fichier à partir des données du bouton
    int file_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "file_id"));

    // Connexion à la base de données PostgreSQL
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connexion à la base de données échouée: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // Construction de la requête de suppression du fichier
    char query[512];
    snprintf(query, sizeof(query), "DELETE FROM files WHERE id = %d", file_id);

    // Exécution de la requête de suppression
    PGresult *result = PQexec(conn, query);

    // Vérification du résultat de la requête
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Échec de la suppression du fichier: %s\n", PQerrorMessage(conn));
    } else {
        printf("Fichier supprimé avec succès. ID: %d\n", file_id);
    }

    // Libération des ressources
    PQclear(result);
    PQfinish(conn);
}

static void display_documents(GtkButton *button, gpointer user_data) {
    UserData *data = (UserData *)user_data;

    // Création de la fenêtre principale
    GtkWidget *documents_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(documents_window), "Mes Documents");
    gtk_window_set_default_size(GTK_WINDOW(documents_window), 600, 400);

    // Création de la boîte principale
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Création de l'étiquette de la liste des documents
    GtkWidget *label = gtk_label_new("Liste des Documents :");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    // Création de la liste des documents
    GtkWidget *listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_NONE);
    gtk_box_pack_start(GTK_BOX(box), listbox, TRUE, TRUE, 0);

    // Connexion à la base de données PostgreSQL
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connexion à la base de données échouée: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    // Construction de la requête pour récupérer les documents de l'utilisateur
    char query[512];
    snprintf(query, sizeof(query), "SELECT * FROM files WHERE id_patient = (SELECT id_utilisateur FROM Utilisateur WHERE email = '%s')", data->email);

    // Exécution de la requête
    PGresult *result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Échec de l'exécution de la requête: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return;
    }

    // Traitement des résultats de la requête
    if (PQntuples(result) > 0) {
        for (int i = 0; i < PQntuples(result); i++) {
            int file_id = atoi(PQgetvalue(result, i, 0));
            const char *file_name = PQgetvalue(result, i, 1);

            // Création de la boîte horizontale pour chaque document
            GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

            // Création de l'étiquette du nom du fichier
            GtkWidget *file_label = gtk_label_new(file_name);
            gtk_box_pack_start(GTK_BOX(hbox), file_label, TRUE, TRUE, 0);

            // Création du bouton de suppression
            GtkWidget *delete_button = gtk_button_new_with_label("Delete");
            g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_file), data);
            g_object_set_data(G_OBJECT(delete_button), "file_id", GINT_TO_POINTER(file_id));
            gtk_box_pack_start(GTK_BOX(hbox), delete_button, FALSE, FALSE, 0);

            // Ajout de la boîte horizontale à la liste des documents
            gtk_list_box_insert(GTK_LIST_BOX(listbox), hbox, -1);
        }
    } else {
        // Aucun fichier trouvé
        GtkWidget *no_files_label = gtk_label_new("Aucun fichier trouvé.");
        gtk_box_pack_start(GTK_BOX(box), no_files_label, FALSE, FALSE, 0);
    }

    // Création du bouton pour ajouter un document
    GtkWidget *add_document_button = gtk_button_new_with_label("Ajouter un document");
    g_signal_connect(add_document_button, "clicked", G_CALLBACK(display_add_document_form), user_data);
    gtk_box_pack_start(GTK_BOX(box), add_document_button, FALSE, FALSE, 0);

    // Libération des ressources de la base de données
    PQclear(result);
    PQfinish(conn);

    // Ajout de la boîte principale à la fenêtre
    gtk_container_add(GTK_CONTAINER(documents_window), box);

    // Affichage de la fenêtre
    gtk_widget_show_all(documents_window);
}


static void on_submit_button_clicked(GtkButton *button, gpointer user_data) {
    if (user_data == NULL) {
        printf("Error: user_data is NULL\n");
        return;
    }

    UserData *userData = (UserData *)user_data;

    if (userData->db == NULL || userData->email == NULL) {
        g_print("Error: Null or invalid database connection or email\n");
        return;
    }

    const gchar *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));

    // Création des objets GFile pour le fichier source et le dossier de destination
    GFile *source_file = g_file_new_for_path(filename);
    GFile *destination_folder = g_file_new_for_path("/docs");

    // Création du fichier de destination avec le nom du fichier d'origine et l'extension .jpg
    GFile *destination_file = g_file_get_child(destination_folder, g_strconcat(name, ".jpg", NULL));

    // Récupération du chemin complet de sauvegarde
    gchar *save_path = g_file_get_path(destination_file);

    g_print("File will be saved to: %s\n", save_path);

    // Vérification de l'existence du fichier source
    gboolean source_exists = g_file_query_exists(source_file, NULL);

    g_print("Checking filename and save_path before g_file_copy:\n");
    g_print("filename: %s\n", filename);
    g_print("save_path: %s\n", save_path);

    GError *error = NULL;
    gboolean success = g_file_copy(source_file, destination_file, G_FILE_COPY_NONE, NULL, NULL, NULL, &error);

    if (success) {
        printf("File uploaded successfully\n");
        gtk_label_set_text(GTK_LABEL(status_label), "File uploaded successfully!");

        // Connexion à la base de données PostgreSQL
        PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

        if (PQstatus(conn) != CONNECTION_OK) {
            printf("Connection to the database failed\n");
            PQfinish(conn);
            return;
        }

        // Récupération de l'ID de l'utilisateur à partir de l'e-mail
        char query[512];
        snprintf(query, sizeof(query), "SELECT id_utilisateur FROM Utilisateur WHERE email = '%s'", userData->email);
        PGresult *result = PQexec(conn, query);

        if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
            userData->id_patient = atoi(PQgetvalue(result, 0, 0));

            // Insertion des informations sur le fichier dans la base de données
            snprintf(query, sizeof(query), "INSERT INTO files (file_name, id_patient) VALUES ('%s', %d)",
                     g_path_get_basename(save_path), userData->id_patient);

            PQclear(result);
            result = PQexec(conn, query);

            if (PQresultStatus(result) != PGRES_COMMAND_OK) {
                g_print("Error: Failed to insert file information into the database.\n");
            } else {
                g_print("File information inserted into the database successfully.\n");
            }
        } else {
            g_print("Error: Failed to retrieve id_patient from the database.\n");
        }

        if (status_label != NULL) {
            gtk_label_set_text(GTK_LABEL(status_label), "Error uploading file.");
        }

        PQclear(result);
        PQfinish(conn);
    } else {
        // En cas d'échec du téléchargement
        if (status_label != NULL) {
            gtk_label_set_text(GTK_LABEL(status_label), "Error uploading file.");
        }
    }

    // Libération des ressources
    g_object_unref(source_file);
    g_object_unref(destination_folder);
    g_object_unref(destination_file);

    g_free(name);
    g_free(filename);
    g_free(save_path);
}



static void display_add_document_form(GtkButton *button, gpointer user_data) {
    g_print("In display_add_document_form...\n");

    // Création de la fenêtre principale
    GtkWidget *file_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(file_window), "File Upload Form");
    gtk_container_set_border_width(GTK_CONTAINER(file_window), 10);
    gtk_widget_set_size_request(file_window, 300, 150);

    // Création de la grille pour organiser les éléments du formulaire
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(file_window), grid);

    // Création d'une étiquette et d'une zone de texte pour le nom du fichier
    GtkWidget *name_label = gtk_label_new("Name:");
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);

    // Création d'une étiquette et d'un sélecteur de fichiers pour le choix du fichier
    GtkWidget *file_label = gtk_label_new("File:");
    gtk_grid_attach(GTK_GRID(grid), file_label, 0, 1, 1, 1);

    GtkWidget *file_chooser = gtk_file_chooser_button_new("Choose File", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser), g_get_home_dir());
    gtk_grid_attach(GTK_GRID(grid), file_chooser, 1, 1, 1, 1);

    // Création du bouton de soumission
    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_button_clicked), user_data);
    gtk_grid_attach(GTK_GRID(grid), submit_button, 0, 2, 2, 1);

    // Affichage de la fenêtre
    gtk_widget_show_all(file_window);
}


static void display_rendezvous(GtkButton *button, gpointer user_data) {
    printf("Display Rendez-vous button clicked\n");

    UserData *data = (UserData *)user_data;

    GtkWidget *rendezvous_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(rendezvous_window), "Mes Rendez-vous");
    gtk_window_set_default_size(GTK_WINDOW(rendezvous_window), 600, 400);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *label = gtk_label_new("Liste des Rendez-vous :");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);


    GtkWidget *listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_NONE);
    gtk_box_pack_start(GTK_BOX(box), listbox, TRUE, TRUE, 0);

    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connexion à la base de données échouée: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT date, heure, lieu, description, id_medecin FROM rendez_vous WHERE id_patient = (SELECT id_utilisateur FROM Utilisateur WHERE email = '%s')", data->email);
    PGresult *result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Échec de l'exécution de la requête: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return;
    }

    if (PQntuples(result) > 0) {
        for (int i = 0; i < PQntuples(result); i++) {
            int rendezvous_id = atoi(PQgetvalue(result, i, 0));
            const char *date = PQgetvalue(result, i, 0);
            const char *heure = PQgetvalue(result, i, 1);
            const char *lieu = PQgetvalue(result, i, 2);
            const char *description = PQgetvalue(result, i, 3);
            int id_medecin = atoi(PQgetvalue(result, i, 4));

            // Query to get the nom of the medecin
            snprintf(query, sizeof(query), "SELECT nom FROM Utilisateur WHERE id_utilisateur = %d", id_medecin);
            PGresult *medecin_result = PQexec(conn, query);
            const char *nom_medecin = (PQresultStatus(medecin_result) == PGRES_TUPLES_OK && PQntuples(medecin_result) > 0) ? PQgetvalue(medecin_result, 0, 0) : "Nom inconnu";

            GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

            GtkWidget *date_label = gtk_label_new(date);
            GtkWidget *heure_label = gtk_label_new(heure);
            GtkWidget *lieu_label = gtk_label_new(lieu);
            GtkWidget *description_label = gtk_label_new(description);
            GtkWidget *medecin_label = gtk_label_new(nom_medecin);

            gtk_box_pack_start(GTK_BOX(hbox), date_label, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), heure_label, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), lieu_label, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), description_label, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), medecin_label, TRUE, TRUE, 0);

            // Create Delete button with rendezvous_id, date, and id_medecin data
            GtkWidget *delete_button = gtk_button_new_with_label("Delete");
            DeleteButtonData *button_data = g_new(DeleteButtonData, 1);
            button_data->user_data = data;
            button_data->rendezvous_id = rendezvous_id;
            button_data->date = g_strdup(date);
            button_data->id_medecin = id_medecin;

            // Set rendezvous_id, date, and id_medecin as properties of the delete button
            g_object_set_data(G_OBJECT(delete_button), "rendezvous_id", GINT_TO_POINTER(rendezvous_id));
            g_object_set_data(G_OBJECT(delete_button), "date", g_strdup(date));
            g_object_set_data(G_OBJECT(delete_button), "id_medecin", GINT_TO_POINTER(id_medecin));

            g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_rendezvous), button_data);

            gtk_box_pack_start(GTK_BOX(hbox), delete_button, FALSE, FALSE, 0);

            gtk_list_box_insert(GTK_LIST_BOX(listbox), hbox, -1);

            PQclear(medecin_result);
        }
    } else {
        GtkWidget *no_rendezvous_label = gtk_label_new("Aucun rendez-vous trouvé.");
        gtk_box_pack_start(GTK_BOX(box), no_rendezvous_label, FALSE, FALSE, 0);
    }
    GtkWidget *prendre_rdv_button = gtk_button_new_with_label("Prendre RDV");
    g_signal_connect(prendre_rdv_button, "clicked", G_CALLBACK(displayFormForRendezVous), user_data);
    gtk_box_pack_start(GTK_BOX(box), prendre_rdv_button, FALSE, FALSE, 0);

    PQclear(result);
    PQfinish(conn);

    gtk_container_add(GTK_CONTAINER(rendezvous_window), box);

    gtk_widget_show_all(rendezvous_window);
}




void delete_rendezvous(GtkButton *button, gpointer user_data) {
    UserData *data = (UserData *)user_data;

    // Recuperation des données
    int rendezvous_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "rendezvous_id"));
    const char *date = g_object_get_data(G_OBJECT(button), "date");
    int id_medecin = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "id_medecin"));

    printf("Deleting rendezvous with ID: %d, Date: %s, id_medecin: %d\n", rendezvous_id, date, id_medecin);

    // Connexion à la base de donnée
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

	if (PQstatus(conn) != CONNECTION_OK) {
	    fprintf(stderr, "Connexion à la base de données échouée: %s\n", PQerrorMessage(conn));
	    PQfinish(conn);
	    return;
	}

	char query[512];
	printf("before sql");

	snprintf(query, sizeof(query), "DELETE FROM rendez_vous WHERE id_medecin = %d AND date = '%s'", id_medecin, date);
	PGresult *result = PQexec(conn, query);
	printf("after sql");

	if (PQresultStatus(result) != PGRES_COMMAND_OK) {
	    fprintf(stderr, "Échec de la suppression du rendez-vous: %s\n", PQerrorMessage(conn));
	} else {
	    printf("Rendez-vous supprimé avec succès!\n");
	}

	PQclear(result);
	PQfinish(conn);

}


// Function to create and show the form for taking an appointment
void displayFormForRendezVous(GtkButton *button, UserData *user_data) {

    UserData *user_datas = g_new(UserData, 1);
    user_datas->db = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");


    if (PQstatus(user_datas->db) != CONNECTION_OK) {
        g_print("Error connecting to the database: %s\n", PQerrorMessage(user_datas->db));
        PQfinish(user_datas->db);
        g_free(user_datas);
        return;
    }


    if (user_datas->db == NULL || PQstatus(user_datas->db) != CONNECTION_OK) {
        g_print("Error: Database connection is not OK\n");

        g_free(user_datas);
        return;
    }


    const char *query = "SELECT id_utilisateur, nom FROM Utilisateur WHERE type='Medecin';";
    g_print("Executing query: %s\n", query);


    PGresult *result = PQexecParams(user_datas->db, query, 0, NULL, NULL, NULL, NULL, 0);


    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        g_print("Error executing the query: %s\n", PQresultErrorMessage(result));
        PQclear(result);
        PQfinish(user_datas->db);
        g_free(user_datas);
        return;
    }


    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Prendre RDV");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);


    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);


    GtkWidget *calendar = gtk_calendar_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Date:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 0, 1, 1);

    GtkWidget *date_entry = gtk_entry_new();

    GtkWidget *time_label = gtk_label_new("Heure:");
    gtk_grid_attach(GTK_GRID(grid), time_label, 0, 1, 1, 1);

    GtkWidget *time_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    GtkWidget *hours_spin = gtk_spin_button_new_with_range(0, 23, 1);
    GtkWidget *minutes_spin = gtk_spin_button_new_with_range(0, 59, 1);

    gtk_box_pack_start(GTK_BOX(time_box), hours_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(time_box), gtk_label_new(":"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(time_box), minutes_spin, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(grid), time_box, 1, 1, 1, 1);

    GtkWidget *lieu_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Lieu:"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lieu_entry, 1, 3, 1, 1);

    GtkWidget *description_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Description:"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), description_entry, 1, 4, 1, 1);


    GtkWidget *medecin_combobox = gtk_combo_box_text_new();
    int num_rows = PQntuples(result);
    for (int i = 0; i < num_rows; ++i) {
        const char *medecin_id = PQgetvalue(result, i, 0);
        const char *medecin_nom = PQgetvalue(result, i, 1);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(medecin_combobox), medecin_nom);
    }
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Médecin:"), 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), medecin_combobox, 1, 5, 1, 1);


    GtkWidget *submit_button = gtk_button_new_with_label("Valider");


    RendezVousFormData *form_data = g_new(RendezVousFormData, 1);

    form_data->user_data = user_data;
    form_data->window = window;
    form_data->date_entry = date_entry;
    form_data->heure_entry = hours_spin;
    form_data->lieu_entry = lieu_entry;
    form_data->calendar = calendar;
    form_data->description_entry = description_entry;
    form_data->medecin_combobox = medecin_combobox;
    form_data->hours_spin = hours_spin;
    form_data->minutes_spin = minutes_spin;
    form_data->medecin_result = result;


    g_signal_connect(submit_button, "clicked", G_CALLBACK(submitRendezVousForm), form_data);
    gtk_grid_attach(GTK_GRID(grid), submit_button, 1, 6, 1, 1);


    gtk_widget_show_all(window);
}



void submitRendezVousForm(GtkButton *button, RendezVousFormData *form_data) {
    GDate selected_date;
    int year, month, day;


    PGconn *connexion = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty client_encoding=UTF8");

    // Vérifier l'état de la connexion
    if (PQstatus(connexion) != CONNECTION_OK) {
        g_print("Error connecting to the database: %s\n", PQerrorMessage(connexion));
        PQfinish(connexion);
        return;
    }

    // Obtenir la date sélectionnée du widget de calendrier GTK
    gtk_calendar_get_date(GTK_CALENDAR(form_data->calendar), &year, &month, &day);


    selected_date.year = year;
    selected_date.month = month + 1;
    selected_date.day = day;

GDate current_date;
g_date_set_time_t(&current_date, time(NULL));

// Compare selected date with the current date
if (g_date_compare(&selected_date, &current_date) < 0) {
    g_print("Error: La date sélectionnée est passée.\n");
    PQfinish(connexion);
    return;
}

    // Vérifier si l'heure sélectionnée est dans la plage autorisée (8:00 - 18:00)
	if (!isTimeWithinRange(form_data->hours_spin, form_data->minutes_spin)) {
	    g_print("Error: Le médecin ne travail qu'à ces horaires (8:00 - 18:00).\n");

	    PQfinish(connexion);
	    return;
	}

    // Convertir la date sélectionnée en une chaîne de caractères
    char selected_date_str[40];
    g_snprintf(selected_date_str, sizeof(selected_date_str), "%d-%02d-%02d",
               selected_date.year, selected_date.month, selected_date.day);

    // Récuperer les valeurs du formulaire
    const char *lieu = gtk_entry_get_text(GTK_ENTRY(form_data->lieu_entry));
    const char *description = gtk_entry_get_text(GTK_ENTRY(form_data->description_entry));
    const char *heure = gtk_entry_get_text(GTK_ENTRY(form_data->hours_spin));
    const char *minutes = gtk_entry_get_text(GTK_ENTRY(form_data->minutes_spin));
    const char *selected_doctor_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(form_data->medecin_combobox));

    // Chercher l'ID du medecin
    const char *id_utilisateur = getSelectedDoctorID(form_data->medecin_result, selected_doctor_name);
    int id_medecin = atoi(id_utilisateur);

    // Chercher l'ID du patient
    int id_patient = getPatientID(form_data->user_data);

    // Verification des rendez-vous
    char heure_minutes[6];
    snprintf(heure_minutes, sizeof(heure_minutes), "%s:%s", heure, minutes);

    if (hasExistingAppointment(form_data->medecin_combobox, form_data->medecin_result, selected_date_str, heure_minutes, connexion, id_medecin)) {
        g_print("Error: Le médecin à déja un rendez-vous à l'heure selectionnée .\n");
        PQfinish(connexion);
        return;
    }

    // Insertion du rendez-vous
    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO rendez_vous (date, heure, lieu, description, id_patient, id_medecin) VALUES ('%s', '%s', '%s', '%s', %d, %d)",selected_date_str, heure_minutes, lieu, description, id_patient, id_medecin);
    PGresult *result = PQexec(connexion, query);


    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        g_print("Error executing INSERT query: %s\n", PQresultErrorMessage(result));
    }

    // Clean up
    PQclear(result);
    PQfinish(connexion);
    gtk_widget_destroy(form_data->window);
}


const char *getSelectedDoctorID(PGresult *medecin_result, const char *selected_doctor_name) {
    int num_rows = PQntuples(medecin_result);

    for (int i = 0; i < num_rows; ++i) {
        const char *medecin_nom = PQgetvalue(medecin_result, i, 1);

        if (strcmp(medecin_nom, selected_doctor_name) == 0) {
            const char *id_utilisateur = PQgetvalue(medecin_result, i, 0);
            return id_utilisateur;
        }
    }
    return NULL;
}


gboolean isTimeWithinRange(GtkWidget *hours_spin, GtkWidget *minutes_spin) {
    int selected_hours = atoi(gtk_entry_get_text(GTK_ENTRY(hours_spin)));
    int selected_minutes = atoi(gtk_entry_get_text(GTK_ENTRY(minutes_spin)));

    return (selected_hours >= 8 && selected_hours < 18 && selected_minutes >= 0 && selected_minutes < 60);
}



// Verifier qu'un rendez-vous n'existe pas déja
gboolean hasExistingAppointment(GtkComboBoxText *medecin_combobox, PGresult *medecin_result, const gchar *selected_date_str, const char *heure_minutes, PGconn *connexion, int id_medecin) {
    char check_query[256];
            g_print("query: %s\n", check_query);

    snprintf(check_query, sizeof(check_query), "SELECT * FROM rendez_vous WHERE id_medecin = %d AND date = '%s' AND heure = '%s'",
             id_medecin, selected_date_str, heure_minutes);

    PGresult *check_result = PQexec(connexion, check_query);

    if (PQresultStatus(check_result) != PGRES_TUPLES_OK) {
        g_print("Error executing appointment check query: %s\n", PQresultErrorMessage(check_result));
        PQclear(check_result);
        PQfinish(connexion);
        return TRUE;
    }

    gboolean has_appointment = (PQntuples(check_result) > 0);

    PQclear(check_result);

    return has_appointment;
}



int getPatientID(UserData *user_data) {
    if (user_data == NULL || user_data->email == NULL) {
        g_print("Error: user_data or user_data->email is NULL\n");
        return -1;
    }

    const char *email = user_data->email;
    g_print("User's email: %s\n", email);

    PGconn *con = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    if (PQstatus(con) != CONNECTION_OK) {
        g_print("Error connecting to the database: %s\n", PQerrorMessage(con));
        PQfinish(con);
        return -1;
    }

	PGresult *result = PQexecParams(con, "SELECT id_utilisateur FROM Utilisateur WHERE email = $1;", 1, NULL, &email, &(int){strlen(email)}, &(int){0}, 0);


    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
        g_print("Error fetching patient ID: %s\n", PQresultErrorMessage(result));
        PQclear(result);
        PQfinish(con);
        return -1;
    }

    const char *id_patient_str = PQgetvalue(result, 0, 0);
    int id_patient = atoi(id_patient_str);
        g_print("%d",id_patient);

    PQclear(result);
    PQfinish(con);

    return id_patient;
}

static void display_medicaments(GtkButton *button, gpointer user_data) {
    UserData *data = (UserData *)user_data;

    // Création d'une fenetre
    GtkWidget *medicaments_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(medicaments_window), "Mes Médicaments");
    gtk_window_set_default_size(GTK_WINDOW(medicaments_window), 600, 400);

    // Création boite verticale
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *label = gtk_label_new("Liste des Médicaments :");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    // Fenetre adapté a la taille de la fenetre si contenue trop long
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *treeview = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);

    // Création d'une liste pour afficher les données
    GtkListStore *list_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(list_store));

    // Ajout de column
    for (int i = 0; i < 3; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Column", renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    }

    // Recherche des Médicaments
    const char *query = "SELECT nom, description, dosage FROM medicament WHERE id_patient = (SELECT id_utilisateur FROM Utilisateur WHERE email = $1)";
    const char *values[1] = {data->email};

    PGconn *conne = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");
    if (PQstatus(conne) != CONNECTION_OK) {
        fprintf(stderr, "Connection to the database failed: %s\n", PQerrorMessage(conne));
        PQfinish(conne);

        return;
    }

    PGresult *result = PQexecParams(conne, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK) {

        for (int i = 0; i < PQntuples(result); i++) {
            const char *nom_medicament = PQgetvalue(result, i, 0);
            const char *description = PQgetvalue(result, i, 1);
            const char *dosage = PQgetvalue(result, i, 2);


            GtkTreeIter iter;
            gtk_list_store_append(list_store, &iter);
            gtk_list_store_set(list_store, &iter, 0, nom_medicament, 1, description, 2, dosage, -1);
        }
    } else {
        // Affichage d'un message si aucun médicaments n'est trouver
        GtkWidget *no_medicaments_label = gtk_label_new("Aucun médicament trouvé.");
        gtk_box_pack_start(GTK_BOX(box), no_medicaments_label, FALSE, FALSE, 0);
    }


    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(medicaments_window), box);

    // Display de toutes les fenetres
    gtk_widget_show_all(medicaments_window);

    // Clear le resullt et la db
    PQclear(result);
    PQfinish(conne);

}



gboolean check_database(int id_utilisateur) {
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    const char *query = "SELECT COUNT(*) FROM rappel WHERE id_patient = $1";
    const char *values[1] = {g_strdup_printf("%d", id_utilisateur)};

    PGresult *result = PQexecParams(conn, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        g_print("Query execution error: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return FALSE;
    }

    int count = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);

    PQfinish(conn);

    return count > 0;
}


void display_rappel_records(GtkButton *button, gpointer user_data) {
    int id_utilisateur = GPOINTER_TO_INT(user_data);
    PGconn *conn = PQconnectdb("host=localhost dbname=Projet_c user=postgres password=azerty");

    if (PQstatus(conn) != CONNECTION_OK) {
        g_print("Database connection error: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    const char *query = "SELECT id_rappel, rendez_vous, conseil, meteo, message FROM rappel WHERE id_patient = $1";
    const char *values[1] = {g_strdup_printf("%d", id_utilisateur)};

    PGresult *result = PQexecParams(conn, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        g_print("Query execution error: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return;
    }

    int num_rows = PQntuples(result);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Rappel Records");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);


    GtkWidget *label_rendez_vous = gtk_label_new("<b>Rendez-vous</b>");
    GtkWidget *label_conseil = gtk_label_new("<b>Conseil</b>");
    GtkWidget *label_meteo = gtk_label_new("<b>Meteo</b>");
    GtkWidget *label_message = gtk_label_new("<b>Message</b>");


    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    gtk_label_set_use_markup(GTK_LABEL(label_rendez_vous), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_conseil), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_meteo), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(label_message), TRUE);


    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
    pango_attr_list_insert(attr_list, underline_attr);

    gtk_label_set_attributes(GTK_LABEL(label_rendez_vous), attr_list);
    gtk_label_set_attributes(GTK_LABEL(label_conseil), attr_list);
    gtk_label_set_attributes(GTK_LABEL(label_meteo), attr_list);
    gtk_label_set_attributes(GTK_LABEL(label_message), attr_list);

    gtk_grid_attach(GTK_GRID(grid), label_rendez_vous, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_conseil, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_meteo, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_message, 3, 0, 1, 1);


    for (int i = 0; i < num_rows; ++i) {
        const char *rendez_vous = PQgetvalue(result, i, 1);
        const char *conseil = PQgetvalue(result, i, 2);
        const char *meteo = PQgetvalue(result, i, 3);
        const char *message = PQgetvalue(result, i, 4);

        GtkWidget *label_rendez_vous_value = gtk_label_new(rendez_vous);
        GtkWidget *label_conseil_value = gtk_label_new(conseil);
        GtkWidget *label_meteo_value = gtk_label_new(meteo);
        GtkWidget *label_message_value = gtk_label_new(message);

        gtk_label_set_ellipsize(GTK_LABEL(label_rendez_vous_value), PANGO_ELLIPSIZE_END);
        gtk_label_set_ellipsize(GTK_LABEL(label_conseil_value), PANGO_ELLIPSIZE_END);
        gtk_label_set_ellipsize(GTK_LABEL(label_meteo_value), PANGO_ELLIPSIZE_END);
        gtk_label_set_ellipsize(GTK_LABEL(label_message_value), PANGO_ELLIPSIZE_END);

        gtk_grid_attach(GTK_GRID(grid), label_rendez_vous_value, 0, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), label_conseil_value, 1, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), label_meteo_value, 2, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), label_message_value, 3, i + 1, 1, 1);
    }

    gtk_widget_show_all(window);

    PQclear(result);
    PQfinish(conn);
}









void display_patient_page(PGconn *db, const char *email) {
    const char *query = "SELECT nom, prenom, date_de_naissance, telephone, city, id_utilisateur   FROM utilisateur WHERE email = $1 AND type = 'Patient'";
    const char *values[1] = {email};
    PGresult *result = PQexecParams(db, query, 1, NULL, values, NULL, NULL, 0);

    if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
        const char *nom = PQgetvalue(result, 0, 0);
        const char *prenom = PQgetvalue(result, 0, 1);
        const char *date_de_naissance = PQgetvalue(result, 0, 2);
        const char *numero_telephone = PQgetvalue(result, 0, 3);
        const char *ville = PQgetvalue(result, 0, 4);
        int id_utilisateur = atoi(PQgetvalue(result, 0, 5));



        const char *nom_affichage = (nom != NULL) ? nom : "Nom inconnu";
        const char *prenom_affichage = (prenom != NULL) ? prenom : "Prénom inconnu";
        const char *date_naissance_affichage = (date_de_naissance != NULL) ? date_de_naissance : "Date de naissance inconnue";
        const char *telephone_affichage = (numero_telephone != NULL) ? numero_telephone : "Numéro de téléphone inconnu";
                const char *ville_affichage = (ville != NULL) ? ville : "Ville inconnu";

        char *nom_utf8 = g_locale_to_utf8(nom_affichage, -1, NULL, NULL, NULL);
        char *prenom_utf8 = g_locale_to_utf8(prenom_affichage, -1, NULL, NULL, NULL);
        char *date_naissance_utf8 = g_locale_to_utf8(date_naissance_affichage, -1, NULL, NULL, NULL);
        char *telephone_utf8 = g_locale_to_utf8(telephone_affichage, -1, NULL, NULL, NULL);


        char *welcome_message = g_strdup_printf("Bienvenue sur l'espace Patient, %s %s", nom_utf8, prenom_utf8);

        UserData *userData = g_new(UserData, 1);
        userData->db = db;
        userData->email = email;
        userData->nom = nom;
        userData->prenom = prenom;
        userData->date_de_naissance = date_de_naissance;
        userData->telephone = numero_telephone;
        userData->city = ville;


        GtkWidget *window;
        GtkWidget *grid;
        GtkWidget *weatherImage = gtk_image_new();
        const char *apiKey = "f1f87f6fd454d3ae28c30950e3c3db6d";
        const char *city = ville;

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Espace Patient");
        gtk_window_set_default_size(GTK_WINDOW(window), 1200, 1000);
        g_signal_connect(window, "destroy", G_CALLBACK(window_close_patient), NULL);

        grid = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(window), grid);

        GtkWidget *label_welcome = gtk_label_new(welcome_message);
        gtk_label_set_markup(GTK_LABEL(label_welcome), g_strdup_printf("<span font_desc='Arial 20'>Bienvenue sur l'espace Patient, %s %s!</span>", nom_utf8, prenom_utf8));
        gtk_grid_attach(GTK_GRID(grid), label_welcome, 0, 0, 2, 1);
        g_free(welcome_message);
        g_free(nom_utf8);
        g_free(prenom_utf8);

        GtkWidget *weatherDisplay = gtk_image_new();
        makeOpenWeatherMapRequest(apiKey, city, weatherDisplay);

        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_pack_start(GTK_BOX(vbox), weatherDisplay, TRUE, TRUE, 0);

        gtk_grid_attach(GTK_GRID(grid), vbox, 0, 6, 2, 1);

        GtkWidget *treeview = create_treeview();
        gtk_grid_attach(GTK_GRID(grid), treeview, 0, 1, 2, 1);
        gtk_widget_set_hexpand(treeview, TRUE);
        gtk_widget_set_vexpand(treeview, TRUE);

        message_box_patient();

        GtkWidget *message = create_message();
        gtk_grid_attach(GTK_GRID(grid), message, 0, 3, 2, 1);
        gtk_widget_set_hexpand(message, TRUE);
        gtk_widget_set_vexpand(message, TRUE);

        GtkWidget *profile_button = gtk_button_new_from_icon_name("user-info-symbolic", GTK_ICON_SIZE_DIALOG);
        g_object_set_data(G_OBJECT(profile_button), "user-db", db);
        g_signal_connect(profile_button, "clicked", G_CALLBACK(on_profile_button_clicked), userData);
        gtk_grid_attach(GTK_GRID(grid), profile_button, 1, 0, 1, 1);
        gtk_widget_set_halign(profile_button, GTK_ALIGN_END);
        gtk_widget_set_valign(profile_button, GTK_ALIGN_START);




        gboolean has_rappel = check_database(id_utilisateur);

	if (has_rappel) {
	    GtkWidget *show_RAPPEL_button = gtk_button_new();
	    GtkWidget *image_rappel = gtk_image_new_from_icon_name("dialog-error", GTK_ICON_SIZE_BUTTON);
	    gtk_button_set_image(GTK_BUTTON(show_RAPPEL_button), image_rappel);
	    g_signal_connect(show_RAPPEL_button, "clicked", G_CALLBACK(display_rappel_records), GINT_TO_POINTER(id_utilisateur));
	    gtk_grid_attach(GTK_GRID(grid), show_RAPPEL_button, 1, 0, 1, 1);
	}

	GtkWidget *show_rendezvous_button = gtk_button_new_with_label("Show Rendez-vous");
        g_signal_connect(show_rendezvous_button, "clicked", G_CALLBACK(display_rendezvous), userData);
	gtk_grid_attach(GTK_GRID(grid), show_rendezvous_button, 0, 2, 1, 1);


	GtkWidget *medicaments_button = gtk_button_new_with_label("Mes Médicaments");
	g_signal_connect(medicaments_button, "clicked", G_CALLBACK(display_medicaments), userData);
	gtk_grid_attach(GTK_GRID(grid), medicaments_button, 1, 2, 1, 1);



	GtkWidget *docbutton = gtk_button_new();
	g_signal_connect(docbutton, "clicked", G_CALLBACK(display_documents), userData);

        GtkWidget *icon = gtk_image_new_from_icon_name("folder", GTK_ICON_SIZE_BUTTON);
        gtk_button_set_image(GTK_BUTTON(docbutton), icon);
        gtk_widget_set_size_request(docbutton, 30, 30);

        gtk_grid_attach(GTK_GRID(grid), docbutton, 0, 0, 1, 1);


        gtk_widget_show_all(window);

        g_free(nom_utf8);
        g_free(prenom_utf8);
        g_free(date_naissance_utf8);

        PQclear(result);
    }
}
