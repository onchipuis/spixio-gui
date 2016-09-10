#ifndef SPIXIO_GTK_H
#define SPIXIO_GTK_H

struct _SMain
{
	// ** Main windows
	GtkWidget  *window_main;
    GtkAboutDialog  *about_main;

    // ** Spin buttons
    GtkSpinButton *spinbutton_word;
    GtkSpinButton *spinbutton_inputs;
    GtkSpinButton *spinbutton_outputs;

    // ** Tree views
    GtkTreeView *tree_inputs;
    GtkTreeView *tree_outputs;

	// ** Toogle actions
	GtkToggleAction *toogleaction_auto;

	// ** Label
	GtkLabel *label_main;

	int radix;
};

typedef struct _SMain SMain;

extern SMain *gMain;

/*G_MODULE_EXPORT void
on_tree_inputs_value_edited (GtkCellRendererText *renderer,
               gchar               *path,
               gchar               *new_text,
               gpointer             user_data);*/

G_MODULE_EXPORT void
on_tree_outputs_value_edited (GtkCellRendererText *renderer,
               gchar               *path,
               gchar               *new_text,
               gpointer             user_data);

void create_timer_auto_update(void);
void destroy_timer_auto_update(void);
void update_spi_things(void);
void update_single(void);

#endif // SPIXIO_GTK_H
