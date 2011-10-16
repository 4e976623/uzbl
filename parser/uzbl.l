%{
#include "uzbl.tab.h"
%}

%x rest_of_line

%option noyywrap nounput
%option reentrant
/* tell flex to add yylval argument to yylex() */
%option bison-bridge

%%

uri\                      { BEGIN(rest_of_line); return URI; }
js\                       { BEGIN(rest_of_line); return JS;  }
search\                   { BEGIN(rest_of_line); return SEARCH; }
search_reverse\           { BEGIN(rest_of_line); return SEARCH_REVERSE; }
set\                      { BEGIN(rest_of_line); return SET; }
print\                    { BEGIN(rest_of_line); return PRINT; }
event\                    { BEGIN(rest_of_line); return EVENT; }
request\                  { BEGIN(rest_of_line); return EVENT; }

menu_add\                 { BEGIN(rest_of_line); return MENU_ADD; }
menu_link_add\            { BEGIN(rest_of_line); return MENU_LINK_ADD; }
menu_image_add\           { BEGIN(rest_of_line); return MENU_IMAGE_ADD; }
menu_editable_add\        { BEGIN(rest_of_line); return MENU_EDITABLE_ADD; }

menu_separator\           { BEGIN(rest_of_line); return MENU_SEPARATOR; }
menu_link_separator\      { BEGIN(rest_of_line); return MENU_LINK_SEPARATOR; }
menu_image_separator\     { BEGIN(rest_of_line); return MENU_IMAGE_SEPARATOR; }
menu_editable_separator\  { BEGIN(rest_of_line); return MENU_EDITABLE_SEPARATOR; }

menu_remove\              { BEGIN(rest_of_line); return MENU_REMOVE; }
menu_link_remove\         { BEGIN(rest_of_line); return MENU_LINK_REMOVE; }
menu_image_remove\        { BEGIN(rest_of_line); return MENU_IMAGE_REMOVE; }
menu_editable_remove\     { BEGIN(rest_of_line); return MENU_EDITABLE_REMOVE; }

include\                  { BEGIN(rest_of_line); return INCLUDE; }

<rest_of_line>[^\n]*    { *(char**)yylval = strdup(yytext); return REST_OF_LINE; }
<rest_of_line>\n        { BEGIN(INITIAL); return EOL; }



[a-zA-Z0-9_/:\.-]+            { *(char**)yylval = strdup(yytext); return WORD; }

\"                      return DQUOTE;
\'                      return SQUOTE;

\n                      return EOL;
#.*\n                   /* ignore comments */;
[ \t]+                  /* ignore whitespace */;
%%
