#include <JavaScriptCore/JavaScript.h>

#include "uzbl-core.h"
#include "javascript.h"
#include "events.h"
#include "type.h"

void
eval_js(WebKitWebView * web_view, const gchar *script, GString *result, const char *file) {
    WebKitWebFrame *frame;
    JSGlobalContextRef context;
    JSObjectRef globalobject;
    JSStringRef js_file;
    JSStringRef js_script;
    JSValueRef js_result;
    JSValueRef js_exc = NULL;
    JSStringRef js_result_string;
    size_t js_result_size;

    frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(web_view));
    context = webkit_web_frame_get_global_context(frame);
    globalobject = JSContextGetGlobalObject(context);

    /* evaluate the script and get return value*/
    js_script = JSStringCreateWithUTF8CString(script);
    js_file = JSStringCreateWithUTF8CString(file);
    js_result = JSEvaluateScript(context, js_script, globalobject, js_file, 0, &js_exc);
    if (result && js_result && !JSValueIsUndefined(context, js_result)) {
        js_result_string = JSValueToStringCopy(context, js_result, NULL);
        js_result_size = JSStringGetMaximumUTF8CStringSize(js_result_string);

        if (js_result_size) {
            char js_result_utf8[js_result_size];
            JSStringGetUTF8CString(js_result_string, js_result_utf8, js_result_size);
            g_string_assign(result, js_result_utf8);
        }

        JSStringRelease(js_result_string);
    }
    else if (js_exc) {
        size_t size;
        JSStringRef prop, val;
        JSObjectRef exc = JSValueToObject(context, js_exc, NULL);

        printf("Exception occured while executing script:\n");

        /* Print file */
        prop = JSStringCreateWithUTF8CString("sourceURL");
        val = JSValueToStringCopy(context, JSObjectGetProperty(context, exc, prop, NULL), NULL);
        size = JSStringGetMaximumUTF8CStringSize(val);
        if(size) {
            char cstr[size];
            JSStringGetUTF8CString(val, cstr, size);
            printf("At %s", cstr);
        }
        JSStringRelease(prop);
        JSStringRelease(val);

        /* Print line */
        prop = JSStringCreateWithUTF8CString("line");
        val = JSValueToStringCopy(context, JSObjectGetProperty(context, exc, prop, NULL), NULL);
        size = JSStringGetMaximumUTF8CStringSize(val);
        if(size) {
            char cstr[size];
            JSStringGetUTF8CString(val, cstr, size);
            printf(":%s: ", cstr);
        }
        JSStringRelease(prop);
        JSStringRelease(val);

        /* Print message */
        val = JSValueToStringCopy(context, exc, NULL);
        size = JSStringGetMaximumUTF8CStringSize(val);
        if(size) {
            char cstr[size];
            JSStringGetUTF8CString(val, cstr, size);
            printf("%s\n", cstr);
        }
        JSStringRelease(val);
    }

    /* cleanup */
    JSStringRelease(js_script);
    JSStringRelease(js_file);
}

JSValueRef
post_uzbl_message_function(JSContextRef ctx, JSObjectRef function, JSObjectRef this_object, size_t argument_count, const JSValueRef arguments[], JSValueRef *exception) {
    (void) function; (void) this_object; (void) exception;

    GArray* c_arguments = g_array_sized_new(TRUE, TRUE, sizeof(gchar *), argument_count);

    // turn each javascript argument into a C string
    for(size_t i = 0; i < argument_count; ++i) {
        JSStringRef arg = JSValueToStringCopy(ctx, arguments[i], NULL);
        size_t arg_size = JSStringGetMaximumUTF8CStringSize(arg);

        gchar *cstring = malloc(arg_size);
        if(cstring) {
          JSStringGetUTF8CString(arg, cstring, arg_size);

          g_array_append_val(c_arguments, cstring);
        }

        JSStringRelease(arg);
    }

    // send the event
    send_event(JS_MESSAGE, NULL, TYPE_STR_ARRAY, c_arguments, NULL);

    // free the strings we created
    for(size_t i = 0; i < argument_count; ++i)
        g_free(g_array_index(c_arguments, gchar *, i));

    g_array_free(c_arguments, TRUE);

    return JSValueMakeUndefined(ctx);
}

void
set_up_javascript() {
    WebKitWebFrame *frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(uzbl.gui.web_view));
    JSGlobalContextRef context = webkit_web_frame_get_global_context(frame);
    JSObjectRef globalobject = JSContextGetGlobalObject(context);

    JSStringRef function_name = JSStringCreateWithUTF8CString("postUzblMessage");

    JSObjectRef post_uzbl_message = JSObjectMakeFunctionWithCallback(context, function_name, post_uzbl_message_function);

    JSObjectSetProperty(context, globalobject,
                        function_name, post_uzbl_message,
                        kJSPropertyAttributeNone, NULL);
}
