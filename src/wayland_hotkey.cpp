#include "wayland_hotkey.h"

#include <gio/gio.h>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <thread>

static const char *PORTAL_DEST     = "org.freedesktop.portal.Desktop";
static const char *PORTAL_PATH     = "/org/freedesktop/portal/desktop";
static const char *SHORTCUTS_IFACE = "org.freedesktop.portal.GlobalShortcuts";
static const char *REQUEST_IFACE   = "org.freedesktop.portal.Request";

static GDBusConnection      *conn           = nullptr;
static gchar                *session_handle = nullptr;
static GMainLoop            *main_loop      = nullptr;
static std::function<void()> g_on_toggle;
static std::thread           g_thread;

// ---------- hotkey signals ----------

static void on_activated(GDBusConnection *, const gchar *, const gchar *,
                          const gchar *, const gchar *, GVariant *params,
                          gpointer) {
    const gchar *sess, *shortcut_id;
    guint64 timestamp;
    GVariant *opts;
    g_variant_get(params, "(&o&st@a{sv})", &sess, &shortcut_id, &timestamp, &opts);
    g_variant_unref(opts);

    if (g_strcmp0(shortcut_id, "toggle-proxy") == 0 && g_on_toggle)
        g_on_toggle();
}

// ---------- async setup: BindShortcuts response ----------

static void on_bind_response(GDBusConnection *c, const gchar *, const gchar *,
                              const gchar *, const gchar *, GVariant *params,
                              gpointer ud) {
    guint *sub = static_cast<guint *>(ud);
    g_dbus_connection_signal_unsubscribe(c, *sub);
    g_free(sub);

    guint32 code;
    GVariant *res;
    g_variant_get(params, "(u@a{sv})", &code, &res);
    g_variant_unref(res);

    if (code != 0) {
        g_printerr("BindShortcuts denied or failed (code=%u).\n", code);
        g_main_loop_quit(main_loop);
        return;
    }

    g_print("Hotkey shortcuts bound.\n");

    g_dbus_connection_signal_subscribe(
        conn, PORTAL_DEST, SHORTCUTS_IFACE, "Activated",
        PORTAL_PATH, nullptr, G_DBUS_SIGNAL_FLAGS_NONE,
        on_activated, nullptr, nullptr);
}

// ---------- async setup: BindShortcuts call done ----------

static void on_bind_call_done(GObject *src, GAsyncResult *ar, gpointer) {
    GError *err = nullptr;
    GVariant *ret = g_dbus_connection_call_finish(G_DBUS_CONNECTION(src), ar, &err);
    if (!ret) {
        g_printerr("BindShortcuts call failed: %s\n", err->message);
        g_error_free(err);
        g_main_loop_quit(main_loop);
        return;
    }

    const gchar *req_path;
    g_variant_get(ret, "(o)", &req_path);

    guint *sub = g_new(guint, 1);
    *sub = g_dbus_connection_signal_subscribe(
        conn, PORTAL_DEST, REQUEST_IFACE, "Response",
        req_path, nullptr, G_DBUS_SIGNAL_FLAGS_NONE,
        on_bind_response, sub, nullptr);

    g_variant_unref(ret);
}

// ---------- async setup: BindShortcuts initiation ----------

static void start_bind_shortcuts() {
    GVariantBuilder shortcuts;
    g_variant_builder_init(&shortcuts, G_VARIANT_TYPE("a(sa{sv})"));
    {
        GVariantBuilder props;
        g_variant_builder_init(&props, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&props, "{sv}", "description",
                              g_variant_new_string("Toggle media proxy"));
        g_variant_builder_add(&shortcuts, "(sa{sv})", "toggle-proxy", &props);
    }

    GVariantBuilder bind_opts;
    g_variant_builder_init(&bind_opts, G_VARIANT_TYPE("a{sv}"));
    char bind_tok[32];
    snprintf(bind_tok, sizeof(bind_tok), "hkb_%x", (unsigned)rand());
    g_variant_builder_add(&bind_opts, "{sv}", "handle_token",
                          g_variant_new_string(bind_tok));

    g_dbus_connection_call(
        conn, PORTAL_DEST, PORTAL_PATH, SHORTCUTS_IFACE, "BindShortcuts",
        g_variant_new("(oa(sa{sv})sa{sv})", session_handle,
                      &shortcuts, "", &bind_opts),
        nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr,
        on_bind_call_done, nullptr);
}

// ---------- async setup: CreateSession response ----------

static void on_create_session_response(GDBusConnection *c, const gchar *,
                                        const gchar *, const gchar *,
                                        const gchar *, GVariant *params,
                                        gpointer ud) {
    guint *sub = static_cast<guint *>(ud);
    g_dbus_connection_signal_unsubscribe(c, *sub);
    g_free(sub);

    guint32 code;
    GVariant *res;
    g_variant_get(params, "(u@a{sv})", &code, &res);

    if (code != 0) {
        g_variant_unref(res);
        g_printerr("CreateSession was denied or failed (code=%u).\n", code);
        g_main_loop_quit(main_loop);
        return;
    }

    GVariant *sh = g_variant_lookup_value(res, "session_handle", G_VARIANT_TYPE_STRING);
    session_handle = g_strdup(g_variant_get_string(sh, nullptr));
    g_variant_unref(sh);
    g_variant_unref(res);

    start_bind_shortcuts();
}

// ---------- async setup: CreateSession call done ----------

static void on_create_session_call_done(GObject *src, GAsyncResult *ar, gpointer) {
    GError *err = nullptr;
    GVariant *ret = g_dbus_connection_call_finish(G_DBUS_CONNECTION(src), ar, &err);
    if (!ret) {
        g_printerr("CreateSession call failed: %s\n", err->message);
        g_error_free(err);
        g_main_loop_quit(main_loop);
        return;
    }

    const gchar *req_path;
    g_variant_get(ret, "(o)", &req_path);

    guint *sub = g_new(guint, 1);
    *sub = g_dbus_connection_signal_subscribe(
        conn, PORTAL_DEST, REQUEST_IFACE, "Response",
        req_path, nullptr, G_DBUS_SIGNAL_FLAGS_NONE,
        on_create_session_response, sub, nullptr);

    g_variant_unref(ret);
}

// ---------- public API ----------

void wayland_hotkey_start(std::function<void()> on_toggle) {
    g_on_toggle = std::move(on_toggle);

    g_thread = std::thread([] {
        GError *err = nullptr;
        conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
        if (!conn) {
            g_printerr("D-Bus connect failed: %s\n", err->message);
            g_error_free(err);
            return;
        }

        // unique tokens force the compositor to create a fresh session each run
        char sess_tok[32], req_tok[32];
        snprintf(sess_tok, sizeof(sess_tok), "hk_%x", (unsigned)rand());
        snprintf(req_tok,  sizeof(req_tok),  "hkr_%x", (unsigned)rand());

        GVariantBuilder opts;
        g_variant_builder_init(&opts, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&opts, "{sv}", "session_handle_token",
                              g_variant_new_string(sess_tok));
        g_variant_builder_add(&opts, "{sv}", "handle_token",
                              g_variant_new_string(req_tok));

        g_dbus_connection_call(
            conn, PORTAL_DEST, PORTAL_PATH, SHORTCUTS_IFACE, "CreateSession",
            g_variant_new("(a{sv})", &opts),
            nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr,
            on_create_session_call_done, nullptr);

        main_loop = g_main_loop_new(nullptr, FALSE);
        g_main_loop_run(main_loop);

        g_main_loop_unref(main_loop);
        main_loop = nullptr;
        g_free(session_handle);
        session_handle = nullptr;
        g_object_unref(conn);
        conn = nullptr;
    });
}

void wayland_hotkey_stop() {
    if (main_loop)
        g_main_loop_quit(main_loop);
    if (g_thread.joinable())
        g_thread.join();
}
