#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#define STAT_LEN 50
#define nombre_jours 30

// ============= STRUCTURES =============
typedef struct {
    char nom_client[STAT_LEN];
    int date_debut;
    int date_fin;
    GdkRGBA color;
} client;

// ============= VARIABLES GLOBALES =============
client *reservations = NULL;
int nb_reservations = 0;
int selected_start = -1;
int selected_end = -1;

// Widgets principaux
GtkWidget *window;
GtkWidget *calendar_grid;
GtkWidget *status_label;
GtkWidget *stats_label;
GtkWidget *nom_entry;
GtkWidget *day_buttons[nombre_jours];
GtkWidget *list_view;
GtkWidget *progress_bar;

// ==================== DECLARATION CALLBACKS ====================
void on_day_clicked(GtkWidget *button, gpointer data);
void on_delete_clicked(GtkWidget *button, gpointer data);
void on_add_reservation_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void on_new_reservation_clicked(GtkWidget *button, gpointer data);
void on_reset_clicked(GtkWidget *button, gpointer data);
void on_view_stats_clicked(GtkWidget *button, gpointer data);

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
    for (int i = 0; i < nb_reservations; i++) {
        if (day >= reservations[i].date_debut && day <= reservations[i].date_fin) {
            *color = reservations[i].color;
            return;
        }
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

        char label[100];
        if (occupant)
            snprintf(label, 100, "📅 %d\n👤 %.8s", day, occupant);
        else
            snprintf(label, 100, "📅 %d\n✓ Libre", day);

        gtk_button_set_label(GTK_BUTTON(day_buttons[i]), label);

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

    char stats[200];
    snprintf(stats, 200, "📊 <b>%d</b> réservations | 📅 Taux d'occupation: <b>%.1f%%</b>", nb_reservations, calculate_occupancy() * 100);
    gtk_label_set_markup(GTK_LABEL(stats_label), stats);
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

        GtkWidget *color_box = gtk_drawing_area_new();
        gtk_widget_set_size_request(color_box, 30, 40);

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

        gtk_box_append(GTK_BOX(box), color_box);
        gtk_box_append(GTK_BOX(box), label);
        gtk_box_append(GTK_BOX(box), delete_btn);

        gtk_box_append(GTK_BOX(list_view), box);
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
    for (int i = index; i < nb_reservations - 1; i++) reservations[i] = reservations[i+1];
    nb_reservations--;
    update_calendar();
    update_list_view();
}

// ==================== CSS ====================
void apply_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "button.day-button { min-width: 90px; min-height:70px; }"
        "button.day-button.free { background:#4CAF50; }"
        "button.day-button.occupied { background:#F44336; }"
        "button.day-button.selected { background:#2196F3; }";
    gtk_css_provider_load_from_data(provider, css, -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

// ==================== INTERFACE ====================
void activate(GtkApplication *app, gpointer user_data) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🏨 Gestion Chambre d'Hôtel");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    stats_label = gtk_label_new("📊 0 réservations | 📅 Taux: 0%");
    progress_bar = gtk_progress_bar_new();

    gtk_box_append(GTK_BOX(main_box), stats_label);
    gtk_box_append(GTK_BOX(main_box), progress_bar);

    calendar_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(calendar_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(calendar_grid), 5);

    for (int i=0; i<nombre_jours; i++) {
        day_buttons[i] = gtk_button_new_with_label("");
        gtk_widget_add_css_class(day_buttons[i], "day-button");
        g_signal_connect(day_buttons[i], "clicked", G_CALLBACK(on_day_clicked), GINT_TO_POINTER(i));
        gtk_grid_attach(GTK_GRID(calendar_grid), day_buttons[i], i%10, i/10, 1, 1);
    }

    GtkWidget *calendar_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(calendar_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(calendar_scroll), calendar_grid);
    gtk_box_append(GTK_BOX(main_box), calendar_scroll);

    list_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *list_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(list_scroll), list_view);
    gtk_box_append(GTK_BOX(main_box), list_scroll);

    apply_css();
    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.hotel.gtk4", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
