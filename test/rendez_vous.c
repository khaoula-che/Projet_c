
GtkWidget *notebook;  // Utiliser une variable globale pour le notebook

void switch_to_menu(GtkButton *button, gpointer data) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);  // 1 est l'index de la page du menu
}

void show_menu() {
    menu_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(menu_window), "SeniorCare");
    gtk_window_set_default_size(GTK_WINDOW(menu_window), 1000, 900);

    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(menu_window), notebook);

    // Page d'accueil
    GtkWidget *home_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *start_button = gtk_button_new_with_label("Démarrer");
    g_signal_connect(start_button, "clicked", G_CALLBACK(switch_to_menu), NULL);
    gtk_box_pack_start(GTK_BOX(home_page), start_button, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), home_page, gtk_label_new("Accueil"));

    // Page du menu
    GtkWidget *menu_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *login_button_menu = gtk_button_new_with_label("Connexion");
    g_signal_connect(login_button_menu, "clicked", G_CALLBACK(switch_to_login), NULL);
    gtk_box_pack_start(GTK_BOX(menu_page), login_button_menu, TRUE, TRUE, 0);
    GtkWidget *register_button_menu = gtk_button_new_with_label("Inscription");
    g_signal_connect(register_button_menu, "clicked", G_CALLBACK(switch_to_register), NULL);
    gtk_box_pack_start(GTK_BOX(menu_page), register_button_menu, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), menu_page, gtk_label_new("Menu"));

    gtk_widget_show_all(menu_window);
}

void show_home_page() {
    GtkWidget *home_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(home_window), "SeniorCare");
    gtk_window_set_default_size(GTK_WINDOW(home_window), 400, 200);
    g_signal_connect(home_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(home_window), vbox);

    GtkWidget *legend_label = gtk_label_new("Bienvenue à SeniorCare, votre assistant de soins personnalisé.");
    gtk_box_pack_start(GTK_BOX(vbox), legend_label, FALSE, FALSE, 0);

    GtkWidget *start_button = gtk_button_new_with_label("Démarrer");
    g_signal_connect(start_button, "clicked", G_CALLBACK(show_menu), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), start_button, FALSE, FALSE, 0);

    gtk_widget_show_all(home_window);
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "fr_FR.UTF-8");
    gtk_init(&argc, &argv);
    show_home_page();
    gtk_main();
    return 0;
}