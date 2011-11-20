#include "menu.h"
#include "util.h"
#include "uzbl-core.h"

void
add_to_menu(GSList *argv, guint context) {
    GUI *g = &uzbl.gui;
    MenuItem *m;
    gchar *item_cmd = NULL;

    if(argv)
        return;

    gchar **split = g_strsplit(argv->data, "=", 2);
    if(!g->menu_items)
        g->menu_items = g_ptr_array_new();

    if(split[1])
        item_cmd = split[1];

    if(split[0]) {
        m = malloc(sizeof(MenuItem));
        m->name = g_strdup(split[0]);
        m->cmd  = g_strdup(item_cmd?item_cmd:"");
        m->context = context;
        m->issep = FALSE;
        g_ptr_array_add(g->menu_items, m);
    }

    g_strfreev(split);
}


void
menu_add(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT);
}


void
menu_add_link(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK);
}


void
menu_add_image(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE);
}


void
menu_add_edit(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE);
}


void
add_separator_to_menu(GSList *argv, guint context) {
    GUI *g = &uzbl.gui;
    MenuItem *m;
    gchar *sep_name;

    if(!argv)
        return;

    sep_name = argv->data;

    if(!g->menu_items)
        g->menu_items = g_ptr_array_new();

    m = malloc(sizeof(MenuItem));
    m->name    = g_strdup(sep_name);
    m->cmd     = NULL;
    m->context = context;
    m->issep   = TRUE;
    g_ptr_array_add(g->menu_items, m);
}


void
menu_add_separator(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_separator_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT);
}


void
menu_add_separator_link(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_separator_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK);
}


void
menu_add_separator_image(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_separator_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE);
}


void
menu_add_separator_edit(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    add_separator_to_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE);
}


void
remove_from_menu(GSList *argv, guint context) {
    GUI *g = &uzbl.gui;
    MenuItem *mi;
    gchar *name = NULL;
    guint i=0;

    if(!argv || !g->menu_items)
        return;

    name = argv->data;

    for(i=0; i < g->menu_items->len; i++) {
        mi = g_ptr_array_index(g->menu_items, i);

        if((context == mi->context) && !strcmp(name, mi->name)) {
            g_free(mi->name);
            g_free(mi->cmd);
            g_free(mi);
            g_ptr_array_remove_index(g->menu_items, i);
        }
    }
}


void
menu_remove(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    remove_from_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT);
}


void
menu_remove_link(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    remove_from_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK);
}


void
menu_remove_image(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    remove_from_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE);
}


void
menu_remove_edit(WebKitWebView *page, GSList *argv, GString *result) {
    (void) page;
    (void) result;

    remove_from_menu(argv, WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE);
}

