#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>
#include "modrinth.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *main_stack;
    GtkWidget *intro_area;
    int intro_frame;
    GtkWidget *status_dot;
    GtkWidget *btn_theme;
    GtkWidget *combo_auth;
    GtkWidget *entry_user;
    GtkWidget *btn_login;
    GtkWidget *combo_loader;
    GtkWidget *combo_version;
    GtkWidget *chk_snap;
    GtkWidget *chk_beta;
    GtkWidget *chk_instance;
    GtkWidget *chk_autoclose;
    GtkWidget *entry_dir;
    GtkWidget *text_log;
    GtkWidget *btn_launch;
    GtkWidget *progress_bar;
    GtkCssProvider *css_provider;

    GPid child_pid;
    gboolean is_dark;
    gboolean is_running;
    gboolean was_cancelled;
    gboolean is_game_launch;
    int status_state; /* 0 = green, 1 = orange, 2 = red */
} AppData;

static const char *ALL_VERSIONS[] = {
    "26.3", "26.2", "26.1",
    "1.21.5", "1.21.4", "1.21.3", "1.21.2", "1.21.1", "1.21",
    "1.20.6", "1.20.5", "1.20.4", "1.20.3", "1.20.2", "1.20.1", "1.20",
    "1.19.4", "1.19.3", "1.19.2", "1.19.1", "1.19",
    "1.18.2", "1.18.1", "1.18",
    "1.17.1", "1.17",
    "1.16.5", "1.16.4", "1.16.3", "1.16.2", "1.16.1", "1.16",
    "1.15.2", "1.15.1", "1.15",
    "1.14.4", "1.14.3", "1.14.2", "1.14.1", "1.14",
    "1.13.2", "1.13.1", "1.13",
    "1.12.2", "1.12.1", "1.12",
    "1.11.2", "1.11.1", "1.11",
    "1.10.2", "1.10.1", "1.10",
    "1.9.4", "1.9.3", "1.9.2", "1.9.1", "1.9",
    "1.8.9", "1.8.8", "1.8.7", "1.8.6", "1.8.5", "1.8.4", "1.8.3", "1.8.2", "1.8.1", "1.8",
    "1.7.10", "1.7.9", "1.7.8", "1.7.7", "1.7.6", "1.7.5", "1.7.4", "1.7.2",
    "1.6.4", "1.6.2", "1.6.1",
    "1.5.2", "1.5.1",
    "1.4.7", "1.4.6", "1.4.5", "1.4.4", "1.4.2",
    "1.3.2", "1.3.1",
    "1.2.5", "1.2.4", "1.2.3", "1.2.2", "1.2.1",
    "1.1", "1.0",
    "26.4-rc1", "26.4-pre2", "26.4-pre1", "26w18a", "26w16a", "26w14a", "26w10a", "26w06a", "26w02a",
    "25w46a", "25w44a", "25w41a", "25w35a", "25w20a", "25w14craftmine", "25w08a", "25w04a",
    "1.21.4-rc3", "1.21.4-pre1", "24w46a", "24w45a", "24w44a", "24w40a", "24w33a",
    "24w14potato", "24w12a", "24w09a", "24w03a",
    "23w51b", "23w45a", "23w31a", "23w13a_or_b", "23w07a",
    "22w46a", "22w24a", "22w13oneblockatatime", "22w11a",
    "21w44a", "21w37a", "21w14a", "21w03a",
    "20w51a", "20w45a", "20w14infinite", "20w06a",
    "19w46b", "19w34a", "19w13b", "19w02a",
    "18w50a", "18w43c", "18w30b", "18w16a", "18w01a",
    "17w50a", "17w43b", "17w18b", "17w06a",
    "16w50a", "16w39c", "16w20a", "16w02a",
    "15w51b", "15w41b", "15w31c", "15w14a",
    "14w34d", "14w25b", "14w11b", "14w02c",
    "13w48b", "13w36b", "13w24b", "13w16b", "13w01b",
    "12w50b", "12w34b", "12w21b", "12w07b", "11w49a",
    "b1.8.1", "b1.8", "b1.7.3", "b1.7.2", "b1.7",
    "b1.6.6", "b1.6.5", "b1.6.4", "b1.6",
    "b1.5_01", "b1.5", "b1.4_01", "b1.4",
    "b1.3_01", "b1.3b", "b1.2_02", "b1.2",
    "b1.1_02", "b1.0_01", "b1.0",
    "a1.2.6", "a1.2.5", "a1.2.3_04", "a1.2.2b", "a1.2.0_02",
    "a1.1.2_01", "a1.1.0", "a1.0.17_04", "a1.0.15", "a1.0.11", "a1.0.4",
    "inf-20100618",
    "c0.30_01c", "c0.0.13a", "c0.0.11a",
    "rd-161348", "rd-160052", "rd-132328", "rd-132211",
    NULL
};

static gboolean is_snapshot_ver(const char *v) {
    return (strchr(v, 'w') != NULL || strstr(v, "pre") != NULL || strstr(v, "rc") != NULL ||
            strstr(v, "craftmine") != NULL || strstr(v, "potato") != NULL || strstr(v, "infinite") != NULL ||
            strstr(v, "oneblock") != NULL);
}

static gboolean is_beta_ver(const char *v) {
    return (*v == 'b' || *v == 'a' || *v == 'c' || strncmp(v, "rd-", 3) == 0 || strncmp(v, "inf-", 4) == 0);
}
static void mkdir_p(const char *path) {
    char *tmp = (char *)calloc(1024, 1);
    if (!tmp) return;
    snprintf(tmp, 1024, "%s", path);
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
    free(tmp);
}

static gboolean copy_file_linux(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return FALSE;
    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return FALSE;
    }
    char *buf = (char *)calloc(8192, 1);
    if (!buf) {
        fclose(in);
        fclose(out);
        return FALSE;
    }
    size_t n;
    while ((n = fread(buf, 1, 8192, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            free(buf);
            fclose(in);
            fclose(out);
            return FALSE;
        }
    }
    free(buf);
    fclose(in);
    fclose(out);
    return TRUE;
}

static void get_effective_game_dir(AppData *app, char *outDir, size_t maxLen) {
    const char *base = gtk_entry_get_text(GTK_ENTRY(app->entry_dir));
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_instance))) {
        gchar *ver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));
        snprintf(outDir, maxLen, "%s/instances/%s", base, ver ? ver : "26.3");
        g_free(ver);
    } else {
        snprintf(outDir, maxLen, "%s", base);
    }
}

static void save_config_linux(AppData *app) {
    const char *home = getenv("HOME");
    if (!home) return;
    char *path = (char *)calloc(512, 1);
    if (!path) return;
    snprintf(path, 512, "%s/.portablemc_gui.conf", home);
    FILE *f = fopen(path, "w");
    free(path);
    if (!f) return;

    gchar *ver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));
    fprintf(f, "dark=%d\n", app->is_dark ? 1 : 0);
    fprintf(f, "auth=%d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_auth)));
    fprintf(f, "user=%s\n", gtk_entry_get_text(GTK_ENTRY(app->entry_user)));
    fprintf(f, "loader=%d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_loader)));
    fprintf(f, "ver=%s\n", ver ? ver : "26.3");
    fprintf(f, "dir=%s\n", gtk_entry_get_text(GTK_ENTRY(app->entry_dir)));
    fprintf(f, "instance=%d\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_instance)) ? 1 : 0);
    fprintf(f, "autoclose=%d\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_autoclose)) ? 1 : 0);
    g_free(ver);
    fclose(f);
}

static void load_config_linux(AppData *app) {
    const char *home = getenv("HOME");
    if (!home) return;
    char *path = (char *)calloc(512, 1);
    if (!path) return;
    snprintf(path, 512, "%s/.portablemc_gui.conf", home);
    FILE *f = fopen(path, "r");
    free(path);
    if (!f) return;

    char *line = (char *)calloc(1024, 1);
    if (!line) { fclose(f); return; }
    while (fgets(line, 1024, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "dark=", 5) == 0) app->is_dark = (atoi(line + 5) == 1);
        else if (strncmp(line, "auth=", 5) == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_auth), atoi(line + 5));
        else if (strncmp(line, "user=", 5) == 0) gtk_entry_set_text(GTK_ENTRY(app->entry_user), line + 5);
        else if (strncmp(line, "loader=", 7) == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_loader), atoi(line + 7));
        else if (strncmp(line, "dir=", 4) == 0 && strlen(line + 4) > 0) gtk_entry_set_text(GTK_ENTRY(app->entry_dir), line + 4);
        else if (strncmp(line, "instance=", 9) == 0) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->chk_instance), atoi(line + 9) == 1);
        else if (strncmp(line, "autoclose=", 10) == 0) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->chk_autoclose), atoi(line + 10) == 1);
    }
    free(line);
    fclose(f);
}

static void apply_theme(AppData *app) {
    const char *css = app->is_dark ?
        "window { background-color: #18181c; color: #ffffff; }"
        "label { color: #ffffff; }"
        "entry, textview, text { background-color: #242429; color: #ffffff; border-radius: 6px; }"
        "button { background-color: #2c2c32; color: #ffffff; border-radius: 6px; border: 1px solid #3c3c44; }"
        "button#launch_btn { background-color: #0a84ff; color: #ffffff; font-weight: bold; }"
        :
        "window { background-color: #f2f2f7; color: #1c1c1e; }"
        "label { color: #1c1c1e; }"
        "entry, textview, text { background-color: #ffffff; color: #1c1c1e; border-radius: 6px; }"
        "button { background-color: #e5e5ea; color: #1c1c1e; border-radius: 6px; border: 1px solid #d1d1d6; }"
        "button#launch_btn { background-color: #007aff; color: #ffffff; font-weight: bold; }";

    gtk_css_provider_load_from_data(app->css_provider, css, -1, NULL);
    gtk_button_set_label(GTK_BUTTON(app->btn_theme), app->is_dark ? "Dark Theme" : "Light Theme");
    gtk_widget_queue_draw(app->status_dot);
}

static gboolean on_draw_status_dot(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app = (AppData *)data;
    if (app->status_state == 0) cairo_set_source_rgb(cr, 0.20, 0.78, 0.35);
    else if (app->status_state == 1) cairo_set_source_rgb(cr, 1.0, 0.58, 0.0);
    else cairo_set_source_rgb(cr, 1.0, 0.23, 0.19);

    cairo_arc(cr, 6.0, 6.0, 5.0, 0, 2 * G_PI);
    cairo_fill(cr);
    return FALSE;
}

static void append_log(AppData *app, const char *text) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->text_log));
    if (gtk_text_buffer_get_char_count(buf) > 25000) {
        gtk_text_buffer_set_text(buf, "", 0);
    }
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_insert(buf, &end, text, -1);

    GtkTextMark *mark = gtk_text_buffer_get_insert(buf);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(app->text_log), mark);
}

static void show_apple_alert(AppData *app, const char *title, const char *msg, const char *folder_path) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_NONE,
                                               "%s", title);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", GTK_RESPONSE_OK);
    if (folder_path) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "Open Folder", GTK_RESPONSE_ACCEPT);
    }

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (res == GTK_RESPONSE_ACCEPT && folder_path) {
        char *cmd = (char *)calloc(1024, 1);
        if (cmd) {
            snprintf(cmd, 1024, "xdg-open \"%s\" &", folder_path);
            system(cmd);
            free(cmd);
        }
    }
}
static void populate_versions(AppData *app) {
    int show_snap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_snap)) ? 1 : 0;
    int show_beta = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_beta)) ? 1 : 0;

    gchar *prev_sel = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(app->combo_version));

    char (*dyn_versions)[64] = calloc(512, sizeof(*dyn_versions));
    int fetched = 0;
    if (dyn_versions) {
        fetched = FetchMojangVersions(dyn_versions, 512, show_snap, show_beta);
    }

    int active_idx = 0;
    int count = 0;

    if (fetched > 0) {
        for (int i = 0; i < fetched; i++) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_version), dyn_versions[i]);
            if (prev_sel && strcmp(dyn_versions[i], prev_sel) == 0) {
                active_idx = count;
            }
            count++;
        }
    } else {
        /* Fallback offline se il PC non è connesso */
        for (int i = 0; ALL_VERSIONS[i] != NULL; i++) {
            const char *v = ALL_VERSIONS[i];
            if (is_snapshot_ver(v) && !show_snap) continue;
            if (is_beta_ver(v) && !show_beta) continue;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_version), v);
            if (prev_sel && strcmp(v, prev_sel) == 0) {
                active_idx = count;
            }
            count++;
        }
    }

    if (dyn_versions) free(dyn_versions);
    if (prev_sel) g_free(prev_sel);

    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_version), active_idx);
}

static void on_theme_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    app->is_dark = !app->is_dark;
    apply_theme(app);
}

static void on_auth_changed(GtkComboBox *widget, gpointer data) {
    AppData *app = (AppData *)data;
    int idx = gtk_combo_box_get_active(widget);
    if (idx == 1) {
        gtk_widget_show(app->btn_login);
        gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry_user), "Optional Microsoft nickname...");
    } else {
        gtk_widget_hide(app->btn_login);
        gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry_user), "Enter offline username...");
    }
}

static void on_browse_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Game Folder",
                                                    GTK_WINDOW(app->window),
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Select", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(app->entry_dir), folder);
        g_free(folder);
    }
    gtk_widget_destroy(dialog);
}

static void on_boost_fps_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    if (gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_loader)) == 0) {
        show_apple_alert(app, "Select a Mod Loader",
                         "Boost FPS requires a Mod Loader (select 'fabric' recommended, or forge/neoforge/quilt).",
                         NULL);
        return;
    }

    char *base_dir = (char *)calloc(1024, 1);
    char *mods_dir = (char *)calloc(1024, 1);
    char *msg = (char *)calloc(512, 1);
    if (!base_dir || !mods_dir || !msg) { free(base_dir); free(mods_dir); free(msg); return; }

    get_effective_game_dir(app, base_dir, 1024);
    mkdir_p(base_dir);

    gchar *ldr = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_loader));
    gchar *ver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));

    append_log(app, "[Boost FPS] Installing optimization pack (Sodium, Lithium, FerriteCore, EntityCulling)...\n");
    int count = InstallBoostFPSPack(base_dir, ldr ? ldr : "fabric", ver ? ver : "26.3");

    snprintf(mods_dir, 1024, "%s/mods", base_dir);
    snprintf(msg, 512, "Installed %d optimization mods for %s (%s)!", count, ver ? ver : "26.3", ldr ? ldr : "fabric");
    append_log(app, "[Boost FPS] Done.\n");
    show_apple_alert(app, "Boost FPS Ready", msg, mods_dir);

    g_free(ldr);
    g_free(ver);
    free(base_dir);
    free(mods_dir);
    free(msg);
}

static void on_search_mods_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    if (gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_loader)) == 0) {
        show_apple_alert(app, "Select a Mod Loader",
                         "Please select Fabric, Forge, NeoForge, or Quilt before searching for mods.",
                         NULL);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Search Modrinth Catalog",
                                                    GTK_WINDOW(app->window),
                                                    GTK_DIALOG_MODAL,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Search", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *lbl = gtk_label_new("Enter mod name (e.g. sodium, iris, jei, xaero):");
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Mod name...");
    gtk_box_pack_start(GTK_BOX(content), lbl, FALSE, FALSE, 6);
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 6);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(dialog);
        return;
    }

    char *query = (char *)calloc(256, 1);
    if (!query) { gtk_widget_destroy(dialog); return; }
    snprintf(query, 256, "%s", gtk_entry_get_text(GTK_ENTRY(entry)));
    gtk_widget_destroy(dialog);
    if (strlen(query) == 0) { free(query); return; }

    gchar *ldr = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_loader));
    gchar *ver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));

    char (*slugs)[64] = calloc(8, sizeof(*slugs));
    char (*titles)[128] = calloc(8, sizeof(*titles));
    if (!slugs || !titles) { free(query); free(slugs); free(titles); g_free(ldr); g_free(ver); return; }

    int found = SearchModrinthProjects(query, ldr ? ldr : "fabric", ver ? ver : "26.3", slugs, titles, 8);
    free(query);

    if (found <= 0) {
        show_apple_alert(app, "No Results", "No compatible mods found on Modrinth for this version/loader.", NULL);
        free(slugs);
        free(titles);
        g_free(ldr);
        g_free(ver);
        return;
    }

    GtkWidget *pick_dlg = gtk_dialog_new_with_buttons("Select Mod to Install",
                                                      GTK_WINDOW(app->window),
                                                      GTK_DIALOG_MODAL,
                                                      "_Cancel", GTK_RESPONSE_CANCEL,
                                                      "_Install", GTK_RESPONSE_ACCEPT,
                                                      NULL);
    GtkWidget *p_content = gtk_dialog_get_content_area(GTK_DIALOG(pick_dlg));
    GtkWidget *combo_res = gtk_combo_box_text_new();
    for (int i = 0; i < found; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_res), titles[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_res), 0);
    gtk_box_pack_start(GTK_BOX(p_content), combo_res, FALSE, FALSE, 8);
    gtk_widget_show_all(pick_dlg);

    if (gtk_dialog_run(GTK_DIALOG(pick_dlg)) == GTK_RESPONSE_ACCEPT) {
        int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_res));
        if (idx >= 0 && idx < found) {
            char *base_dir = (char *)calloc(1024, 1);
            char *mods_dir = (char *)calloc(1024, 1);
            char *msg = (char *)calloc(512, 1);
            if (base_dir && mods_dir && msg) {
                get_effective_game_dir(app, base_dir, 1024);
                mkdir_p(base_dir);
                snprintf(mods_dir, 1024, "%s/mods", base_dir);

                int ok = DownloadModFromModrinth(slugs[idx], base_dir, ldr ? ldr : "fabric", ver ? ver : "26.3");
                if (ok) {
                    snprintf(msg, 512, "'%s' was downloaded and installed!", titles[idx]);
                    show_apple_alert(app, "Mod Installed", msg, mods_dir);
                } else {
                    show_apple_alert(app, "Download Failed", "Could not download the selected mod.", NULL);
                }
            }
            free(base_dir);
            free(mods_dir);
            free(msg);
        }
    }
    gtk_widget_destroy(pick_dlg);
    free(slugs);
    free(titles);
    g_free(ldr);
    g_free(ver);
}
static void on_mod_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    char *base_dir = (char *)calloc(1024, 1);
    if (!base_dir) return;
    get_effective_game_dir(app, base_dir, 1024);
    if (strlen(base_dir) == 0) {
        free(base_dir);
        show_apple_alert(app, "Missing Game Folder", "Please select a destination game folder first.", NULL);
        return;
    }

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Minecraft Mod (.jar)",
                                                    GTK_WINDOW(app->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Import", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Minecraft Mods (*.jar)");
    gtk_file_filter_add_pattern(filter, "*.jar");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *src = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *mods_dir = (char *)calloc(1024, 1);
        char *dst = (char *)calloc(1024, 1);
        if (mods_dir && dst) {
            snprintf(mods_dir, 1024, "%s/mods", base_dir);
            mkdir_p(mods_dir);

            char *base_name = g_path_get_basename(src);
            snprintf(dst, 1024, "%s/%s", mods_dir, base_name);

            if (copy_file_linux(src, dst)) {
                show_apple_alert(app, "Mod Imported", "The .jar mod was copied to your mods folder.", mods_dir);
            } else {
                show_apple_alert(app, "Import Error", "Failed to copy the .jar mod file.", NULL);
            }
            g_free(base_name);
        }
        free(mods_dir);
        free(dst);
        g_free(src);
    }
    gtk_widget_destroy(dialog);
    free(base_dir);
}

static gboolean has_custom_skin_loader(const char *mods_dir) {
    DIR *d = opendir(mods_dir);
    if (!d) return FALSE;
    struct dirent *dir;
    gboolean found = FALSE;
    while ((dir = readdir(d)) != NULL) {
        if (g_ascii_strncasecmp(dir->d_name, "customskinloader", 16) == 0 &&
            g_str_has_suffix(dir->d_name, ".jar")) {
            found = TRUE;
            break;
        }
    }
    closedir(d);
    return found;
}

static void import_skin_or_cape(AppData *app, gboolean is_cape) {
    if (gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_auth)) == 1) {
        char *mc_cmd = (char *)calloc(256, 1);
        if (mc_cmd) {
            snprintf(mc_cmd, 256, "xdg-open \"https://www%cminecraft%cnet\" &", '.', '.');
            system(mc_cmd);
            free(mc_cmd);
        }
        return;
    }

    if (gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_loader)) == 0) {
        show_apple_alert(app, "Vanilla Not Supported",
                         "Offline skins and capes cannot be applied on Vanilla.\n\nPlease select Fabric, Forge, NeoForge, or Quilt first.",
                         NULL);
        return;
    }

    char *base_dir = (char *)calloc(1024, 1);
    if (!base_dir) return;
    get_effective_game_dir(app, base_dir, 1024);
    if (strlen(base_dir) == 0) {
        free(base_dir);
        show_apple_alert(app, "Missing Game Folder", "Please select a destination game folder first.", NULL);
        return;
    }

    const char *user = gtk_entry_get_text(GTK_ENTRY(app->entry_user));
    if (strlen(user) == 0) user = "Steve";

    GtkWidget *dialog = gtk_file_chooser_dialog_new(is_cape ? "Select Cape Texture (.png)" : "Select Skin Texture (.png)",
                                                    GTK_WINDOW(app->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Import", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "PNG Textures (*.png)");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *src = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *target_dir = (char *)calloc(1024, 1);
        char *dst_png = (char *)calloc(1024, 1);
        if (target_dir && dst_png) {
            snprintf(target_dir, 1024, "%s/CustomSkinLoader/LocalSkin/%s", base_dir, is_cape ? "capes" : "skins");
            mkdir_p(target_dir);
            snprintf(dst_png, 1024, "%s/%s.png", target_dir, user);

            if (copy_file_linux(src, dst_png)) {
                char *json_path = (char *)calloc(1024, 1);
                if (json_path) {
                    snprintf(json_path, 1024, "%s/CustomSkinLoader/CustomSkinLoader.json", base_dir);
                    if (access(json_path, F_OK) != 0) {
                        FILE *fj = fopen(json_path, "w");
                        if (fj) {
                            fprintf(fj, "{\n"
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
                    free(json_path);
                }

                char *mods_dir = (char *)calloc(1024, 1);
                char *final_msg = (char *)calloc(512, 1);
                if (mods_dir && final_msg) {
                    snprintf(mods_dir, 1024, "%s/mods", base_dir);
                    mkdir_p(mods_dir);

                    const char *status_note = "CustomSkinLoader is already installed in your mods folder.";
                    if (!has_custom_skin_loader(mods_dir)) {
                        gchar *loader_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_loader));
                        gchar *ver_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));
                        int ok = DownloadModFromModrinth("customskinloader", base_dir,
                                                         loader_str ? loader_str : "fabric",
                                                         ver_str ? ver_str : "26.3");
                        if (ok) {
                            status_note = "CustomSkinLoader was automatically downloaded to your mods folder!";
                        } else {
                            status_note = "Could not auto-download CustomSkinLoader. Please import the .jar manually via '+ Mod'.";
                        }
                        g_free(loader_str);
                        g_free(ver_str);
                    }

                    snprintf(final_msg, 512, "Texture saved for '%s'!\n\n%s", user, status_note);
                    show_apple_alert(app, is_cape ? "Cape Imported" : "Skin Imported", final_msg, target_dir);
                }
                free(mods_dir);
                free(final_msg);
            } else {
                show_apple_alert(app, "Import Error", "Failed to copy the .png file.", NULL);
            }
        }
        free(target_dir);
        free(dst_png);
        g_free(src);
    }
    gtk_widget_destroy(dialog);
    free(base_dir);
}

static void on_skin_clicked(GtkWidget *w, gpointer data) { import_skin_or_cape((AppData *)data, FALSE); }
static void on_cape_clicked(GtkWidget *w, gpointer data) { import_skin_or_cape((AppData *)data, TRUE); }
static gboolean on_child_stdout(GIOChannel *source, GIOCondition cond, gpointer data) {
    AppData *app = (AppData *)data;
    if (cond & G_IO_HUP) return FALSE;

    gchar *line = NULL;
    gsize len = 0;
    GError *err = NULL;
    GIOStatus status = g_io_channel_read_line(source, &line, &len, NULL, &err);
    if (status == G_IO_STATUS_NORMAL && line) {
        append_log(app, line);
        if (app->is_game_launch && (strstr(line, "Launching ") != NULL || strstr(line, "Setting user:") != NULL)) {
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->chk_autoclose))) {
                save_config_linux(app);
                g_free(line);
                gtk_main_quit();
                return FALSE;
            }
        }
        g_free(line);
    }
    if (err) g_error_free(err);
    return TRUE;
}

static void on_child_watch(GPid pid, gint status, gpointer data) {
    AppData *app = (AppData *)data;
    g_spawn_close_pid(pid);
    app->is_running = FALSE;
    app->child_pid = 0;

    gtk_widget_hide(app->progress_bar);
    gtk_button_set_label(GTK_BUTTON(app->btn_launch), "LAUNCH MINECRAFT");

    if (app->was_cancelled) {
        app->status_state = 2;
        append_log(app, "\n--- Process cancelled by user ---\n");
        char *eff_dir = (char *)calloc(1024, 1);
        if (eff_dir) {
            get_effective_game_dir(app, eff_dir, 1024);
            show_apple_alert(app, "Download Cancelled",
                             "The process was stopped.\nCheck your game folder if a download was interrupted.",
                             eff_dir);
            free(eff_dir);
        }
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        app->status_state = 0;
        append_log(app, "\n--- Process finished ---\n");
    } else {
        app->status_state = 2;
        append_log(app, "\n--- Process exited with errors ---\n");
    }
    gtk_widget_queue_draw(app->status_dot);
}

static gboolean pulse_progress(gpointer data) {
    AppData *app = (AppData *)data;
    if (!app->is_running) return FALSE;
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(app->progress_bar));
    return TRUE;
}

static void run_pmc_command(AppData *app, gchar **argv, gboolean is_game_launch) {
    GError *err = NULL;
    gint out_fd = -1;

    app->is_running = TRUE;
    app->was_cancelled = FALSE;
    app->is_game_launch = is_game_launch;
    app->status_state = 1;
    gtk_widget_queue_draw(app->status_dot);

    gtk_button_set_label(GTK_BUTTON(app->btn_launch), "RUNNING... (Click to Cancel)");
    gtk_widget_show(app->progress_bar);
    g_timeout_add(80, pulse_progress, app);

    gboolean ok = g_spawn_async_with_pipes(NULL, argv, NULL,
                                           G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                                           NULL, NULL, &app->child_pid,
                                           NULL, &out_fd, NULL, &err);
    if (!ok) {
        app->is_running = FALSE;
        app->status_state = 2;
        gtk_widget_queue_draw(app->status_dot);
        gtk_widget_hide(app->progress_bar);
        gtk_button_set_label(GTK_BUTTON(app->btn_launch), "LAUNCH MINECRAFT");
        append_log(app, "[ERROR] Could not run 'portablemc'. Install it via 'pip install portablemc' or place it in PATH.\n");
        if (err) g_error_free(err);
        return;
    }

    g_child_watch_add(app->child_pid, on_child_watch, app);

    GIOChannel *channel = g_io_channel_unix_new(out_fd);
    g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);
    g_io_channel_set_close_on_unref(channel, TRUE);
    g_io_add_watch(channel, G_IO_IN | G_IO_HUP, on_child_stdout, app);
    g_io_channel_unref(channel);
}

static void on_login_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    if (app->is_running) return;

    char *dir = (char *)calloc(1024, 1);
    if (!dir) return;
    get_effective_game_dir(app, dir, 1024);
    mkdir_p(dir);

    append_log(app, "Starting Microsoft authentication...\n");
    char *login_cmd = (char *)calloc(256, 1);
    if (login_cmd) {
        snprintf(login_cmd, 256, "xdg-open \"https://www%cmicrosoft%ccom/link\" &", '.', '.');
        system(login_cmd);
        free(login_cmd);
    }

    gchar **argv = g_new0(gchar *, 7);
    int idx = 0;
    argv[idx++] = "portablemc";
    argv[idx++] = "--main-dir";
    argv[idx++] = dir;
    argv[idx++] = "auth";
    argv[idx++] = "login";
    argv[idx++] = "--no-browser";
    argv[idx++] = NULL;

    run_pmc_command(app, argv, FALSE);
    g_free(argv);
    free(dir);
}

static void on_launch_clicked(GtkWidget *w, gpointer data) {
    AppData *app = (AppData *)data;
    if (app->is_running && app->child_pid > 0) {
        app->was_cancelled = TRUE;
        kill(app->child_pid, SIGTERM);
        return;
    }

    save_config_linux(app);

    char *dir = (char *)calloc(1024, 1);
    char *full_ver = (char *)calloc(256, 1);
    if (!dir || !full_ver) { free(dir); free(full_ver); return; }

    get_effective_game_dir(app, dir, 1024);
    const char *user = gtk_entry_get_text(GTK_ENTRY(app->entry_user));
    gchar *loader = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_loader));
    gchar *ver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_version));

    mkdir_p(dir);

    if (!loader || strcmp(loader, "Vanilla") == 0) {
        snprintf(full_ver, 256, "%s", ver ? ver : "26.3");
    } else {
        snprintf(full_ver, 256, "%s:%s", loader, ver ? ver : "26.3");
    }

    int auth_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_auth));
    gchar **argv = g_new0(gchar *, 12);
    int idx = 0;
    argv[idx++] = "portablemc";
    argv[idx++] = "--main-dir";
    argv[idx++] = dir;
    argv[idx++] = "start";

    if (auth_idx == 1) {
        argv[idx++] = "-a";
        if (strlen(user) > 0) {
            argv[idx++] = "-u";
            argv[idx++] = (gchar *)user;
        }
    } else {
        argv[idx++] = "-u";
        argv[idx++] = (strlen(user) > 0) ? (gchar *)user : "Player";
    }
    argv[idx++] = full_ver;
    argv[idx++] = NULL;

    append_log(app, "Starting process...\n");
    run_pmc_command(app, argv, TRUE);

    g_free(argv);
    g_free(loader);
    g_free(ver);
    free(dir);
    free(full_ver);
}
static void on_filter_toggled(GtkToggleButton *btn, gpointer data) {
    populate_versions((AppData *)data);
}

/* --- ANIMAZIONE INIZIALE CUBO ISOMETRICO IN CAIRO --- */
static void draw_cairo_quad(cairo_t *cr, double x0, double y0, double x2, double y2,
                            double x1, double y1, double x3, double y3,
                            double r1, double g1, double b1, double r2, double g2, double b2, double alpha) {
    cairo_new_path(cr);
    cairo_move_to(cr, x0, y0);
    cairo_line_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_line_to(cr, x3, y3);
    cairo_close_path(cr);

    cairo_pattern_t *pat = cairo_pattern_create_linear(x0, y0, x2, y2);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, r1, g1, b1, alpha);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, r2, g2, b2, alpha);
    cairo_set_source(cr, pat);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}

static gboolean on_draw_intro(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app = (AppData *)data;
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    double cx = w / 2.0;
    double cy = h / 2.0 - 10.0;

    float t = (float)app->intro_frame / 95.0f;
    double alpha = (t < 0.82f) ? 1.0 : (1.0 - (t - 0.82f) / 0.18f);
    if (alpha < 0.0) alpha = 0.0;

    if (t < 0.52f) {
        float pT = t / 0.52f;
        float spread = sinf(pT * 3.14159265f);
        float ease = 1.0f - (1.0f - spread) * (1.0f - spread);
        for (int i = 0; i < 12; i++) {
            float angle = (6.2831853f / 12.0f) * i + pT * 2.2f;
            float dist = ease * (55.0f + (i % 3) * 18.0f);
            double dx = cx + cosf(angle) * dist;
            double dy = cy + sinf(angle) * dist;
            double r = 5.0 + 4.0 * (1.0 - ease);
            if (i % 2 == 0) cairo_set_source_rgba(cr, 0.0, 0.55, 1.0, alpha);
            else cairo_set_source_rgba(cr, 0.20, 0.78, 0.43, alpha);
            cairo_arc(cr, dx, dy, r, 0, 2 * G_PI);
            cairo_fill(cr);
        }
    }

    if (t > 0.28f) {
        float cT = (t - 0.28f) / 0.30f;
        if (cT > 1.0f) cT = 1.0f;
        float scale = (cT < 0.5f) ? (2.0f * cT * cT) : (1.0f - powf(-2.0f * cT + 2.0f, 2.0f) / 2.0f);
        double S = 48.0 * scale;
        double dx = S * 0.8660254;
        double dy = S * 0.5;

        draw_cairo_quad(cr, cx, cy - S, cx, cy, cx + dx, cy - dy, cx + dx, cy + dy,
                        0.31, 0.90, 0.47, 0.12, 0.72, 0.72, alpha);
        draw_cairo_quad(cr, cx - dx, cy - dy, cx, cy + S, cx, cy, cx - dx, cy + dy,
                        0.18, 0.78, 0.59, 0.0, 0.43, 1.0, alpha);
        draw_cairo_quad(cr, cx, cy, cx + dx, cy + dy, cx + dx, cy - dy, cx, cy + S,
                        0.10, 0.69, 0.82, 0.06, 0.37, 0.80, alpha);
    }

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 18.0);
    if (app->is_dark) cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
    else cairo_set_source_rgba(cr, 0.11, 0.11, 0.12, alpha);

    cairo_text_extents_t ext;
    cairo_text_extents(cr, "PortableMC", &ext);
    cairo_move_to(cr, cx - ext.width / 2.0, cy + 78.0);
    cairo_show_text(cr, "PortableMC");
    return FALSE;
}

static gboolean on_intro_tick(gpointer data) {
    AppData *app = (AppData *)data;
    app->intro_frame++;
    if (app->intro_frame >= 95) {
        gtk_stack_set_visible_child_name(GTK_STACK(app->main_stack), "ui");
        if (gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_auth)) == 0) {
            gtk_widget_hide(app->btn_login);
        }
        return G_SOURCE_REMOVE;
    }
    gtk_widget_queue_draw(app->intro_area);
    return G_SOURCE_CONTINUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData app = {0};
    app.is_dark = FALSE;
    app.status_state = 0;

    app.css_provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(app.css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "PortableMC v2.0");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 540, 480);
    gtk_window_set_resizable(GTK_WINDOW(app.window), FALSE);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 18);

    /* Header */
    GtkWidget *hbox_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), "<span size='x-large' weight='bold'>PortableMC</span>");
    gtk_box_pack_start(GTK_BOX(hbox_top), lbl_title, FALSE, FALSE, 0);

    app.status_dot = gtk_drawing_area_new();
    gtk_widget_set_size_request(app.status_dot, 14, 14);
    g_signal_connect(app.status_dot, "draw", G_CALLBACK(on_draw_status_dot), &app);
    gtk_box_pack_start(GTK_BOX(hbox_top), app.status_dot, FALSE, FALSE, 4);

    app.btn_theme = gtk_button_new_with_label("Light Theme");
    g_signal_connect(app.btn_theme, "clicked", G_CALLBACK(on_theme_clicked), &app);
    gtk_box_pack_end(GTK_BOX(hbox_top), app.btn_theme, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_top, FALSE, FALSE, 0);

    /* Row 1: Auth + User + Login */
    GtkWidget *hbox_auth = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    app.combo_auth = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app.combo_auth), "Username");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app.combo_auth), "Microsoft");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app.combo_auth), 0);
    g_signal_connect(app.combo_auth, "changed", G_CALLBACK(on_auth_changed), &app);
    gtk_box_pack_start(GTK_BOX(hbox_auth), app.combo_auth, FALSE, FALSE, 0);

    app.entry_user = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app.entry_user), "Enter offline username...");
    gtk_box_pack_start(GTK_BOX(hbox_auth), app.entry_user, TRUE, TRUE, 0);

    app.btn_login = gtk_button_new_with_label("Login");
    g_signal_connect(app.btn_login, "clicked", G_CALLBACK(on_login_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_auth), app.btn_login, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_auth, FALSE, FALSE, 0);

    /* Row 2: Mod Loader */
    GtkWidget *hbox_loader = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_loader = gtk_label_new("Mod Loader");
    gtk_widget_set_size_request(lbl_loader, 110, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl_loader), 0.0);
    gtk_box_pack_start(GTK_BOX(hbox_loader), lbl_loader, FALSE, FALSE, 0);

    app.combo_loader = gtk_combo_box_text_new();
    const char *loaders[] = {"Vanilla", "fabric", "forge", "neoforge", "quilt"};
    for (int i = 0; i < 5; i++) gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app.combo_loader), loaders[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(app.combo_loader), 0);
    gtk_box_pack_start(GTK_BOX(hbox_loader), app.combo_loader, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_loader, FALSE, FALSE, 0);

    /* Row 3: Version + Snapshots + Beta */
    GtkWidget *hbox_ver = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lbl_ver = gtk_label_new("Version");
    gtk_widget_set_size_request(lbl_ver, 110, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl_ver), 0.0);
    gtk_box_pack_start(GTK_BOX(hbox_ver), lbl_ver, FALSE, FALSE, 0);

    app.combo_version = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(hbox_ver), app.combo_version, TRUE, TRUE, 0);

    app.chk_snap = gtk_check_button_new_with_label("Snapshot");
    g_signal_connect(app.chk_snap, "toggled", G_CALLBACK(on_filter_toggled), &app);
    gtk_box_pack_start(GTK_BOX(hbox_ver), app.chk_snap, FALSE, FALSE, 0);

    app.chk_beta = gtk_check_button_new_with_label("Beta");
    g_signal_connect(app.chk_beta, "toggled", G_CALLBACK(on_filter_toggled), &app);
    gtk_box_pack_start(GTK_BOX(hbox_ver), app.chk_beta, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_ver, FALSE, FALSE, 0);

    populate_versions(&app);

    /* Row 4: Game Folder + Browse + Skin + Cape */
    GtkWidget *hbox_dir = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *lbl_dir = gtk_label_new("Game folder");
    gtk_widget_set_size_request(lbl_dir, 110, -1);
    gtk_label_set_xalign(GTK_LABEL(lbl_dir), 0.0);
    gtk_box_pack_start(GTK_BOX(hbox_dir), lbl_dir, FALSE, FALSE, 0);

    app.entry_dir = gtk_entry_new();
    char *def_dir = (char *)calloc(512, 1);
    const char *home = getenv("HOME");
    if (def_dir) {
        snprintf(def_dir, 512, "%s/.minecraft", home ? home : ".");
        gtk_entry_set_text(GTK_ENTRY(app.entry_dir), def_dir);
        free(def_dir);
    }
    gtk_box_pack_start(GTK_BOX(hbox_dir), app.entry_dir, TRUE, TRUE, 0);

    GtkWidget *btn_browse = gtk_button_new_with_label("Browse");
    g_signal_connect(btn_browse, "clicked", G_CALLBACK(on_browse_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_browse, FALSE, FALSE, 0);

    GtkWidget *btn_skin = gtk_button_new_with_label("+ Skin");
    g_signal_connect(btn_skin, "clicked", G_CALLBACK(on_skin_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_skin, FALSE, FALSE, 0);

    GtkWidget *btn_cape = gtk_button_new_with_label("+ Cape");
    g_signal_connect(btn_cape, "clicked", G_CALLBACK(on_cape_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_dir), btn_cape, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox_dir, FALSE, FALSE, 0);

    /* Row 5: Tools (+ Mod, Search Mods, Boost FPS, Isolated, Auto-Close) */
    GtkWidget *hbox_tools = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    GtkWidget *btn_mod = gtk_button_new_with_label("+ Mod");
    g_signal_connect(btn_mod, "clicked", G_CALLBACK(on_mod_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_tools), btn_mod, FALSE, FALSE, 0);

    GtkWidget *btn_search = gtk_button_new_with_label("Search Mods");
    g_signal_connect(btn_search, "clicked", G_CALLBACK(on_search_mods_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_tools), btn_search, FALSE, FALSE, 0);

    GtkWidget *btn_boost = gtk_button_new_with_label("Boost FPS");
    g_signal_connect(btn_boost, "clicked", G_CALLBACK(on_boost_fps_clicked), &app);
    gtk_box_pack_start(GTK_BOX(hbox_tools), btn_boost, FALSE, FALSE, 0);

    app.chk_instance = gtk_check_button_new_with_label("Isolated");
    gtk_box_pack_start(GTK_BOX(hbox_tools), app.chk_instance, FALSE, FALSE, 0);

    app.chk_autoclose = gtk_check_button_new_with_label("Auto-Close");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app.chk_autoclose), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox_tools), app.chk_autoclose, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox_tools, FALSE, FALSE, 0);

    /* Log Console */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    app.text_log = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app.text_log), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(app.text_log), TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), app.text_log);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* Progress Bar & Launch Button */
    app.progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), app.progress_bar, FALSE, FALSE, 0);

    app.btn_launch = gtk_button_new_with_label("LAUNCH MINECRAFT");
    gtk_widget_set_name(app.btn_launch, "launch_btn");
    gtk_widget_set_size_request(app.btn_launch, -1, 44);
    g_signal_connect(app.btn_launch, "clicked", G_CALLBACK(on_launch_clicked), &app);
    gtk_box_pack_start(GTK_BOX(vbox), app.btn_launch, FALSE, FALSE, 0);

    /* Stack per animazione iniziale + UI */
    app.main_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app.main_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_container_add(GTK_CONTAINER(app.window), app.main_stack);

    app.intro_area = gtk_drawing_area_new();
    g_signal_connect(app.intro_area, "draw", G_CALLBACK(on_draw_intro), &app);
    gtk_stack_add_named(GTK_STACK(app.main_stack), app.intro_area, "intro");
    gtk_stack_add_named(GTK_STACK(app.main_stack), vbox, "ui");
    gtk_stack_set_visible_child_name(GTK_STACK(app.main_stack), "intro");

    load_config_linux(&app);
    apply_theme(&app);
    gtk_widget_show_all(app.window);
    gtk_widget_hide(app.btn_login);
    gtk_widget_hide(app.progress_bar);

    g_timeout_add(33, on_intro_tick, &app);
    append_log(&app, "Ready.\n");

    gtk_main();
    return 0;
}
