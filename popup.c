#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>
#include <gio/gio.h>

#define DBUS_NAME "org.example.PopupDaemon"
#define DBUS_PATH "/org/example/PopupDaemon"
#define DBUS_INTERFACE "org.example.PopupDaemon"

static GtkWidget *label;

static void handle_method_call(GDBusConnection       *connection,
                               const gchar           *sender,
                               const gchar           *object_path,
                               const gchar           *interface_name,
                               const gchar           *method_name,
                               GVariant              *parameters,
                               GDBusMethodInvocation *invocation,
                               gpointer               user_data) {
    if (g_strcmp0(method_name, "SetLabel") == 0) {
        const gchar *new_text;
        g_variant_get(parameters, "(&s)", &new_text);
        gtk_label_set_text(GTK_LABEL(label), new_text);
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
        "    <method name='SetLabel'>"
        "      <arg type='s' name='new_text' direction='in'/>"
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

    GtkWindow *gtk_window = GTK_WINDOW(gtk_application_window_new(app));

    gtk_layer_init_for_window(gtk_window);
    gtk_window_set_default_size(gtk_window, 300, 300);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    label = gtk_label_new("Hello, World!");
    gtk_widget_add_css_class(GTK_WIDGET(label), "label");
    gtk_window_set_child(gtk_window, label);
    gtk_window_present(gtk_window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("gtk.popup", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // D-Bus 서비스 등록
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
