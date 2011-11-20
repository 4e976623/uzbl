%{

#include <glib.h>

#include "../src/uzbl-core.h"
#include "../src/commands.h"
#include "../src/variables.h"

#include "uzbl.tab.h"
#include "uzbl.flex.h"

Dingus *uzbl_new_dingus(const gchar *command, GSList *argv);
void uzbl_free_dingus(Dingus *dingus);

void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n", str);
}

%}

%error-verbose
%define api.pure
%define api.push-pull push

%union {
  GSList* list;
  Dingus* dingus;
  gchar* string;
}

%token <string> SET PRINT JS SEARCH SEARCH_REVERSE EVENT
%token <string> MENU_ADD       MENU_LINK_ADD       MENU_IMAGE_ADD        MENU_EDITABLE_ADD
%token <string> MENU_SEPARATOR MENU_LINK_SEPARATOR MENU_IMAGE_SEPARATOR  MENU_EDITABLE_SEPARATOR
%token <string> MENU_REMOVE    MENU_LINK_REMOVE    MENU_IMAGE_REMOVE     MENU_EDITABLE_REMOVE
%token <string> INCLUDE REST_OF_LINE DQUOTE SQUOTE EOL WORD
%token <string> FUNC LCURLY RCURLY

%type  <list>    rol arguments lines
%type  <dingus>  command line
%%
script: lines { execute($1); }

lines:
        { $$ = NULL; }
        |
        lines line    { $$ = g_slist_append($$, $2); }
        |
        lines command { $$ = g_slist_append($$, $2); }
        ;

line: command EOL { $$ = $1; }
      |
      EOL         { $$ = NULL; }

command:
       SET rol {
          $$ = uzbl_new_dingus("set", $2);
       }
       |
       PRINT rol          {
          $$ = uzbl_new_dingus("print", $2);
       }
       |
       JS rol             {
          $$ = uzbl_new_dingus("js", $2);
       }
       |
       SEARCH rol         {
          $$ = uzbl_new_dingus("search", $2);
       }
       |
       SEARCH_REVERSE rol {
          $$ = uzbl_new_dingus("search_reverse", $2);
       }
       |
       EVENT rol          {
          $$ = uzbl_new_dingus("event", $2);
       }
       |
       INCLUDE rol        {
          $$ = uzbl_new_dingus("include", $2);
       }
       |
       FUNC WORD LCURLY lines RCURLY {
          // i don't know WTF i'm doing 
          GSList *whatever = NULL;
          whatever = g_slist_prepend(whatever, $4);
          whatever = g_slist_prepend(whatever, $2);

          $$ = uzbl_new_dingus("func", whatever);
          //g_slist_free_full($2, g_free);
          // and should probably free something else too
       }
       |
       WORD arguments     {
          $$ = uzbl_new_dingus($1, $2);
       }
       ;

rol:
  REST_OF_LINE { $$ = g_slist_prepend(NULL, $1); }

arguments:
      // should be left-recursive instead?
      WORD arguments { $$ = g_slist_prepend($2, $1); }
      |
      { $$ = NULL; }
      ;

%%

Dingus *uzbl_new_dingus(const gchar *command, GSList *argv) {
  Dingus *result  = malloc(sizeof(Dingus));
  result->command = command;
  result->argv    = argv;
  return result;
}

void execute_line(Dingus *dingus) {
  if(!dingus)
    return;

  const gchar *cmd = dingus->command;

  if (!strcmp(cmd, "set")) {
    set_var(dingus->argv->data);
  } else if (!strcmp(cmd, "print")) {
    puts(dingus->argv->data);
  } else if (!strcmp(cmd, "js")) {
    eval_js(uzbl.gui.web_view, dingus->argv->data, NULL, "(command)");
  } else if (!strcmp(cmd, "search")) {
    search_forward_text(uzbl.gui.web_view, dingus->argv, NULL);
  } else if (!strcmp(cmd, "search_reverse")) {
    search_reverse_text(uzbl.gui.web_view, dingus->argv, NULL);
  } else if (!strcmp(cmd, "event")) {
    event(dingus->argv->data);
  } else if (!strcmp(cmd, "include")) {
    include(dingus->argv->data);
  } else if (!strcmp(cmd, "func")) {
    define_function(dingus->argv->data, dingus->argv->next->data);
  } else {
    CommandInfo *c = g_hash_table_lookup(uzbl.behave.commands, cmd);

    if(c)
        run_parsed_command(c, dingus->argv, NULL);
  }
}

void execute(GSList *lines) {
  g_slist_foreach(lines, execute_line, NULL);
}

void define_function(const gchar* name, GSList *body) {
}

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
