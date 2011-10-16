const struct {
    /*@null@*/ char *command;
} default_config[] = {
{ "set status_format = <b>\\@[\\@TITLE]\\@</b> - \\@[\\@uri]\\@ - <span foreground=\"#bbb\">\\@NAME</span>\n" },
{ "set show_status = 1\n" },
{ "set title_format_long = \\@keycmd \\@TITLE - Uzbl browser <\\@NAME> > \\@SELECTED_URI\n" },
{ "set title_format_short = \\@TITLE - Uzbl browser <\\@NAME>\n" },
{ "set max_conns = 100\n" }, /* WebkitGTK default: 10 */
{ "set max_conns_host = 6\n" }, /* WebkitGTK default: 2 */
{ NULL }
};
