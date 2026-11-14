#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *status_dot;
    GtkWidget *btn_theme;
    GtkWidget *combo_auth;
    GtkWidget *entry_user;
    GtkWidget *btn_login;
    GtkWidget *combo_loader;
    GtkWidget *combo_version;
    GtkWidget *chk_snap;
    GtkWidget *chk_beta;
    GtkWidget *entry_dir;
    GtkWidget *text_log;
    GtkTextBuffer *log_buffer;
    GtkWidget *btn_launch;
    GtkWidget *progress_bar;
    GtkWidget *stack_action;
    GtkCssProvider *css_provider;

    gboolean is_dark;
    gboolean is_running;
    gboolean was_cancelled;
    int app_status; /* 0 = ready (green), 1 = running (orange), 2 = error (red) */
    GPid child_pid;
} AppData;

static const char *ALL_VERSIONS[] = {
    "26.4", "26.1.2", "26.1.1", "26.1",
    "1.21.11", "1.21.10", "1.21.8", "1.21.5", "1.21.4", "1.21.3", "1.21.1", "1.21",
    "1.20.6", "1.20.4", "1.20.2", "1.20.1", "1.20",
    "1.19.4", "1.19.2", "1.18.2", "1.17.1", "1.16.5", "1.12.2", "1.8.9", "1.7.10",
    "26w14a", "25w14craftmine", "24w14potato", "23w13a_or_b", "20w14infinite",
    "b1.7.3", "b1.8.1", "a1.2.6", "a1.1.2_01", "c0.30_01c", "rd-132211",
    NULL
};

static gboolean is_snapshot_ver(const char *v) {
    return (strchr(v, 'w') != NULL || strstr(v, "pre") != NULL || strstr(v, "rc") != NULL ||
            strstr(v, "craftmine") != NULL || strstr(v, "potato") != NULL || strstr(v, "infinite") != NULL);
}

static gboolean is_beta_ver(const char *v) {
    return (v[0] == 'b' || v[0] == 'a' || v[0] == 'c' || strncmp(v, "rd-", 3) == 0);
}

static void mkdir_p(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

static gboolean copy_file_bin(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return FALSE;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return FALSE; }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in); fclose(out);
            return FALSE;
        }
    }
    fclose(in);
    fclose(out);
    return TRUE;
}

static void append_log(AppData *app, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->log_buffer, &end);
    gtk_text_buffer_insert(app->log_buffer, &end, text, -1);
    GtkTextMark *mark = gtk_text_buffer_create_mark(app->log_buffer, NULL, &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->text_log), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(app->log_buffer, mark);
}

static void apply_theme(AppData *app) {
    const char *css_light =
        "window { background-image: linear-gradient(to bottom, #f5f5f7, #e5e5ea); color: #1c1c1e; }"
        "entry, combobox button, textview text { background-color: #ffffff; color: #1c1c1e; border-radius: 10px; border: 1px solid #d1d1d6; padding: 5px 8px; }"
        "button.pill { background-image: none; background-color: #ffffff; color: #1c1c1e; border-radius: 10px; border: 1px solid #d1d1d6; padding: 5px 10px; font-weight: 600; }"
        "button.pill:hover { background-color: #f0f0f5; }"
        "button.launch { background-image: none; background-color: #007aff; color: #ffffff; border-radius: 14px; border: none; font-weight: bold; font-size: 14px; padding: 10px; }"
        "button.launch:hover { background-color: #0066d6; }"
        "progressbar trough { min-height: 40px; border-radius: 14px; background-color: #d1d1d6; }"
        "progressbar progress { min-height: 40px; border-radius: 14px; background-color: #34c759; }";

    const char *css_dark =
        "window { background-image: linear-gradient(to bottom, #1c1c1e, #101012); color: #f5f5f7; }"
        "label, checkbutton { color: #f5f5f7; }"
        "entry, combobox button, textview text { background-color: #2c2c2e; color: #f5f5f7; border-radius: 10px; border: 1px solid #3a3a3c; padding: 5px 8px; }"
        "button.pill { background-image: none; background-color: #2c2c2e; color: #f5f5f7; border-radius: 10px; border: 1px solid #3a3a3c; padding: 5px 10px; font-weight: 600; }"
        "button.pill:hover { background-color: #3a3a3c; }"
        "button.launch { background-image: none; background-color: #0a84ff; color: #ffffff; border-radius: 14px; border: none; font-weight: bold; font-size: 14px; padding: 10px; }"
        "button.launch:hover { background-color: #0070e0; }"
        "progressbar trough { min-height: 40px; border-radius: 14px; background-color: #3a3a3c; }"
        "progressbar progress { min-height: 40px; border-radius: 14px; background-color: #30d158; }";

    gtk_css_provider_load_from_data(app->css_provider, app->is_dark ? css_dark : css_light, -1, NULL);
    gtk_button_set_label(GTK_BUTTON(app->btn_theme), app->is_dark ? "Dark Theme" : "Light Theme");
}

static gboolean on_draw_status_dot(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app = (AppData *)data;
    if (app->app_status == 0) cairo_set_source_rgb(cr, 0.20, 0.78, 0.35);
    else if (app->app_status == 1) cairo_set_source_rgb(cr, 1.00, 0.58, 0.00);
    else cairo_set_source_rgb(cr, 1.00, 0.23, 0.19);

    cairo_arc(cr, 6, 6, 5, 0, 2 * G_PI);
    cairo_fill(cr);
    return FALSE;
}

static void show_apple_alert(AppData *app, const char *title, const char *msg, const char *folder_to_open) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, "%s", title);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg);

    if (folder_to_open) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "Open Folder", GTK_RESPONSE_ACCEPT);
    }
    gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", GTK_RESPONSE_OK);

    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT && folder_to_open) {
        char cmd[1200];
        snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" &", folder_to_open);
        system(cmd);
    }
    gtk_widget_destroy(dialog);
}

static void populate_versions(AppData *app) {
    gboolean show_snap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_snap));
    gboolean show_beta = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_beta));

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(app->combo_version));
    for (int i = 0; ALL_VERSIONS[i] != NULL; i++) {
        const char *v = ALL_VERSIONS[i];
        if (is_snapshot_ver(v) && !show_snap) continue;
        if (is_beta_ver(v) && !show_beta) continue;
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_version), v);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_version), 0);
}

static void on_toggle_theme(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = (AppData *)data;
    app->is_dark = !app->is_dark;
    apply_theme(app);
}

static void on_auth_changed(GtkComboBox *combo, gpointer data) {
    AppData *app = (AppData *)data;
    int idx = gtk_combo_box_get_active(combo);
    if (idx == 1) {
        gtk_widget_show(app->btn_login);
        gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry_user), "Optional Microsoft nickname...");
    } else {
        gtk_widget_hide(app->btn_login);
        gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry_user), "Enter offline username...");
    }
}

static void on_browse_dir(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = (AppData *)data;
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Select Game Folder",
        GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Select", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        gtk_entry_set_text(GTK_ENTRY(app->entry_dir), folder);
        g_free(folder);
    }
    gtk_widget_destroy(dlg);
}

static void on_import_mod(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = (AppData *)data;
    const char *base_dir = gtk_entry_get_text(GTK_ENTRY(app->entry_dir));
    if (!base_dir || strlen(base_dir) == 0) {
        show_apple_alert(app, "Missing Game Folder", "Please select a destination game folder first.", NULL);
        return;
    }

    GtkWidget *dlg = gtk_file_chooser_dialog_new("Select Mod (.jar)",
        GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Import", GTK_RESPONSE_ACCEPT, NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Minecraft Mod (*.jar)");
    gtk_file_filter_add_pattern(filter, "*.jar");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), filter);

    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *src = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        char mods_dir[1024], dst[1200];
        snprintf(mods_dir, sizeof(mods_dir), "%s/mods", base_dir);
        mkdir_p(mods_dir);

        const char *fname = strrchr(src, '/');
        fname = fname ? fname + 1 : src;
        snprintf(dst, sizeof(dst), "%s/%s", mods_dir, fname);

        if (copy_file_bin(src, dst)) {
            show_apple_alert(app, "Mod Imported", "The .jar mod was successfully copied to your mods folder.", mods_dir);
        } else {
            show_apple_alert(app, "Import Error", "Failed to copy the .jar file.", NULL);
        }
        g_free(src);
    }
    gtk_widget_destroy(dlg);
}

static void import_skin_or_cape(AppData *app, gboolean is_cape) {
    int auth = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_auth));
    if (auth == 1) {
        system("xdg-open \"https://minecraft.net\" &");
        return;
    }

    int loader = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_loader));
    if (loader == 0) {
        show_apple_alert(app, "Vanilla Not Supported",
            "Offline skins and capes cannot be applied on Vanilla.\n\nPlease select Fabric, Forge, NeoForge, or Quilt and install the CustomSkinLoader mod.",
            NULL);
        return;
    }

    const char *base_dir = gtk_entry_get_text(GTK_ENTRY(app->entry_dir));
    if (!base_dir || strlen(base_dir) == 0) {
        show_apple_alert(app, "Missing Game Folder", "Please select a destination game folder first.", NULL);
        return;
    }

    const char *raw_user = gtk_entry_get_text(GTK_ENTRY(app->entry_user));
    const char *user = (raw_user && strlen(raw_user) > 0) ? raw_user : "Steve";

    GtkWidget *dlg = gtk_file_chooser_dialog_new(is_cape ? "Select Cape (.png)" : "Select Skin (.png)",
        GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Import", GTK_RESPONSE_ACCEPT, NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "PNG Texture (*.png)");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), filter);

    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *src = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        char target_dir[1024], dst_png[1200], json_path[1100];
        snprintf(target_dir, sizeof(target_dir), "%s/CustomSkinLoader/LocalSkin/%s",
                 base_dir, is_cape ? "capes" : "skins");
        mkdir_p(target_dir);

        snprintf(dst_png, sizeof(dst_png), "%s/%s.png", target_dir, user);
        if (copy_file_bin(src, dst_png)) {
            snprintf(json_path, sizeof(json_path), "%s/CustomSkinLoader/CustomSkinLoader.json", base_dir);
            if (access(json_path, F_OK) != 0) {
                FILE *fj = fopen(json_path, "w");
                if (fj) {
                    fprintf(fj,
                        "{\n"
                        "  \"enable\": true,\n"
                        "  \"enableDynamicSkull\": true,\n"
                        "  \"enableTransparentSkin\": true,\n"
                        "  \"forceLoadAllTextures\": true,\n"
                        "  \"enableLocalProfile\": true,\n"
                        "  \"loadlist\": [\n"
                        "    {\n"
                        "      \"name\": \"LocalSkin\",\n"
                        "      \"type\": \"Legacy\",\n"
                        "      \"checkPNG\": false,\n"
                        "      \"skin\": \"LocalSkin/skins/{USERNAME}.png\",\n"
                        "      \"cape\": \"LocalSkin/capes/{USERNAME}.png\",\n"
                        "      \"elytra\": \"LocalSkin/elytras/{USERNAME}.png\"\n"
                        "    }\n"
                        "  ]\n"
                        "}\n");
                    fclose(fj);
                }
            }
            show_apple_alert(app, is_cape ? "Cape Imported" : "Skin Imported",
                "Texture saved! Make sure CustomSkinLoader is inside your mods folder.", target_dir);
        } else {
            show_apple_alert(app, "Import Error", "Failed to copy the .png file.", NULL);
        }
        g_free(src);
    }
    gtk_widget_destroy(dlg);
}

static void on_import_skin(GtkButton *btn, gpointer data) { (void)btn; import_skin_or_cape((AppData *)data, FALSE); }
static void on_import_cape(GtkButton *btn, gpointer data) { (void)btn; import_skin_or_cape((AppData *)data, TRUE); }
static void on_child_watch(GPid pid, gint status, gpointer data) {
    AppData *app = (AppData *)data;
    g_spawn_close_pid(pid);
    app->is_running = FALSE;
    app->child_pid = 0;

    gtk_stack_set_visible_child_name(GTK_STACK(app->stack_action), "launch");

    if (app->was_cancelled) {
        app->app_status = 2;
        append_log(app, "\n--- Process cancelled by user ---\n");
        const char *dir = gtk_entry_get_text(GTK_ENTRY(app->entry_dir));
        show_apple_alert(app, "Download Cancelled",
            "The process was stopped.\nIf a download was interrupted, you may want to check your game folder.", dir);
    } else if (status == 0) {
        app->app_status = 0;
        append_log(app, "\n--- Process finished ---\n");
    } else {
        app->app_status = 2;
        append_log(app, "\n--- Process exited with errors ---\n");
    }
    gtk_widget_queue_draw(app->status_dot);
}

static gboolean on_child_stdout(GIOChannel *channel, GIOCondition cond, gpointer data) {
    AppData *app = (AppData *)data;
    if (cond & G_IO_HUP) {
        g_io_channel_unref(channel);
        return FALSE;
    }
    gchar *line = NULL;
    gsize len = 0;
    if (g_io_channel_read_line(channel, &line, &len, NULL, NULL) == G_IO_STATUS_NORMAL && line) {
        append_log(app, line);
        char *pct_ptr = strchr(line, '%');
        if (pct_ptr && pct_ptr > line) {
            int val = atoi(pct_ptr - 3);
            if (val >= 0 && val <= 100) {
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), val / 100.0);
                char txt[64];
                snprintf(txt, sizeof(txt), "DOWNLOADING... %d%% (Click to Cancel)", val);
                gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app->progress_bar), txt);
            }
        }
        g_free(line);
    }
    return TRUE;
}

static void start_cli_command(AppData *app, char **argv) {
    GError *err = NULL;
    gint out_fd = -1;

    app->is_running = TRUE;
    app->was_cancelled = FALSE;
    app->app_status = 1;
    gtk_widget_queue_draw(app->status_dot);

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.15);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app->progress_bar), "RUNNING... (Click to Cancel)");
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack_action), "progress");

    if (!g_spawn_async_with_pipes(NULL, argv, NULL,
            G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL,
            NULL, NULL, &app->child_pid, NULL, &out_fd, NULL, &err)) {
        append_log(app, "[ERROR] Could not run portablemc. Make sure 'portablemc' is installed or placed next to this app.\n");
        if (err) { append_log(app, err->message); append_log(app, "\n"); g_error_free(err); }
        app->is_running = FALSE;
        app->app_status = 2;
        gtk_widget_queue_draw(app->status_dot);
        gtk_stack_set_visible_child_name(GTK_STACK(app->stack_action), "launch");
        return;
    }

    g_child_watch_add(app->child_pid, on_child_watch, app);
    GIOChannel *ch = g_io_channel_unix_new(out_fd);
    g_io_channel_set_encoding(ch, NULL, NULL);
    g_io_channel_set_flags(ch, G_IO_FLAG_NONBLOCK, NULL);
    g_io_add_watch(ch, G_IO_IN | G_IO_HUP, on_child_stdout, app);
}
static const char *resolve_pmc_bin(void) {
    if (access("./portablemc", X_OK) == 0) return "./portablemc";
    if (access("./portablemc-cli", X_OK) == 0) return "./portablemc-cli";
    return "portablemc";
}

static void on_login_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = (AppData *)data;
    if (app->is_running) return;

    const char *dir = gtk_entry_get_text(GTK_ENTRY(app->entry_dir));
    mkdir_p(dir);

    append_log(app, "Starting Microsoft authentication...\n");
    system("xdg-open \"https://microsoft.com\" &");

    char *argv[] = { (char *)resolve_pmc_bin(), "--main-dir", (char *)dir, "auth", "login", "--no-browser", NULL };
    start_cli_command(app, argv);
}

static void on_launch_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = (AppData *)data;
    if (app->is_running) return;

    const char *dir = gtk_entry_get_text(GTK_ENTRY(app->entry_dir));
    const char *user = gtk_entry_get_text(GTK_ENTRY(app->entry_user));
    int auth = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_auth));
    gchar *loader = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_loader));
    gchar *ver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));

    mkdir_p(dir);

    char full_ver[128];
    if (loader && strcmp(loader, "Vanilla") != 0) {
        snprintf(full_ver, sizeof(full_ver), "%s:%s", loader, ver ? ver : "1.21.4");
    } else {
        snprintf(full_ver, sizeof(full_ver), "%s", ver ? ver : "1.21.4");
    }

    char *argv[12];
    int argc = 0;
    argv[argc++] = (char *)resolve_pmc_bin();
    argv[argc++] = "--main-dir";
    argv[argc++] = (char *)dir;
    argv[argc++] = "start";

    if (auth == 1) {
        argv[argc++] = "-a";
        if (user && strlen(user) > 0) {
            argv[argc++] = "-u";
            argv[argc++] = (char *)user;
        }
    } else {
        argv[argc++] = "-u";
        argv[argc++] = (char *)((user && strlen(user) > 0) ? user : "Player");
    }

    argv[argc++] = full_ver;
    argv[argc] = NULL;

    append_log(app, "Starting process...\n");
    start_cli_command(app, argv);

    g_free(loader);
    g_free(ver);
}

static gboolean on_cancel_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    (void)widget; (void)event;
    AppData *app = (AppData *)data;
    if (app->is_running && app->child_pid > 0) {
        app->was_cancelled = TRUE;
        kill(app->child_pid, SIGTERM);
    }
    return TRUE;
}
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData app = { 0 };
    app.css_provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(app.css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "PortableMC");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 520, 440);
    gtk_window_set_resizable(GTK_WINDOW(app.window), FALSE);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    if (access("icon.png", F_OK) == 0) {
        gtk_window_set_icon_from_file(GTK_WINDOW(app.window), "icon.png", NULL);
    }

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    /* Header */
    GtkWidget *hbox_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), "<span size='x-large' weight='bold'>PortableMC</span>");
    gtk_box_pack_start(GTK_BOX(hbox_top), lbl_title, FALSE, FALSE, 0);

    app.status_dot = gtk_drawing_area_new();
    gtk_widget_set_size_request(app.status_dot, 14, 14);
    gtk_widget_set_valign(app.status_dot, GTK_ALIGN_CENTER);
    g_signal_connect(app.status_dot, "draw", G_CALLBACK(on_draw_status_dot), &app);
    gtk_box_pack_start(GTK_BOX(hbox_top), app.status_dot, FALSE, FALSE, 2);

    app.btn_theme = gtk_button_new_with_label("Light Theme");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_theme), "pill");
    g_signal_connect(app.btn_theme, "clicked", G_CALLBACK(on_toggle_theme), &app);
    gtk_box_pack_end(GTK_BOX(hbox_top), app.btn_theme, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_top, FALSE, FALSE, 0);

    /* Row 1: Auth + User + Login */
    GtkWidget *hbox_auth = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    app.combo_auth = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app.combo_auth), "Username");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app.combo_auth), "Microsoft");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app.combo_auth), 0);
    gtk_widget_set_size_request(app.combo_auth, 130, -1);
    g_signal_connect(app.combo_auth, "changed", G_CALLBACK(on_auth_changed), &app);
    gtk_box_pack_start(GTK_BOX(hbox_auth), app.combo_auth, FALSE, FALSE, 0);

    app.entry_user = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app.entry_user), "Enter offline username...");
    gtk_box_pack_start(GTK_BOX(hbox_auth), app.entry_user, TRUE, TRUE, 0);

    app.btn_login = gtk_button_new_with_label("Login");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_login), "pill");
    g_signal_connect(app.btn_login, "clicked", G_CALLBACK(on_login_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_auth), app.btn_login, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_auth, FALSE, FALSE, 0);

    /* Row 2: Mod Loader */
    GtkWidget *hbox_loader = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_loader = gtk_label_new("Mod Loader");
    gtk_widget_set_size_request(lbl_loader, 130, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl_loader), 0.0);
    gtk_box_pack_start(GTK_BOX(hbox_loader), lbl_loader, FALSE, FALSE, 0);

    app.combo_loader = gtk_combo_box_text_new();
    const char *loaders[] = { "Vanilla", "fabric", "forge", "neoforge", "quilt", NULL };
    for (int i = 0; loaders[i]; i++) gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app.combo_loader), loaders[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(app.combo_loader), 0);
    gtk_box_pack_start(GTK_BOX(hbox_loader), app.combo_loader, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_loader, FALSE, FALSE, 0);

    /* Row 3: Version + Snapshot + Beta */
    GtkWidget *hbox_ver = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_ver = gtk_label_new("Version");
    gtk_widget_set_size_request(lbl_ver, 130, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl_ver), 0.0);
    gtk_box_pack_start(GTK_BOX(hbox_ver), lbl_ver, FALSE, FALSE, 0);

    app.combo_version = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(hbox_ver), app.combo_version, TRUE, TRUE, 0);

    app.chk_snap = gtk_check_button_new_with_label("Snapshot");
    app.chk_beta = gtk_check_button_new_with_label("Beta");
    g_signal_connect_swapped(app.chk_snap, "toggled", G_CALLBACK(populate_versions), &app);
    g_signal_connect_swapped(app.chk_beta, "toggled", G_CALLBACK(populate_versions), &app);
    gtk_box_pack_start(GTK_BOX(hbox_ver), app.chk_snap, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ver), app.chk_beta, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_ver, FALSE, FALSE, 0);

    populate_versions(&app);

    /* Row 4: Game Folder + Browse + Mod + Skin + Cape */
    GtkWidget *hbox_dir = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *lbl_dir = gtk_label_new("Game folder");
    gtk_widget_set_size_request(lbl_dir, 130, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl_dir), 0.0);
    gtk_box_pack_start(GTK_BOX(hbox_dir), lbl_dir, FALSE, FALSE, 0);

    char default_mc[1024];
    const char *home = getenv("HOME");
    snprintf(default_mc, sizeof(default_mc), "%s/.minecraft", home ? home : ".");
    app.entry_dir = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app.entry_dir), default_mc);
    gtk_box_pack_start(GTK_BOX(hbox_dir), app.entry_dir, TRUE, TRUE, 0);

    GtkWidget *btn_browse = gtk_button_new_with_label("Browse");
    GtkWidget *btn_mod    = gtk_button_new_with_label("+ Mod");
    GtkWidget *btn_skin   = gtk_button_new_with_label("+ Skin");
    GtkWidget *btn_cape   = gtk_button_new_with_label("+ Cape");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_browse), "pill");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_mod), "pill");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_skin), "pill");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_cape), "pill");

    g_signal_connect(btn_browse, "clicked", G_CALLBACK(on_browse_dir), &app);
    g_signal_connect(btn_mod, "clicked", G_CALLBACK(on_import_mod), &app);
    g_signal_connect(btn_skin, "clicked", G_CALLBACK(on_import_skin), &app);
    g_signal_connect(btn_cape, "clicked", G_CALLBACK(on_import_cape), &app);

    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_browse, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_mod, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_skin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_cape, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_dir, FALSE, FALSE, 0);

    /* Log Area */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app.text_log = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app.text_log), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app.text_log), GTK_WRAP_WORD_CHAR);
    app.log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app.text_log));
    append_log(&app, "Ready.\n");
    gtk_container_add(GTK_CONTAINER(scroll), app.text_log);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* Action Stack: Launch Button / Progress Bar */
    app.stack_action = gtk_stack_new();
    app.btn_launch = gtk_button_new_with_label("LAUNCH MINECRAFT");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.btn_launch), "launch");
    g_signal_connect(app.btn_launch, "clicked", G_CALLBACK(on_launch_clicked), &app);
    gtk_stack_add_named(GTK_STACK(app.stack_action), app.btn_launch, "launch");

    GtkWidget *ev_box = gtk_event_box_new();
    app.progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(app.progress_bar), TRUE);
    gtk_container_add(GTK_CONTAINER(ev_box), app.progress_bar);
    g_signal_connect(ev_box, "button-press-event", G_CALLBACK(on_cancel_clicked), &app);
    gtk_stack_add_named(GTK_STACK(app.stack_action), ev_box, "progress");

    gtk_box_pack_start(GTK_BOX(vbox), app.stack_action, FALSE, FALSE, 0);

    apply_theme(&app);
    gtk_widget_show_all(app.window);
    gtk_widget_hide(app.btn_login);

    gtk_main();
    return 0;
}
