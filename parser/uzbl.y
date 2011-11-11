%{

#include <glib.h>

#include "../src/uzbl-core.h"
#include "../src/commands.h"
#include "../src/variables.h"

#include "uzbl.tab.h"
#include "uzbl.flex.h"

void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n", str);
}

%}

%error-verbose
%define api.pure
%define api.push-pull push

%union {
  GArray* array;
  gchar* string;
}

%token <string> SET PRINT URI JS SEARCH SEARCH_REVERSE EVENT
%token <string> MENU_ADD       MENU_LINK_ADD       MENU_IMAGE_ADD        MENU_EDITABLE_ADD
%token <string> MENU_SEPARATOR MENU_LINK_SEPARATOR MENU_IMAGE_SEPARATOR  MENU_EDITABLE_SEPARATOR
%token <string> MENU_REMOVE    MENU_LINK_REMOVE    MENU_IMAGE_REMOVE     MENU_EDITABLE_REMOVE
%token <string> INCLUDE REST_OF_LINE DQUOTE SQUOTE EOL WORD

%type  <string> command rol
%type  <array>  arguments
%%
lines:
        | lines line | lines command
        ;

line: command EOL | EOL

command:
       SET rol            { set_var($2); g_free($2); }
       |
       PRINT rol          { printf("print %s\n", $2); g_free($2); }
       |
       URI rol            { set_var_value("uri", $2); g_free($2); }
       |
       JS rol             { eval_js(uzbl.gui.web_view, $2, NULL, "(command)"); g_free($2); }
       |
       SEARCH rol         { search_text(uzbl.gui.web_view, $2, TRUE); g_free($2); }
       |
       SEARCH_REVERSE rol { search_text(uzbl.gui.web_view, $2, FALSE); g_free($2); }
       |
       EVENT rol          { event($2); g_free($2); }
       |
       INCLUDE rol        { include($2); g_free($2); }
       |
       WORD arguments     {
          CommandInfo *c = g_hash_table_lookup(uzbl.behave.commands, $1);

          run_parsed_command(c, $2, NULL);

/* this crashes for some reason...
          for(int i = 0; i < $2->len; ++i)
            g_free($2->data[i]);
*/

          g_array_free($2, TRUE);
       }
       ;

rol:
  REST_OF_LINE

arguments:
      WORD arguments { g_array_prepend_val($2, $1); $$ = $2; }
      |
      { $$ = g_array_new(TRUE, FALSE, sizeof(gchar*)); }
      ;

%%

int parse_string(const char* input)
{
  gchar *expanded = expand(input, 0);

  if(expanded[0] == 0) {
      g_free(expanded);
      return 0;
  }

  yypstate *ps;
  yyscan_t scanner;
  YYSTYPE fake_it;
  int status;

  ps = yypstate_new();
  yylex_init(&scanner);


  YY_BUFFER_STATE buf = yy_scan_string(expanded, scanner);

  do {
    memset(&fake_it, 0, sizeof(YYSTYPE));
    int result = yylex(&fake_it, scanner);
    status = yypush_parse (ps, result, &fake_it);
  } while (status == YYPUSH_MORE);

  yy_delete_buffer(buf, scanner);
  yylex_destroy(scanner);

  g_free(expanded);
  return 0;
}
