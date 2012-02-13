#ifndef __UZBL_JAVASCRIPT_H__
#define __UZBL_JAVASCRIPT_H__

void
eval_js(WebKitWebView * web_view, const gchar *script, GString *result, const char *file);

void
set_up_javascript();

#endif
