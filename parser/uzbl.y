%{
#define YYSTYPE char *

#include "uzbl.flex.h"

#include "../src/commands.h"
#include "../src/variables.h"

extern gchar* expand(const char* s, guint recurse);

#include <glib.h>

void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n", str);
}

%}

%error-verbose
%define api.pure
%define api.push-pull push

%token SET PRINT URI JS SEARCH SEARCH_REVERSE EVENT
%token MENU_ADD       MENU_LINK_ADD       MENU_IMAGE_ADD        MENU_EDITABLE_ADD
%token MENU_SEPARATOR MENU_LINK_SEPARATOR MENU_IMAGE_SEPARATOR  MENU_EDITABLE_SEPARATOR
%token MENU_REMOVE    MENU_LINK_REMOVE    MENU_IMAGE_REMOVE     MENU_EDITABLE_REMOVE
%token INCLUDE REST_OF_LINE DQUOTE SQUOTE EOL WORD
%%
lines:
        | lines line
        ;

line: command EOL | EOL

command:
       SET rol            { set_var($2); g_free($2); }
       |
       PRINT rol          { printf("print %s\n", $2); }
       |
       URI rol            { set_var_value("uri", $2); g_free($2); }
       |
       JS rol             { printf("js %s\n", $2); }
       |
       SEARCH rol         { printf("search %s\n", $2); }
       |
       SEARCH_REVERSE rol { printf("search_reverse %s\n", $2); }
       |
       EVENT rol          { event($2); g_free($2); }
       |
       INCLUDE rol        { include($2); g_free($2); }
       |
       WORD arguments     { printf("CUSTOM COMMAND %s\n", $1); }
       ;

rol:
  REST_OF_LINE

arguments:
      | arguments argument
      ;

argument: WORD | quoted_words

quoted_words: DQUOTE words DQUOTE | SQUOTE words SQUOTE

words:
      | words WORD
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
