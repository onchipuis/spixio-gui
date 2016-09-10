#include "stdafx.h"
#include "spixio-gtk.h"
#include "spixio-gui.h"
#include "spixio.h"

SMain *gMain;

G_MODULE_EXPORT void                on_action_about_activate                      (GtkMenuItem *menuitem,
                                                        gpointer     user_data)
{
    // TODO: Add response here
    gtk_window_set_transient_for(GTK_WINDOW(gMain->about_main), GTK_WINDOW(gMain->window_main));
    gtk_dialog_run( GTK_DIALOG( gMain->about_main ) );
    gtk_widget_hide( GTK_WIDGET(gMain->about_main) );
}

/*G_MODULE_EXPORT void
on_tree_inputs_value_edited (GtkCellRendererText *renderer,
               gchar               *path,
               gchar               *new_text,
               gpointer             user_data)
{

}*/

G_MODULE_EXPORT void
on_tree_outputs_value_edited (GtkCellRendererText *renderer,
               gchar               *path,
               gchar               *new_text,
               gpointer             user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(gMain->tree_outputs);
	GtkTreeIter iter;

	g_object_ref(model);

	if (gtk_tree_model_get_iter_from_string (model, &iter, path))
	{
        char buf2[100];
		gint value;
		gtk_tree_model_get (model, &iter, 2, &value, -1);

        int ndig = get_ndig(GSPI_WORD, gMain->radix);
		int nvalue = real_radix(new_text, buf2, ndig, gMain->radix, gMain->radix);

		if(GSPI_DONE) write_single_word(value, GSPI_ADDR, (uint32_t)nvalue, GSPI_WORD);

		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 1, buf2, -1);
	}

	g_object_unref(model);
}

void update_spi_things(void)
{
    GSPI_MAXOBJ = FUNC_GSPI_MAXOBJ;
    GSPI_ADDR = FUNC_GSPI_SPI_ADDR_BITS;

    gtk_label_set_label(gMain->label_main, GSPI_NAME);
    refill(GSPI_INPUTS, GSPI_OUTPUTS, GSPI_WORD, gMain->radix);
}

G_MODULE_EXPORT void
on_spinbutton_word_value_changed (GtkSpinButton *spin_button,
               gpointer       user_data)
{
    GSPI_WORD = gtk_spin_button_get_value(spin_button);
    update_spi_things();
}

G_MODULE_EXPORT void
on_spinbutton_inputs_value_changed (GtkSpinButton *spin_button,
               gpointer       user_data)
{
    GSPI_INPUTS = gtk_spin_button_get_value(spin_button);
    update_spi_things();
}

G_MODULE_EXPORT void
on_spinbutton_outputs_value_changed (GtkSpinButton *spin_button,
               gpointer       user_data)
{
    GSPI_OUTPUTS = gtk_spin_button_get_value(spin_button);
    update_spi_things();
}

G_MODULE_EXPORT void
on_action_radix_activate (GtkAction *action,
               gpointer   user_data)
{
    int bradix = gMain->radix;
    switch(gMain->radix)
    {
        case 2: gMain->radix = 8; break;
        case 8: gMain->radix = 10; break;
        case 10: gMain->radix = 16; break;
        case 16: gMain->radix = 2; break;
        default: gMain->radix = 16; break;
    }
    change_radix(GSPI_WORD, GSPI_WORD, bradix, gMain->radix);
}

G_MODULE_EXPORT void
on_action_detect_activate (GtkAction *action,
               gpointer   user_data)
{
    spi_close();
    spi_init();
}

G_MODULE_EXPORT void
on_action_update_activate (GtkAction *action,
               gpointer   user_data)
{
    update_single();
}

G_MODULE_EXPORT void
on_toggleaction_auto_toggled (GtkToggleAction *toggleaction,
               gpointer         user_data)
{
    gboolean bis = gtk_toggle_action_get_active(toggleaction);
    if(bis == TRUE) create_timer_auto_update();
    else destroy_timer_auto_update();
}

void update_single(void)
{
    GtkTreeIter iter;
    GtkTreeView* tree_view = gMain->tree_inputs;
	gboolean b;
	GtkTreeModel *model;

    model = gtk_tree_view_get_model(tree_view);
    if(model == NULL || !GSPI_DONE) return;

    g_object_ref(model);

    gtk_tree_view_set_model(tree_view, NULL); /* Detach model from view */

    for(b = gtk_tree_model_get_iter_first(model, &iter);
            b!=FALSE; b=gtk_tree_model_iter_next(model, &iter))
    {
        gint value;
        uint32_t nvalue;
        char chbuf2[100];
        gtk_tree_model_get(model, &iter, 2, &value, -1);

        read_single_word((uint32_t)value, GSPI_ADDR, &nvalue, GSPI_WORD);
        sprint_radix(chbuf2, nvalue, get_ndig(GSPI_WORD, gMain->radix), gMain->radix);

		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						   1, chbuf2,
						   -1);
    }

    gtk_tree_view_set_model(tree_view, model); /* Detach model from view */

    g_object_unref(model);
}

gboolean auto_update_enable = FALSE;
gboolean auto_update_timer( gpointer data )
{
    update_single();
    return auto_update_enable;
}

void create_timer_auto_update(void)
{
    auto_update_enable = TRUE;
    g_timeout_add( 1000,
                      auto_update_timer,
                      NULL );
}

void destroy_timer_auto_update(void)
{
    auto_update_enable = FALSE;
}


