/*
 * Uzbl Commands
 */
#ifndef __COMMANDS__
#define __COMMANDS__

#include <webkit/webkit.h>

typedef void (*Command)(WebKitWebView*, GSList *argv, GString *result, void *user_data);

typedef struct {
    const gchar *key;
    Command      function;
    void *user_data;
} CommandInfo;

typedef struct {
  gchar  *command;
  GSList *argv;
} Dingus;

/**
 * Initialises the hash table uzbl.behave.commands with the available commands.
 */
void
commands_hash();

/**
 * Sends the BUILTINS events with the available commands.
 */
void
builtins();


void        view_reload(WebKitWebView *page, GSList *argv, GString *result);
void        view_reload_bypass_cache(WebKitWebView *page, GSList *argv, GString *result);
void        view_stop_loading(WebKitWebView *page, GSList *argv, GString *result);
void        view_zoom_in(WebKitWebView *page, GSList *argv, GString *result);
void        view_zoom_out(WebKitWebView *page, GSList *argv, GString *result);
void        view_go_back(WebKitWebView *page, GSList *argv, GString *result);
void        view_go_forward(WebKitWebView *page, GSList *argv, GString *result);
void        toggle_zoom_type (WebKitWebView* page, GSList *argv, GString *result);
void        scroll_cmd(WebKitWebView* page, GSList *argv, GString *result);
void        print(WebKitWebView *page, GSList *argv, GString *result);
void        chain(WebKitWebView *page, GSList *argv, GString *result);
void        close_uzbl(WebKitWebView *page, GSList *argv, GString *result);
void        spawn_async(WebKitWebView *web_view, GSList *argv, GString *result);
void        spawn_sh_async(WebKitWebView *web_view, GSList *argv, GString *result);
void        spawn_sync(WebKitWebView *web_view, GSList *argv, GString *result);
void        spawn_sh_sync(WebKitWebView *web_view, GSList *argv, GString *result);
void        spawn_sync_exec(WebKitWebView *web_view, GSList *argv, GString *result);
void        search_forward_text (WebKitWebView *page, GSList *argv, GString *result);
void        search_reverse_text (WebKitWebView *page, GSList *argv, GString *result);
void        search_clear(WebKitWebView *page, GSList *argv, GString *result);
void        dehilight (WebKitWebView *page, GSList *argv, GString *result);
void        hardcopy(WebKitWebView *page, GSList *argv, GString *result);
void        show_inspector(WebKitWebView *page, GSList *argv, GString *result);
void        add_cookie(WebKitWebView *page, GSList *argv, GString *result);
void        delete_cookie(WebKitWebView *page, GSList *argv, GString *result);
void        clear_cookies(WebKitWebView *pag, GSList *argv, GString *result);
void        download(WebKitWebView *pag, GSList *argv, GString *result);
void        toggle_var(WebKitWebView *page, GSList *argv, GString *result);
void        run_js (WebKitWebView * web_view, GSList *argv, GString *result);
void        run_external_js (WebKitWebView * web_view, GSList *argv, GString *result);
void        act_dump_config(WebKitWebView* page, GSList *argv, GString *result);
void        act_dump_config_as_events(WebKitWebView* page, GSList *argv, GString *result);

void        event(const gchar *rest_of_line);
void        set_var(const gchar *rest_of_line);
void        include(const gchar *path);

#endif
