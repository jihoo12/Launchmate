#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>

static GtkWidget *window;
static GtkWidget *box;
static GtkWidget *entry;

static void launch_application(const char *app_name) {
    GError *error = NULL;
    GAppInfo *app_info = g_app_info_create_from_commandline(app_name, NULL, G_APP_INFO_CREATE_NONE, &error);

    if (error) {
        g_warning("애플리케이션 정보 생성 오류: %s", error->message);
        g_error_free(error);
        return;
    }

    if (!g_app_info_launch(app_info, NULL, NULL, &error)) {
        g_warning("애플리케이션 실행 오류: %s", error->message);
        g_error_free(error);
    }

    g_object_unref(app_info);
}

static void on_entry_activated(GtkEntry *entry, gpointer user_data) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    launch_application(text);
    gtk_editable_set_text(GTK_EDITABLE(entry), "");
    gtk_window_close(GTK_WINDOW(window));
}

static gboolean on_key_press(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    if (keyval == GDK_KEY_Escape) {
        gtk_window_close(GTK_WINDOW(window));
        return TRUE;
    }
    return FALSE;
}

static void activate(GtkApplication* app, void *_data) {
    (void)_data;

    window = GTK_WIDGET(gtk_application_window_new(app));

    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
    
    // 창을 화면 중앙에 위치시키기
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, FALSE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);

    // 마진 설정 (화면 가장자리로부터의 거리)
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, 100);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, 100);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, 100);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, 100);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "$HOME/.config/launchmate/style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                        GTK_STYLE_PROVIDER(provider),
                        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(box,300,300);
    gtk_widget_add_css_class(box, "launcher-box");
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_window_set_child(GTK_WINDOW(window), box);

    entry = gtk_entry_new();
    gtk_widget_add_css_class(entry, "launcher-entry");
    gtk_widget_set_size_request(entry, 300, 50);  // 입력 상자의 크기를 명시적으로 설정
    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activated), NULL);
    gtk_box_append(GTK_BOX(box), entry);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_press), NULL);
    gtk_widget_add_controller(window, key_controller);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
    gtk_widget_set_visible(window, TRUE);
    gtk_widget_grab_focus(entry);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("gtk.launcher", G_APPLICATION_DEFAULT_FLAGS);
    
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_unref(app);
    
    return status;
}
