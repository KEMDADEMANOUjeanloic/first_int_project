#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "enregistrement.h"

// ============= VARIABLES GLOBALES =============
client *reservations = NULL; // utilise la struct client de include/types.h
int nb_reservations = 0;
client *waiting_list = NULL;
int waiting_count = 0;
int selected_start = -1;
int selected_end = -1;

// Widgets principaux
GtkWidget *window;
GtkWidget *calendar_grid;
GtkWidget *status_label;
GtkWidget *stats_count_label;
GtkWidget *stats_rate_label;
GtkWidget *nom_entry;
GtkWidget *day_buttons[nombre_jours];
GtkWidget *list_view;
GtkWidget *progress_bar;
GtkWidget *waiting_grid; /* GtkGrid used to show waiting list as a table */
GtkWidget *entry_start_widget;
GtkWidget *entry_end_widget;

// ==================== DECLARATION CALLBACKS ====================
void on_day_clicked(GtkWidget *button, gpointer data);
void on_delete_clicked(GtkWidget *button, gpointer data);
void on_add_reservation_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void on_new_reservation_clicked(GtkWidget *button, gpointer data);
void on_reset_clicked(GtkWidget *button, gpointer data);
void on_view_stats_clicked(GtkWidget *button, gpointer data);
void on_cancel_clicked(GtkWidget *button, gpointer data);
void on_prolong_clicked(GtkWidget *button, gpointer data);
void on_delay_clicked(GtkWidget *button, gpointer data);
void on_help_clicked(GtkWidget *button, gpointer data);
static void new_conflict_response_cb(GtkDialog *dialog, int response, gpointer user_data);

// ==================== UTILITAIRES ====================
GdkRGBA generate_color(const char *nom) {
    unsigned int hash = 0;
    for (int i = 0; nom[i]; i++)
        hash = hash * 31 + nom[i];

    GdkRGBA color;
    color.red = 0.3 + (hash % 100) / 200.0;
    color.green = 0.4 + ((hash / 100) % 100) / 200.0;
    color.blue = 0.5 + ((hash / 10000) % 100) / 200.0;
    color.alpha = 1.0;
    return color;
}

/* Normalize a client name: trim whitespace and lowercase into dst (size dstlen) */
static void normalize_name(const char *src, char *dst, size_t dstlen) {
    if (!src || dstlen == 0) { if (dstlen) dst[0] = '\0'; return; }
    const char *s = src;
    while (*s && g_ascii_isspace((gchar)*s)) s++;
    const char *e = src + strlen(src) - 1;
    while (e >= s && g_ascii_isspace((gchar)*e)) e--;
    size_t n = (e >= s) ? (size_t)(e - s + 1) : 0;
    size_t copy = (n < dstlen-1) ? n : dstlen-1;
    for (size_t i = 0; i < copy; i++) dst[i] = g_ascii_tolower((gchar)s[i]);
    dst[copy] = '\0';
}

/* Case-insensitive, trimmed comparison for client names */
static int name_equal(const char *a, const char *b) {
    char na[STAT_LEN]; char nb[STAT_LEN];
    normalize_name(a, na, sizeof(na));
    normalize_name(b, nb, sizeof(nb));
    return strcmp(na, nb) == 0;
}

int check_conflict(int d1, int d2) {
    for (int i = 0; i < nb_reservations; i++) {
        if (!(reservations[i].date_fin < d1 || d2 < reservations[i].date_debut))
            return 1;
    }
    return 0;
}

const char* get_occupant(int day) {
    for (int i = 0; i < nb_reservations; i++) {
        if (day >= reservations[i].date_debut && day <= reservations[i].date_fin)
            return reservations[i].nom_client;
    }
    return NULL;
}

void get_day_color(int day, GdkRGBA *color) {
    const char *name = get_occupant(day);
    if (name) {
        *color = generate_color(name);
        return;
    }
    gdk_rgba_parse(color, "#4CAF50");
}

float calculate_occupancy() {
    int occupied = 0;
    for (int d = 1; d <= nombre_jours; d++)
        if (get_occupant(d) != NULL)
            occupied++;
    return (float)occupied / nombre_jours;
}

// ==================== MISE A JOUR CALENDRIER ====================
void update_calendar() {
    for (int i = 0; i < nombre_jours; i++) {
        int day = i + 1;
        const char *occupant = get_occupant(day);

        /* Use Pango markup so the occupant name is bold; safely escape the text */
        GtkWidget *child = gtk_widget_get_first_child(day_buttons[i]);
        if (occupant) {
            gchar *markup = g_markup_printf_escaped("Jour %d\n<b>%s</b>", day, occupant);
            if (child && GTK_IS_LABEL(child)) gtk_label_set_markup(GTK_LABEL(child), markup);
            else {
                GtkWidget *lbl = gtk_label_new(NULL);
                gtk_label_set_markup(GTK_LABEL(lbl), markup);
                gtk_button_set_child(GTK_BUTTON(day_buttons[i]), lbl);
            }
            g_free(markup);
        } else {
            gchar *markup = g_markup_printf_escaped("Jour %d\nLibre", day);
            if (child && GTK_IS_LABEL(child)) gtk_label_set_markup(GTK_LABEL(child), markup);
            else {
                GtkWidget *lbl = gtk_label_new(NULL);
                gtk_label_set_markup(GTK_LABEL(lbl), markup);
                gtk_button_set_child(GTK_BUTTON(day_buttons[i]), lbl);
            }
            g_free(markup);
        }

        GdkRGBA color;
        get_day_color(day, &color);

        GtkStyleContext *context = gtk_widget_get_style_context(day_buttons[i]);
        gtk_style_context_remove_class(context, "selected");
        gtk_style_context_remove_class(context, "occupied");
        gtk_style_context_remove_class(context, "free");

        if ((selected_start != -1 && i >= selected_start && 
             (selected_end == -1 ? i == selected_start : i <= selected_end))) {
            gtk_style_context_add_class(context, "selected");
        } else if (occupant) {
            gtk_style_context_add_class(context, "occupied");
        } else {
            gtk_style_context_add_class(context, "free");
        }
    }

    char count_str[100];
    snprintf(count_str, sizeof(count_str), "📊 <b>%d</b> réservations", nb_reservations);
    gtk_label_set_markup(GTK_LABEL(stats_count_label), count_str);
    char rate_str[100];
    snprintf(rate_str, sizeof(rate_str), "📅 Taux d'occupation: <b>%.1f%%</b>", calculate_occupancy() * 100);
    gtk_label_set_markup(GTK_LABEL(stats_rate_label), rate_str);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), calculate_occupancy());
}

// ==================== MISE A JOUR LISTE ====================
void update_list_view() {
    GtkWidget *child = gtk_widget_get_first_child(list_view);
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(list_view), child);
        child = next;
    }

    if (nb_reservations == 0) {
        GtkWidget *empty_label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(empty_label), "<span size='large'>📋 Aucune réservation</span>");
        gtk_box_append(GTK_BOX(list_view), empty_label);
        return;
    }

    for (int i = 0; i < nb_reservations; i++) {
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_widget_set_margin_start(box, 10);
        gtk_widget_set_margin_end(box, 10);
        gtk_widget_set_margin_top(box, 5);
        gtk_widget_set_margin_bottom(box, 5);
        char info[200];
        int duree = reservations[i].date_fin - reservations[i].date_debut + 1;
        snprintf(info, 200, "<b>%s</b>\n📅 Jours %d → %d (%d nuits)",
                 reservations[i].nom_client,
                 reservations[i].date_debut,
                 reservations[i].date_fin,
                 duree);

        GtkWidget *label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label), info);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0);
        gtk_widget_set_hexpand(label, TRUE);

        GtkWidget *delete_btn = gtk_button_new_from_icon_name("user-trash-symbolic");
        gtk_style_context_add_class(gtk_widget_get_style_context(delete_btn), "destructive-action");
        g_object_set_data(G_OBJECT(delete_btn), "reservation_index", GINT_TO_POINTER(i));
        g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_clicked), NULL);
        gtk_box_append(GTK_BOX(box), label);
        gtk_box_append(GTK_BOX(box), delete_btn);

        gtk_box_append(GTK_BOX(list_view), box);
    }
}

void update_waiting_view() {
    /* Cleanup: remove any waiting entries that already have a reservation (same name)
       This prevents showing a client both on the calendar and in the waiting list. */
    if (waiting_list && waiting_count > 0) {
        int write = 0;
        for (int i = 0; i < waiting_count; i++) {
            int present_in_res = 0;
            for (int j = 0; j < nb_reservations; j++) {
                if (strcmp(waiting_list[i].nom_client, reservations[j].nom_client) == 0) { present_in_res = 1; break; }
            }
            if (!present_in_res) {
                if (write != i) waiting_list[write] = waiting_list[i];
                write++;
            }
        }
        if (write != waiting_count) {
            waiting_count = write;
            if (waiting_count == 0) { free(waiting_list); waiting_list = NULL; }
            else {
                client *tw = realloc(waiting_list, waiting_count * sizeof(client));
                if (tw) waiting_list = tw;
            }
        }
    }
    if (!waiting_grid) return;
    // Clear all children from grid (we simply recreate rows)
    GtkWidget *child = gtk_widget_get_first_child(waiting_grid);
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_widget_unparent(child);
        child = next;
    }

    // Header
    GtkWidget *h_nom = gtk_label_new("Nom");
    GtkWidget *h_start = gtk_label_new("Date début");
    GtkWidget *h_end = gtk_label_new("Date fin");
    GtkWidget *h_nuits = gtk_label_new("Nuits");
    gtk_widget_set_halign(h_nom, GTK_ALIGN_START);
    gtk_widget_set_halign(h_start, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(h_end, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(h_nuits, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(h_nom, "waiting-header");
    gtk_widget_add_css_class(h_start, "waiting-header");
    gtk_widget_add_css_class(h_end, "waiting-header");
    gtk_widget_add_css_class(h_nuits, "waiting-header");
    gtk_grid_attach(GTK_GRID(waiting_grid), h_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(waiting_grid), h_start, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(waiting_grid), h_end, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(waiting_grid), h_nuits, 3, 0, 1, 1);

    if (waiting_count == 0) {
        GtkWidget *empty = gtk_label_new("(vide)");
        gtk_widget_add_css_class(empty, "waiting-empty");
        gtk_grid_attach(GTK_GRID(waiting_grid), empty, 0, 1, 4, 1);
        return;
    }

    for (int i = 0; i < waiting_count; i++) {
    GtkWidget *n = gtk_label_new(waiting_list[i].nom_client);
        char buf_start[16]; char buf_end[16]; char buf_nuits[16];
        snprintf(buf_start, sizeof(buf_start), "%d", waiting_list[i].date_debut);
        snprintf(buf_end, sizeof(buf_end), "%d", waiting_list[i].date_fin);
        int duree = waiting_list[i].date_fin - waiting_list[i].date_debut + 1;
        snprintf(buf_nuits, sizeof(buf_nuits), "%d", duree);
    GtkWidget *s = gtk_label_new(buf_start);
    GtkWidget *e = gtk_label_new(buf_end);
    GtkWidget *nuits = gtk_label_new(buf_nuits);
    gtk_widget_set_halign(n, GTK_ALIGN_START);
    gtk_widget_set_halign(s, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(e, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(nuits, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(n, "waiting-cell");
    gtk_widget_add_css_class(s, "waiting-cell");
    gtk_widget_add_css_class(e, "waiting-cell");
    gtk_widget_add_css_class(nuits, "waiting-cell");
    gtk_widget_set_hexpand(n, TRUE);
    gtk_grid_attach(GTK_GRID(waiting_grid), n, 0, i+1, 1, 1);
    gtk_grid_attach(GTK_GRID(waiting_grid), s, 1, i+1, 1, 1);
    gtk_grid_attach(GTK_GRID(waiting_grid), e, 2, i+1, 1, 1);
    gtk_grid_attach(GTK_GRID(waiting_grid), nuits, 3, i+1, 1, 1);
    }
}

/* Try to promote the first waiting client that fits the current reservations */
void try_promote_from_waiting(void) {
    for (int k = 0; k < waiting_count; k++) {
        int conflit = 0;
        for (int j = 0; j < nb_reservations; j++) {
            if (!((reservations[j].date_fin < waiting_list[k].date_debut) || (waiting_list[k].date_fin < reservations[j].date_debut))) {
                conflit = 1;
                break;
            }
        }
        if (!conflit) {
            client *t = realloc(reservations, (nb_reservations + 1) * sizeof(client));
            if (!t) return; // out of memory, give up
            reservations = t;
            reservations[nb_reservations] = waiting_list[k];
            nb_reservations++;
            // remove from waiting list
            for (int l = k; l < waiting_count - 1; l++) waiting_list[l] = waiting_list[l+1];
            waiting_count--;
            if (waiting_count == 0) { free(waiting_list); waiting_list = NULL; }
            else {
                client *tw = realloc(waiting_list, waiting_count * sizeof(client));
                if (tw) waiting_list = tw;
            }
            update_calendar();
            update_list_view();
            update_waiting_view();
            return; // promote only one at a time
        }
    }
}

/* Remove any entries from waiting_list that match the given client name */
void remove_from_waiting_by_name(const char *name) {
    if (!waiting_list || !name) return;
    int write = 0;
    for (int i = 0; i < waiting_count; i++) {
        if (strcmp(waiting_list[i].nom_client, name) != 0) {
            if (write != i) waiting_list[write] = waiting_list[i];
            write++;
        }
    }
    if (write != waiting_count) {
        waiting_count = write;
        if (waiting_count == 0) { free(waiting_list); waiting_list = NULL; }
        else {
            client *tw = realloc(waiting_list, waiting_count * sizeof(client));
            if (tw) waiting_list = tw;
        }
        update_waiting_view();
    }
}

// ==================== CALLBACKS SIMPLES ====================
void on_day_clicked(GtkWidget *button, gpointer data) {
    int day = GPOINTER_TO_INT(data);
    if (selected_start == -1) { selected_start = day; selected_end = -1; }
    else if (selected_end == -1 && day >= selected_start) selected_end = day;
    else { selected_start = day; selected_end = -1; }
    update_calendar();
}

void on_delete_clicked(GtkWidget *button, gpointer data) {
    int index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "reservation_index"));
    if (index < 0 || index >= nb_reservations) return;
    // supprimer la réservation sélectionnée
    for (int i = index; i < nb_reservations - 1; i++) reservations[i] = reservations[i+1];
    nb_reservations--;
    client *t = realloc(reservations, (nb_reservations > 0 ? nb_reservations : 0) * sizeof(client));
    if (t || nb_reservations == 0) reservations = t;
    // après suppression, essayer de promouvoir un client de la file d'attente
    try_promote_from_waiting();
    update_calendar();
    update_list_view();
    update_waiting_view();
}

// ==================== CSS ====================
void apply_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "button.day-button { min-width: 90px; min-height:70px; }"
        "button.day-button.free { background:#4CAF50; }"
        "button.day-button.occupied { background:#F44336; }"
        "button.day-button.selected { background:#2196F3; }"
        ".titled-frame { border: 3px solid #666666; padding:8px; margin:6px; }"
        ".waiting-header { font-weight: bold; background: #eeeeee; border: 2px solid #999999; padding:6px; }"
        ".waiting-cell { border: 2px solid #dddddd; padding:6px; }"
        ".waiting-empty { font-style: italic; color: #666666; padding:6px; }"
    ".action-button { font-weight: bold; padding: 8px 12px; }"
    ".help-button { font-weight: bold; background: #eeeeff; padding: 4px 8px; }"
    ".help-circle { min-width: 34px; min-height: 34px; max-width: 34px; max-height: 34px; border-radius: 20px; background: #3b82f6; color: white; padding: 0; }"
    ".help-label { font-weight: bold; color: white; font-size: 14px; }";
    gtk_css_provider_load_from_data(provider, css, -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

// ==================== INTERFACE ====================
void activate(GtkApplication *app, gpointer user_data) {
    // Charger les réservations persistées au démarrage
    reservations = charger_reservations(&nb_reservations, FICHIER_RESERVATIONS);
    if (reservations == NULL) nb_reservations = 0;
    // Charger la file d'attente
    waiting_list = charger_reservations(&waiting_count, FICHIER_ATTENTE);
    if (waiting_list == NULL) waiting_count = 0;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🏨 Gestion Chambre d'Hôtel");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    /* Layout principal : 3 colonnes
       gauche : calendrier (+ liste des réservations sous le calendrier)
       centre : formulaire (haut) + statistiques (bas)
       droite : file d'attente (table)
    */
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_window_set_child(GTK_WINDOW(window), main_hbox);

    // ===== colonne gauche : calendrier + liste =====
    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_hexpand(left_vbox, TRUE);
    gtk_widget_set_vexpand(left_vbox, TRUE);
    gtk_box_append(GTK_BOX(main_hbox), left_vbox);

    calendar_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(calendar_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(calendar_grid), 5);

    for (int i = 0; i < nombre_jours; i++) {
        day_buttons[i] = gtk_button_new_with_label("");
        gtk_widget_add_css_class(day_buttons[i], "day-button");
        g_signal_connect(day_buttons[i], "clicked", G_CALLBACK(on_day_clicked), GINT_TO_POINTER(i));
        // placer en 3 colonnes et 10 lignes (col = i/10, row = i%10)
        int col = i / 10;
        int row = i % 10;
        gtk_grid_attach(GTK_GRID(calendar_grid), day_buttons[i], col, row, 1, 1);
    }

    GtkWidget *calendar_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(calendar_scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(calendar_scroll), calendar_grid);
    /* Allow calendar to grow/shrink with window */
    gtk_widget_set_hexpand(calendar_scroll, TRUE);
    gtk_widget_set_vexpand(calendar_scroll, TRUE);
    /* Put calendar inside a titled frame to clearly delimit the section */
    GtkWidget *calendar_frame = gtk_frame_new("Calendrier");
    gtk_frame_set_child(GTK_FRAME(calendar_frame), calendar_scroll);
    gtk_widget_set_hexpand(calendar_frame, TRUE);
    gtk_widget_set_vexpand(calendar_frame, TRUE);
    gtk_widget_add_css_class(calendar_frame, "titled-frame");
    gtk_box_append(GTK_BOX(left_vbox), calendar_frame);

    /* action_box moved to center above form (see below) */

    // the reservations frame will be placed under statistics in the center column

    // ===== colonne centre : formulaire + statistiques =====
    GtkWidget *center_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(center_vbox, TRUE);
    gtk_widget_set_vexpand(center_vbox, TRUE);
    gtk_widget_set_margin_top(center_vbox, 8);
    gtk_box_append(GTK_BOX(main_hbox), center_vbox);

    // Barre d'actions centrée au-dessus des champs (Réserver / Annuler / Retard / Prolonger)
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(action_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_bottom(action_box, 8);
    GtkWidget *btn_reserver = gtk_button_new_with_label("Réserver");
    GtkWidget *btn_annuler = gtk_button_new_with_label("Annuler");
    GtkWidget *btn_retard = gtk_button_new_with_label("Retard");
    GtkWidget *btn_prolong = gtk_button_new_with_label("Prolonger");
    /* small circular help button with '?' label */
    GtkWidget *btn_help = gtk_button_new();
    GtkWidget *help_lbl = gtk_label_new("?");
    gtk_widget_add_css_class(help_lbl, "help-label");
    gtk_button_set_child(GTK_BUTTON(btn_help), help_lbl);
    g_signal_connect(btn_reserver, "clicked", G_CALLBACK(on_new_reservation_clicked), NULL);
    g_signal_connect(btn_annuler, "clicked", G_CALLBACK(on_cancel_clicked), NULL);
    g_signal_connect(btn_retard, "clicked", G_CALLBACK(on_delay_clicked), NULL);
    g_signal_connect(btn_prolong, "clicked", G_CALLBACK(on_prolong_clicked), NULL);
    g_signal_connect(btn_help, "clicked", G_CALLBACK(on_help_clicked), NULL);
    /* style action buttons bold */
    gtk_widget_add_css_class(btn_reserver, "action-button");
    gtk_widget_add_css_class(btn_annuler, "action-button");
    gtk_widget_add_css_class(btn_retard, "action-button");
    gtk_widget_add_css_class(btn_prolong, "action-button");
    gtk_widget_add_css_class(btn_help, "help-circle");
    gtk_box_append(GTK_BOX(action_box), btn_reserver);
    gtk_box_append(GTK_BOX(action_box), btn_annuler);
    gtk_box_append(GTK_BOX(action_box), btn_retard);
    gtk_box_append(GTK_BOX(action_box), btn_prolong);
    gtk_box_append(GTK_BOX(action_box), btn_help);
    /* Wrap buttons in a frame for clear delimitation */
    GtkWidget *action_frame = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(action_frame), action_box);
    gtk_widget_set_hexpand(action_frame, TRUE);
    gtk_widget_add_css_class(action_frame, "titled-frame");
    gtk_box_append(GTK_BOX(center_vbox), action_frame);

    // formulaire (milieu haut)
    GtkWidget *form_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form_grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(form_grid), 6);
    GtkWidget *label_nom = gtk_label_new("Nom:");
    GtkWidget *entry_nom = gtk_entry_new();
    GtkWidget *label_start = gtk_label_new("Date début (1-30):");
    GtkWidget *entry_start = gtk_entry_new();
    GtkWidget *label_end = gtk_label_new("Date fin (1-30):");
    GtkWidget *entry_end = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(form_grid), label_nom, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), entry_nom, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_start, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), entry_start, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), label_end, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(form_grid), entry_end, 1, 2, 1, 1);
    /* expose the form entries to callbacks */
    nom_entry = entry_nom;
    entry_start_widget = entry_start;
    entry_end_widget = entry_end;

    /* Put the form inside a frame */
    GtkWidget *form_frame = gtk_frame_new("Formulaire");
    gtk_frame_set_child(GTK_FRAME(form_frame), form_grid);
    gtk_widget_set_hexpand(form_frame, TRUE);
    gtk_widget_add_css_class(form_frame, "titled-frame");
    gtk_box_append(GTK_BOX(center_vbox), form_frame);

    // Statistiques (milieu bas) - two stacked labels
    stats_count_label = gtk_label_new("📊 0 réservations");
    stats_rate_label = gtk_label_new("📅 Taux: 0%");
    progress_bar = gtk_progress_bar_new();
    gtk_widget_set_halign(stats_count_label, GTK_ALIGN_START);
    gtk_widget_set_halign(stats_rate_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(center_vbox), stats_count_label);
    gtk_box_append(GTK_BOX(center_vbox), stats_rate_label);
    gtk_box_append(GTK_BOX(center_vbox), progress_bar);

    /* Reservations frame moved here: under statistics in center column */
    list_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *list_frame = gtk_frame_new("Réservations");
    gtk_frame_set_child(GTK_FRAME(list_frame), list_view);
    gtk_widget_set_hexpand(list_frame, TRUE);
    gtk_widget_set_vexpand(list_frame, TRUE);
    gtk_widget_add_css_class(list_frame, "titled-frame");
    gtk_box_append(GTK_BOX(center_vbox), list_frame);

    // ===== colonne droite : file d'attente sous forme de tableau (scrolled + cadre) =====
    waiting_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(waiting_grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(waiting_grid), 12);
    GtkWidget *waiting_scroll = gtk_scrolled_window_new();
    /* Disable scrollbars so the table is visible without scrolling; container will expand */
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(waiting_scroll), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(waiting_scroll), waiting_grid);
    gtk_widget_set_hexpand(waiting_scroll, TRUE);
    gtk_widget_set_vexpand(waiting_scroll, TRUE);
    GtkWidget *wait_frame = gtk_frame_new("File d'attente");
    gtk_frame_set_child(GTK_FRAME(wait_frame), waiting_scroll);
    gtk_widget_set_hexpand(wait_frame, TRUE);
    gtk_widget_set_vexpand(wait_frame, TRUE);
    gtk_widget_add_css_class(wait_frame, "titled-frame");
    gtk_box_append(GTK_BOX(main_hbox), wait_frame);

    // ... action_box moved earlier above calendar ...

    apply_css();
    gtk_widget_show(window);

    /* Mettre à jour l'affichage après chargement des données */
    update_calendar();
    update_list_view();
    update_waiting_view();
}

/* -------------------- Dialog helpers & callbacks -------------------- */

static void show_message(GtkWindow *parent, const char *title, const char *message) {
    GtkWidget *d = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_window_set_title(GTK_WINDOW(d), title);
    g_signal_connect_swapped(d, "response", G_CALLBACK(gtk_window_destroy), d);
    gtk_window_present(GTK_WINDOW(d));
}

typedef struct { GtkWidget *dialog; GtkWidget *entry_nom; GtkWidget *entry_start; GtkWidget *entry_end; } NewResData;

typedef struct { GtkWidget *dialog; client tmp; } ConflictData;

static void new_res_response_cb(GtkDialog *dialog, int response, gpointer user_data) {
    NewResData *d = (NewResData*)user_data;
    if (response == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_nom)));
        const char *s_start = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_start)));
        const char *s_end = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_end)));
        int d1 = atoi(s_start);
        int d2 = atoi(s_end);
        if (nom == NULL || strlen(nom) == 0 || d1 < 1 || d2 < d1 || d2 > nombre_jours) {
            show_message(GTK_WINDOW(d->dialog), "Erreur", "Entrées invalides. Vérifiez le nom et les dates.");
        } else {
            client tmp;
            strncpy(tmp.nom_client, nom, STAT_LEN-1);
            tmp.nom_client[STAT_LEN-1] = '\0';
            tmp.date_debut = d1;
            tmp.date_fin = d2;
            /* Check conflict locally to avoid console prompts from fonction_reservation */
            if (check_conflict(tmp.date_debut, tmp.date_fin)) {
                /* Ask user to modify or go to waiting list */
                GtkWidget *conf = gtk_message_dialog_new(GTK_WINDOW(d->dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                    "Conflit : la période choisie est déjà occupée. Voulez-vous modifier les dates ou mettre en file d'attente ?");
                gtk_window_set_title(GTK_WINDOW(conf), "Conflit");
                gtk_dialog_add_button(GTK_DIALOG(conf), "Modifier", GTK_RESPONSE_CANCEL);
                gtk_dialog_add_button(GTK_DIALOG(conf), "Mettre en file d'attente", GTK_RESPONSE_YES);
                ConflictData *cd = g_malloc(sizeof(ConflictData));
                cd->dialog = conf; cd->tmp = tmp;
                g_signal_connect(conf, "response", G_CALLBACK(new_conflict_response_cb), cd);
                gtk_window_present(GTK_WINDOW(conf));
            } else {
                /* No conflict: add reservation directly */
                client *t = realloc(reservations, (nb_reservations + 1) * sizeof(client));
                if (!t) {
                    show_message(GTK_WINDOW(d->dialog), "Erreur", "Erreur mémoire lors de l'ajout de la réservation.");
                } else {
                    reservations = t;
                    reservations[nb_reservations] = tmp;
                    nb_reservations++;
                    update_calendar();
                    update_list_view();
                    /* Remove from waiting list if present */
                    remove_from_waiting_by_name(tmp.nom_client);
                    show_message(GTK_WINDOW(d->dialog), "Réservé", "Réservation ajoutée avec succès.");
                }
            }
        }
    }
    gtk_window_destroy(GTK_WINDOW(d->dialog));
    g_free(d);
}

void on_new_reservation_clicked(GtkWidget *button, gpointer data) {
    (void)button; (void)data;
    if (!nom_entry || !entry_start_widget || !entry_end_widget) {
        show_message(GTK_WINDOW(window), "Erreur", "Champs de formulaire non initialisés.");
        return;
    }
    const char *nom = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(nom_entry)));
    const char *s_start = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_start_widget)));
    const char *s_end = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_end_widget)));
    int d1 = atoi(s_start);
    int d2 = atoi(s_end);
    if (nom == NULL || strlen(nom) == 0 || d1 < 1 || d2 < d1 || d2 > nombre_jours) {
        show_message(GTK_WINDOW(window), "Erreur", "Entrées invalides. Vérifiez le nom et les dates.");
        return;
    }
    client tmp;
    strncpy(tmp.nom_client, nom, STAT_LEN-1);
    tmp.nom_client[STAT_LEN-1] = '\0';
    tmp.date_debut = d1;
    tmp.date_fin = d2;
    /* Avoid calling fonction_reservation (which may prompt on stdin). Check conflict here. */
    if (check_conflict(tmp.date_debut, tmp.date_fin)) {
        /* present dialog: modify or waiting list */
        GtkWidget *conf = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
            "Conflit : la période choisie est déjà occupée. Voulez-vous modifier les dates ou mettre en file d'attente ?");
        gtk_window_set_title(GTK_WINDOW(conf), "Conflit");
        gtk_dialog_add_button(GTK_DIALOG(conf), "Modifier", GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button(GTK_DIALOG(conf), "Mettre en file d'attente", GTK_RESPONSE_YES);
        ConflictData *cd = g_malloc(sizeof(ConflictData)); cd->dialog = conf; cd->tmp = tmp;
        g_signal_connect(conf, "response", G_CALLBACK(new_conflict_response_cb), cd);
        gtk_window_present(GTK_WINDOW(conf));
    } else {
        client *t = realloc(reservations, (nb_reservations + 1) * sizeof(client));
        if (!t) {
            show_message(GTK_WINDOW(window), "Erreur", "Erreur mémoire lors de l'ajout de la réservation.");
        } else {
            reservations = t;
            reservations[nb_reservations] = tmp;
            nb_reservations++;
            update_calendar();
            update_list_view();
            remove_from_waiting_by_name(tmp.nom_client);
            show_message(GTK_WINDOW(window), "Réservé", "Réservation ajoutée avec succès.");
        }
    }
    /* clear form */
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(nom_entry)), "", -1);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_start_widget)), "", -1);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_end_widget)), "", -1);
}

typedef struct { GtkWidget *dialog; GtkWidget *entry_start; GtkWidget *entry_end; } CancelData;

static void cancel_response_cb(GtkDialog *dialog, int response, gpointer user_data) {
    CancelData *d = (CancelData*)user_data;
    if (response == GTK_RESPONSE_OK) {
        /* Appeler la fonction métier d'annulation qui gère aussi la file d'attente */
        annulation(reservations, &nb_reservations, waiting_list, &waiting_count);
        /* La fonction annulation modifie les tableaux en place ; il faut rafraîchir l'UI */
        update_calendar();
        update_list_view();
        show_message(GTK_WINDOW(d->dialog), "Opération", "Annulation effectuée (voir console pour détails si nécessaire).");
    }
    gtk_window_destroy(GTK_WINDOW(d->dialog));
    g_free(d);
}

void on_cancel_clicked(GtkWidget *button, gpointer data) {
    (void)button; (void)data;
    if (!entry_start_widget || !entry_end_widget) {
        show_message(GTK_WINDOW(window), "Erreur", "Champs de formulaire non initialisés.");
        return;
    }
    const char *s_start = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_start_widget)));
    const char *s_end = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_end_widget)));
    int d1 = atoi(s_start);
    int d2 = atoi(s_end);
    if (d1 < 1 || d2 < d1 || d2 > nombre_jours) {
        show_message(GTK_WINDOW(window), "Erreur", "Dates invalides pour l'annulation.");
        return;
    }
    /* perform cancellation by date range */
    int found = -1;
    for (int i = 0; i < nb_reservations; i++) {
        if (reservations[i].date_debut == d1 && reservations[i].date_fin == d2) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        show_message(GTK_WINDOW(window), "Non trouvé", "Aucune réservation trouvée pour ces dates.");
        return;
    }
    /* remove reservation */
    for (int i = found; i < nb_reservations - 1; i++) reservations[i] = reservations[i+1];
    nb_reservations--;
    client *t = realloc(reservations, (nb_reservations > 0 ? nb_reservations : 0) * sizeof(client));
    if (t || nb_reservations == 0) reservations = t;
    /* try to promote from waiting list */
    try_promote_from_waiting();
    update_calendar();
    update_list_view();
    update_waiting_view();
    show_message(GTK_WINDOW(window), "Annulation", "Réservation annulée.");
}

typedef struct { GtkWidget *dialog; GtkWidget *entry_nom; GtkWidget *entry_newend; } ProlongData;

static void prolong_response_cb(GtkDialog *dialog, int response, gpointer user_data) {
    ProlongData *d = (ProlongData*)user_data;
    if (response == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_nom)));
        int newend = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_newend))));
        int found = -1;
        for (int i = 0; i < nb_reservations; i++) {
        if (name_equal(reservations[i].nom_client, nom)) { found = i; break; }
        }
        if (found == -1) {
            show_message(GTK_WINDOW(d->dialog), "Non trouvé", "Aucune réservation sous ce nom.");
        } else if (newend <= reservations[found].date_fin || newend > nombre_jours) {
            show_message(GTK_WINDOW(d->dialog), "Erreur", "La nouvelle date doit être supérieure à la date de fin actuelle et ≤ 30.");
        } else {
            int start_end = reservations[found].date_fin + 1;
            for (int j = 0; j < nb_reservations; j++) {
                if (j == found) continue;
                if (!((reservations[j].date_fin < start_end) || (newend < reservations[j].date_debut))) {
                    show_message(GTK_WINDOW(d->dialog), "Conflit", "Prolongement impossible : conflit avec une autre réservation.");
                    gtk_window_destroy(GTK_WINDOW(d->dialog));
                    g_free(d);
                    return;
                }
            }
            reservations[found].date_fin = newend;
            update_calendar();
            update_list_view();
            show_message(GTK_WINDOW(d->dialog), "Prolongé", "Séjour prolongé avec succès.");
        }
    }
    gtk_window_destroy(GTK_WINDOW(d->dialog));
    g_free(d);
}

static void new_conflict_response_cb(GtkDialog *dialog, int response, gpointer user_data) {
    ConflictData *d = (ConflictData*)user_data;
    if (response == GTK_RESPONSE_YES) {
        /* Add to waiting list */
        file_d_attente(&waiting_list, &waiting_count, d->tmp);
        update_waiting_view();
        show_message(GTK_WINDOW(d->dialog), "Conflit", "Conflit : le client a été ajouté à la file d'attente.");
    } else {
        /* Modifier: user will edit the form manually */
    }
    gtk_window_destroy(GTK_WINDOW(d->dialog));
    g_free(d);
}

void on_help_clicked(GtkWidget *button, gpointer data) {
    (void)button; (void)data;
    const char *msg =
        "Comment utiliser les boutons :\n\n"
        "• Réserver : remplir Nom, Date début et Date fin puis cliquer 'Réserver'. Si la période est libre, la réservation sera ajoutée.\n\n"
        "• Annuler : renseigner Date début et Date fin exactes de la réservation à annuler et cliquer 'Annuler'.\n\n"
        "• Retard : pour décaler le début d'un séjour (sans changer la date de fin), saisir le Nom et la nouvelle date de début puis cliquer 'Retard'. Le système vérifiera les conflits.\n\n"
        "• Prolonger : pour allonger la date de fin, saisir le Nom et la nouvelle date de fin puis cliquer 'Prolonger'. Le système vérifie les conflits.\n\n"
        "• File d'attente : si votre période est en conflit, vous pouvez choisir de vous ajouter à la file d'attente. Vous serez automatiquement promu(e) si une place se libère.\n";
    show_message(GTK_WINDOW(window), "Aide", msg);
}

void on_prolong_clicked(GtkWidget *button, gpointer data) {
    (void)button; (void)data;
    if (!nom_entry || !entry_end_widget) {
        show_message(GTK_WINDOW(window), "Erreur", "Champs de formulaire non initialisés.");
        return;
    }
    const char *nom = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(nom_entry)));
    const char *s_newend = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_end_widget)));
    int newend = atoi(s_newend);
    int found = -1;
    for (int i = 0; i < nb_reservations; i++) {
    if (name_equal(reservations[i].nom_client, nom)) { found = i; break; }
    }
    if (found == -1) {
        show_message(GTK_WINDOW(window), "Non trouvé", "Aucune réservation sous ce nom.");
        return;
    }
    if (newend <= reservations[found].date_fin || newend > nombre_jours) {
        show_message(GTK_WINDOW(window), "Erreur", "La nouvelle date doit être supérieure à la date de fin actuelle et ≤ 30.");
        return;
    }
    int start_end = reservations[found].date_fin + 1;
    for (int j = 0; j < nb_reservations; j++) {
        if (j == found) continue;
        if (!((reservations[j].date_fin < start_end) || (newend < reservations[j].date_debut))) {
            show_message(GTK_WINDOW(window), "Conflit", "Prolongement impossible : conflit avec une autre réservation.");
            return;
        }
    }
    reservations[found].date_fin = newend;
    update_calendar();
    update_list_view();
    show_message(GTK_WINDOW(window), "Prolongé", "Séjour prolongé avec succès.");
}

typedef struct { GtkWidget *dialog; GtkWidget *entry_nom; GtkWidget *entry_newstart; } DelayData;

static void delay_response_cb(GtkDialog *dialog, int response, gpointer user_data) {
    DelayData *d = (DelayData*)user_data;
    if (response == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_nom)));
        int newstart = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->entry_newstart))));
        int found = -1;
        for (int i = 0; i < nb_reservations; i++) {
        if (name_equal(reservations[i].nom_client, nom)) { found = i; break; }
        }
        if (found == -1) {
            show_message(GTK_WINDOW(d->dialog), "Non trouvé", "Aucune réservation sous ce nom.");
            gtk_window_destroy(GTK_WINDOW(d->dialog));
            g_free(d);
            return;
        }
        int old_start = reservations[found].date_debut;
        int old_end = reservations[found].date_fin;
        /* newstart must be strictly greater than old start and not after old end */
        if (newstart <= old_start || newstart < 1 || newstart > old_end) {
            show_message(GTK_WINDOW(d->dialog), "Erreur", "La nouvelle date de début doit être > date de début actuelle et ≤ date de fin actuelle.");
            gtk_window_destroy(GTK_WINDOW(d->dialog));
            g_free(d);
            return;
        }
        int new_start = newstart;
        int new_end = old_end; /* keep same end date, only change start */
        /* check conflicts for the new interval [new_start .. new_end] */
        for (int j = 0; j < nb_reservations; j++) {
            if (j == found) continue;
            if (!((reservations[j].date_fin < new_start) || (new_end < reservations[j].date_debut))) {
                show_message(GTK_WINDOW(d->dialog), "Conflit", "Retard impossible : conflit avec d'autres réservations.");
                gtk_window_destroy(GTK_WINDOW(d->dialog));
                g_free(d);
                return;
            }
        }
        reservations[found].date_debut = new_start;
        /* reservations[found].date_fin remains old_end */
        update_calendar();
        update_list_view();
        /* After applying a delay, try to promote someone from the waiting list */
        try_promote_from_waiting();
        update_waiting_view();
        show_message(GTK_WINDOW(d->dialog), "Retard", "Retard appliqué avec succès.");
    }
    gtk_window_destroy(GTK_WINDOW(d->dialog));
    g_free(d);
}

void on_delay_clicked(GtkWidget *button, gpointer data) {
    (void)button; (void)data;
    if (!nom_entry || !entry_start_widget) {
        show_message(GTK_WINDOW(window), "Erreur", "Champs de formulaire non initialisés.");
        return;
    }
    const char *nom = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(nom_entry)));
    const char *s_newstart = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_start_widget)));
    int newstart = atoi(s_newstart);
    int found = -1;
    for (int i = 0; i < nb_reservations; i++) {
    if (name_equal(reservations[i].nom_client, nom)) { found = i; break; }
    }
    if (found == -1) {
        show_message(GTK_WINDOW(window), "Non trouvé", "Aucune réservation sous ce nom.");
        return;
    }
    int old_start = reservations[found].date_debut;
    int old_end = reservations[found].date_fin;
    if (newstart <= old_start || newstart < 1 || newstart > old_end) {
        show_message(GTK_WINDOW(window), "Erreur", "La nouvelle date de début doit être > date de début actuelle et ≤ date de fin actuelle.");
        return;
    }
    int new_start = newstart;
    int new_end = old_end; /* keep same end date, only change start */
    for (int j = 0; j < nb_reservations; j++) {
        if (j == found) continue;
        if (!((reservations[j].date_fin < new_start) || (new_end < reservations[j].date_debut))) {
            show_message(GTK_WINDOW(window), "Conflit", "Retard impossible : conflit avec d'autres réservations.");
            return;
        }
    }
    reservations[found].date_debut = new_start;
    update_calendar();
    update_list_view();
    /* After applying a delay, try to promote someone from the waiting list */
    try_promote_from_waiting();
    update_waiting_view();
    show_message(GTK_WINDOW(window), "Retard", "Retard appliqué avec succès.");
    /* All done; previous dialog-based code removed. */
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.hotel.gtk4", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    /* Sauvegarder les réservations avant de quitter */
    if (reservations != NULL) {
        sauvegarder_reservations(reservations, nb_reservations, FICHIER_RESERVATIONS);
        free(reservations);
        reservations = NULL;
    }
    if (waiting_list != NULL) {
        sauvegarder_reservations(waiting_list, waiting_count, FICHIER_ATTENTE);
        free(waiting_list);
        waiting_list = NULL;
    }
    return status;
}
