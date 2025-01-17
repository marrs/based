void view_table(View_Table_Model model)
{
    clear();
    attron(A_BOLD);
        mvprintw(2, 1, "%s", model.table->name);
    attroff(A_BOLD);
    table_widget(model.table, model.cursor);

    char help_msg[255] = "Options are (q)uit and (e)dit.\n";
    if (model.status_bar_text && strlen(model.status_bar_text)) {
        status_bar_widget(model.status_bar_text);
    } else {
        status_bar_widget(help_msg);
    }
    refresh();
}
