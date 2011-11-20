#include "commands.h"
#include "uzbl-core.h"
#include "events.h"
#include "util.h"
#include "menu.h"
#include "callbacks.h"
#include "variables.h"
#include "type.h"

/* -- command to callback/function map for things we cannot attach to any signals */
CommandInfo cmdlist[] =
{   /* key                              function      user_data      */
    { "back",                           view_go_back, 0                },
    { "forward",                        view_go_forward, 0             },
    { "scroll",                         scroll_cmd, 0                  },
    { "reload",                         view_reload, 0                 },
    { "reload_ign_cache",               view_reload_bypass_cache, 0    },
    { "stop",                           view_stop_loading, 0           },
    { "zoom_in",                        view_zoom_in, 0                }, //Can crash (when max zoom reached?).
    { "zoom_out",                       view_zoom_out, 0               },
    { "js",                             run_js, 0                      },
    { "script",                         run_external_js, 0             },
    { "spawn",                          spawn_async, 0                 },
    { "sync_spawn",                     spawn_sync, 0                  },
    { "sync_spawn_exec",                spawn_sync_exec, 0             }, // needed for load_cookies.sh :(
    { "sh",                             spawn_sh_async, 0              },
    { "sync_sh",                        spawn_sh_sync, 0               },
    { "exit",                           close_uzbl, 0                  },
    { "search",                         search_forward_text, 0         },
    { "search_reverse",                 search_reverse_text, 0         },
    { "search_clear",                   search_clear, 0                },
    { "dehilight",                      dehilight, 0                   },
    { "toggle",                         toggle_var, 0                  },
    { "dump_config",                    act_dump_config, 0             },
    { "dump_config_as_events",          act_dump_config_as_events, 0   },
    { "chain",                          chain, 0                       },
    { "print",                          print, 0                       },
    { "menu_add",                       menu_add, 0                    },
    { "menu_link_add",                  menu_add_link, 0               },
    { "menu_image_add",                 menu_add_image, 0              },
    { "menu_editable_add",              menu_add_edit, 0               },
    { "menu_separator",                 menu_add_separator, 0          },
    { "menu_link_separator",            menu_add_separator_link, 0     },
    { "menu_image_separator",           menu_add_separator_image, 0    },
    { "menu_editable_separator",        menu_add_separator_edit, 0     },
    { "menu_remove",                    menu_remove, 0                 },
    { "menu_link_remove",               menu_remove_link, 0            },
    { "menu_image_remove",              menu_remove_image, 0           },
    { "menu_editable_remove",           menu_remove_edit, 0            },
    { "hardcopy",                       hardcopy, 0                    },
    { "show_inspector",                 show_inspector, 0              },
    { "add_cookie",                     add_cookie, 0                  },
    { "delete_cookie",                  delete_cookie, 0               },
    { "clear_cookies",                  clear_cookies, 0               },
    { "download",                       download, 0                    }
};

void
commands_hash() {
    unsigned int i;
    uzbl.behave.commands = g_hash_table_new(g_str_hash, g_str_equal);

    for (i = 0; i < LENGTH(cmdlist); i++)
        g_hash_table_insert(uzbl.behave.commands, (gpointer) cmdlist[i].key, &cmdlist[i]);
}

void
builtins() {
    unsigned int i;
    unsigned int len = LENGTH(cmdlist);
    GString*     command_list = g_string_new("");

    for (i = 0; i < len; i++) {
        g_string_append(command_list, cmdlist[i].key);
        g_string_append_c(command_list, ' ');
    }

    send_event(BUILTINS, NULL, TYPE_STR, command_list->str, NULL);
    g_string_free(command_list, TRUE);
}

/* VIEW funcs (little webkit wrappers) */
#define VIEWFUNC(name) void view_##name(WebKitWebView *page, GSList *argv, GString *result){(void)argv; (void)result; webkit_web_view_##name(page);}
VIEWFUNC(reload)
VIEWFUNC(reload_bypass_cache)
VIEWFUNC(stop_loading)
VIEWFUNC(zoom_in)
VIEWFUNC(zoom_out)
VIEWFUNC(go_back)
VIEWFUNC(go_forward)
#undef VIEWFUNC

/*
 * scroll vertical 20
 * scroll vertical 20%
 * scroll vertical -40
 * scroll vertical begin
 * scroll vertical end
 * scroll horizontal 10
 * scroll horizontal -500
 * scroll horizontal begin
 * scroll horizontal end
 */
void
scroll_cmd(WebKitWebView* page, GSList *argv, GString *result) {
    (void) page; (void) result;

    const gchar *direction = argv->data;
    argv = g_slist_next(argv);

    const gchar *argv1     = argv->data;

    GtkAdjustment *bar = NULL;

    if (g_strcmp0(direction, "horizontal") == 0)
        bar = uzbl.gui.bar_h;
    else if (g_strcmp0(direction, "vertical") == 0)
        bar = uzbl.gui.bar_v;
    else {
        if(uzbl.state.verbose)
            puts("Unrecognized scroll format");
        return;
    }

    if (g_strcmp0(argv1, "begin") == 0)
        gtk_adjustment_set_value(bar, gtk_adjustment_get_lower(bar));
    else if (g_strcmp0(argv1, "end") == 0)
        gtk_adjustment_set_value (bar, gtk_adjustment_get_upper(bar) -
                                gtk_adjustment_get_page_size(bar));
    else
        scroll(bar, argv1);
}

void
set_var(const gchar *rest_of_line) {
    if(!rest_of_line)
        return;

    gchar **split = g_strsplit(rest_of_line, "=", 2);
    if (split[0] != NULL) {
        gchar *value = split[1] ? g_strchug(split[1]) : " ";
        set_var_value(g_strstrip(split[0]), value);
    }
    g_strfreev(split);
}

void
toggle_var(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page; (void) result;

    if(!argv)
        return;

    const gchar *var_name = argv->data;
    argv = g_slist_next(argv);

    uzbl_cmdprop *c = get_var_c(var_name);

    switch(c->type) {
    case TYPE_STR:
    {
        const gchar *next;

        if(g_slist_length(argv) >= 2) {
            gchar *current = get_var_value_string_c(c);

            const gchar *first   = argv->data;
            argv = g_slist_next(argv);

            const gchar *this    = first;
                         next    = argv->data;

            while(next && strcmp(current, this)) {
                this = next;
                argv = g_slist_next(argv);
                next = argv->data;
            }

            if(!next)
                next = first;

            g_free(current);
        } else
            next = "";

        set_var_value_string_c(c, next);
        break;
    }
    case TYPE_INT:
    {
        int current = get_var_value_int_c(c);
        int next;

        if(g_slist_length(argv) >= 2) {
            int first = strtoul(argv->data, NULL, 10);
            argv = g_slist_next(argv);

            int this = first;

            const gchar *next_s = argv->data;

            while(next_s && this != current) {
                this   = strtoul(next_s, NULL, 10);
                argv = g_slist_next(argv);
                next_s = argv->data;
            }

            if(next_s)
                next = strtoul(next_s, NULL, 10);
            else
                next = first;
        } else
            next = !current;

        set_var_value_int_c(c, next);
        break;
    }
    case TYPE_FLOAT:
    {
        float current = get_var_value_float_c(c);
        float next;

        if(g_slist_length(argv) >= 2) {
            float first = strtod(argv->data, NULL);
            argv = g_slist_next(argv);
            float  this = first;

            const gchar *next_s = argv->data;

            while(next_s && this != current) {
                this   = strtod(next_s, NULL);
                argv = g_slist_next(argv);
                next_s = argv->data;
            }

            if(next_s)
                next = strtod(next_s, NULL);
            else
                next = first;
        } else
            next = !current;

        set_var_value_float_c(c, next);
        break;
    }
    default:
        g_assert_not_reached();
    }

    send_set_var_event(var_name, c);
}

void
event(const gchar *rest_of_line) {
    GString *event_name;
    gchar **split = NULL;

    split = g_strsplit(rest_of_line, " ", 2);

    if(!split[0])
        return;

    event_name = g_string_ascii_up(g_string_new(split[0]));

    send_event(0, event_name->str, TYPE_FORMATTEDSTR, split[1] ? split[1] : "", NULL);

    g_string_free(event_name, TRUE);
    g_strfreev(split);
}

void
print(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page; (void) result;
    gchar* buf;

    if(!result)
        return;

    buf = expand(argv->data, 0);
    g_string_assign(result, buf);
    g_free(buf);
}

void
hardcopy(WebKitWebView *page, GSList *argv, GString *result) {
    (void) argv; (void) result;
    webkit_web_frame_print(webkit_web_view_get_main_frame(page));
}

void
include(const gchar *path) {
    if(path[0] == 0)
        return;

    if((path = find_existing_file(path))) {
        run_command_file(path);
        send_event(FILE_INCLUDED, NULL, TYPE_STR, path, NULL);
    }
}

void
show_inspector(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page; (void) argv; (void) result;

    webkit_web_inspector_show(uzbl.gui.inspector);
}

void
add_cookie(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page; (void) result;
    gchar *host, *path, *name, *value;
    gboolean secure = 0;
    SoupDate *expires = NULL;

    if(g_slist_length(argv) != 6)
        return;

    // Parse with same syntax as ADD_COOKIE event
    host = argv->data;
    argv = g_slist_next(argv);

    path = argv->data;
    argv = g_slist_next(argv);

    name = argv->data;
    argv = g_slist_next(argv);

    value = argv->data;
    argv = g_slist_next(argv);

    secure = strcmp (argv->data, "https") == 0;
    argv = g_slist_next(argv);

    if (strlen (argv->data) != 0)
        expires = soup_date_new_from_time_t (strtoul (argv->data, NULL, 10));

    // Create new cookie
    SoupCookie * cookie = soup_cookie_new (name, value, host, path, -1);
    soup_cookie_set_secure (cookie, secure);
    if (expires)
        soup_cookie_set_expires (cookie, expires);

    // Add cookie to jar
    uzbl.net.soup_cookie_jar->in_manual_add = 1;
    soup_cookie_jar_add_cookie (SOUP_COOKIE_JAR (uzbl.net.soup_cookie_jar), cookie);
    uzbl.net.soup_cookie_jar->in_manual_add = 0;
}

void
delete_cookie(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page; (void) result;

    if(g_slist_length(argv) < 4)
        return;

    const gchar *host = argv->data;
    argv = g_slist_next(argv);

    const gchar *path = argv->data;
    argv = g_slist_next(argv);

    const gchar *name = argv->data;
    argv = g_slist_next(argv);

    const gchar *value = argv->data;
    argv = g_slist_next(argv);

    SoupCookie * cookie = soup_cookie_new (
        name,
        value,
        host,
        path,
        0);

    uzbl.net.soup_cookie_jar->in_manual_add = 1;
    soup_cookie_jar_delete_cookie (SOUP_COOKIE_JAR (uzbl.net.soup_cookie_jar), cookie);
    uzbl.net.soup_cookie_jar->in_manual_add = 0;
}

void
clear_cookies(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page; (void) argv; (void) result;

    // Replace the current cookie jar with a new empty jar
    soup_session_remove_feature (uzbl.net.soup_session,
        SOUP_SESSION_FEATURE (uzbl.net.soup_cookie_jar));
    g_object_unref (G_OBJECT (uzbl.net.soup_cookie_jar));
    uzbl.net.soup_cookie_jar = uzbl_cookie_jar_new ();
    soup_session_add_feature(uzbl.net.soup_session,
        SOUP_SESSION_FEATURE (uzbl.net.soup_cookie_jar));
}

void
download(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void) result;

    const gchar *uri         = argv->data;
    const gchar *destination = NULL;

    if(argv->next)
        destination = argv->next->data;

    WebKitNetworkRequest *req = webkit_network_request_new(uri);
    WebKitDownload *download = webkit_download_new(req);

    download_cb(web_view, download, (gpointer)destination);

    if(webkit_download_get_destination_uri(download))
        webkit_download_start(download);
    else
        g_object_unref(download);

    g_object_unref(req);
}

void
run_js (WebKitWebView * web_view, GSList *argv, GString *result) {
    if (argv)
        eval_js(web_view, argv->data, result, "(command)");
}

void
run_external_js (WebKitWebView * web_view, GSList *argv, GString *result) {
    (void) result;
    gchar *path = NULL;

    if (!argv || !(path = find_existing_file(argv->data)))
        return;

    gchar *file_contents = NULL;

    GIOChannel *chan = g_io_channel_new_file(path, "r", NULL);
    if (chan) {
        gsize len;
        g_io_channel_read_to_end(chan, &file_contents, &len, NULL);
        g_io_channel_unref (chan);
    }

    if (uzbl.state.verbose)
        printf ("External JavaScript file %s loaded\n", argv->data);

    argv = g_slist_next(argv);

    gchar *js = str_replace("%s", argv ? argv->data : "", file_contents);
    g_free (file_contents);

    eval_js (web_view, js, result, path);
    g_free (js);
    g_free(path);
}

void
search_clear(WebKitWebView *page, GSList *argv, GString *result) {
    (void) argv; (void) result;
    webkit_web_view_unmark_text_matches (page);
    g_free(uzbl.state.searchtx);
    uzbl.state.searchtx = NULL;
}

void
search_forward_text (WebKitWebView *page, GSList *argv, GString *result) {
    (void) result;
    if(!argv)
        return;
    search_text(page, argv->data, TRUE);
}

void
search_reverse_text(WebKitWebView *page, GSList *argv, GString *result) {
    (void) result;
    if(!argv)
        return;
    search_text(page, argv->data, FALSE);
}

void
dehilight(WebKitWebView *page, GSList *argv, GString *result) {
    (void) argv; (void) result;
    webkit_web_view_set_highlight_text_matches (page, FALSE);
}

void
chain(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    while ((argv = g_slist_next(argv))) {
        parse_string(argv->data);
    }
}

void
close_uzbl (WebKitWebView *page, GSList *argv, GString *result) {
    (void)page; (void)argv; (void)result;
    // hide window a soon as possible to avoid getting stuck with a
    // non-response window in the cleanup steps
    if (uzbl.gui.main_window)
        gtk_widget_destroy(uzbl.gui.main_window);
    else if (uzbl.gui.plug)
        gtk_widget_destroy(GTK_WIDGET(uzbl.gui.plug));

    gtk_main_quit ();
}

void
spawn_async(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view; (void)result;
    spawn(argv, NULL, FALSE);
}

void
spawn_sync(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view;
    spawn(argv, result, FALSE);
}

void
spawn_sync_exec(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view;
    if(!result) {
        GString *force_result = g_string_new("");
        spawn(argv, force_result, TRUE);
        g_string_free (force_result, TRUE);
    } else
        spawn(argv, result, TRUE);
}

void
spawn_sh_async(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view; (void)result;
    spawn_sh(argv, NULL);
}

void
spawn_sh_sync(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view; (void)result;
    spawn_sh(argv, result);
}

void
act_dump_config(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view; (void) argv; (void)result;
    dump_config();
}

void
act_dump_config_as_events(WebKitWebView *web_view, GSList *argv, GString *result) {
    (void)web_view; (void) argv; (void)result;
    dump_config_as_events();
}
