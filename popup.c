#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>
#include <gio/gio.h>

#define DBUS_NAME "org.example.LauncherDaemon"
#define DBUS_PATH "/org/example/LauncherDaemon"
#define DBUS_INTERFACE "org.example.LauncherDaemon"

static GtkWidget *window;
static GtkWidget *box;
static GtkWidget *entry;

static void launch_application(const char *app_name) {
    GError *error = NULL;
    GAppInfo *app_info = g_app_info_create_from_commandline(app_name, NULL, G_APP_INFO_CREATE_NONE, &error);

    if (error) {
        g_warning("Error creating app info: %s", error->message);
        g_error_free(error);
        return;
    }

    if (!g_app_info_launch(app_info, NULL, NULL, &error)) {
        g_warning("Error launching app: %s", error->message);
        g_error_free(error);
    }

    g_object_unref(app_info);
}

static void on_entry_activated(GtkEntry *entry, gpointer user_data) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    launch_application(text);
    gtk_editable_set_text(GTK_EDITABLE(entry), "");
    gtk_widget_set_visible(window, FALSE);
}

static void handle_method_call(GDBusConnection       *connection,
                               const gchar           *sender,
                               const gchar           *object_path,
                               const gchar           *interface_name,
                               const gchar           *method_name,
                               GVariant              *parameters,
                               GDBusMethodInvocation *invocation,
                               gpointer               user_data) {
  if (g_strcmp0(method_name, "ShowLauncher") == 0) {
    gtk_widget_set_visible(window, TRUE);
    gtk_window_present(GTK_WINDOW(window));
    gtk_widget_grab_focus(entry);
    g_dbus_method_invocation_return_value(invocation, NULL);
  } else if (g_strcmp0(method_name, "HideLauncher") == 0) {
    gtk_widget_set_visible(window, FALSE);
    g_dbus_method_invocation_return_value(invocation, NULL);
  }
}

static const GDBusInterfaceVTable interface_vtable = {
    handle_method_call,
    NULL,
    NULL,
};

static void on_bus_acquired(GDBusConnection *connection,
                            const gchar     *name,
                            gpointer         user_data) {
    GDBusNodeInfo *introspection_data = g_dbus_node_info_new_for_xml(
        "<node>"
        "  <interface name='" DBUS_INTERFACE "'>"
        "    <method name='ShowLauncher'>"
        "    </method>"
        "    <method name='HideLauncher'>"
        "    </method>"
        "  </interface>"
        "</node>",
        NULL);

    g_dbus_connection_register_object(connection,
                                      DBUS_PATH,
                                      introspection_data->interfaces[0],
                                      &interface_vtable,
                                      NULL,
                                      NULL,
                                      NULL);

    g_dbus_node_info_unref(introspection_data);
}

static void activate(GtkApplication* app, void *_data) {
  (void)_data;

  window = GTK_WIDGET(gtk_application_window_new(app));

  gtk_layer_init_for_window(GTK_WINDOW(window));
  gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
  gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 50);

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, "style.css");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
					     GTK_STYLE_PROVIDER(provider),
					     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_add_css_class(box, "launcher-box");
  gtk_window_set_child(GTK_WINDOW(window), box);

  entry = gtk_entry_new();
  gtk_widget_add_css_class(entry, "launcher-entry");
  g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activated), NULL);
  gtk_box_append(GTK_BOX(box), entry);

  gtk_widget_set_visible(window, FALSE);
}


int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("gtk.launcher", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    g_bus_own_name(G_BUS_TYPE_SESSION,
                   DBUS_NAME,
                   G_BUS_NAME_OWNER_FLAGS_NONE,
                   on_bus_acquired,
                   NULL,
                   NULL,
                   NULL,
                   NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
