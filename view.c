void view_table(View_Table_Model model)
{
    table_widget(model.table, model.cursor);

    char help_msg[255] = "Options are (q)uit and (e)dit.\n";
    if (model.status_bar_text && strlen(model.status_bar_text)) {
        status_bar_widget(model.status_bar_text);
    } else {
        status_bar_widget(help_msg);
    }
    refresh();
}
