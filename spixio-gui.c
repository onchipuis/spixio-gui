#include "stdafx.h"
#include "spixio.h"
#include "spixio-gui.h"
#include "spixio-gtk.h"

/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/

void GetMaxCurrentDirectory(char* ch)
{
	char strDirect[MAX_PATH];

	getcwd(strDirect, sizeof(char)*MAX_PATH);
	strcpy(ch, strDirect);
}

void SetMaxCurrentDirectory(char* ch)
{
	char strDirect[MAX_PATH];

	strcpy(strDirect, ch);
	chdir(strDirect);
}

void GetMaxProcessDirectory(char* ch)
{
#ifdef _WIN32
	char strApp[MAX_PATH];
	char strDrive[MAX_PATH];
	char strDir[MAX_PATH];
	char strFile[MAX_PATH];
	char strExt[MAX_PATH];
	GetModuleFileName(NULL, strApp, MAX_PATH);
	_tsplitpath(strApp, strDrive, strDir, strFile, strExt);
	sprintf(ch, _T("%s%s"), strDrive, strDir);
#endif // WIN32

#if defined (UNIX) || defined(__linux__)
    char buf[MAX_PATH + 1];
    if (readlink("/proc/self/exe", buf, sizeof(buf) - 1) == -1)
	{
        fprintf(stderr, "readlink() failed\n");
		return;
	}
    char * found = strrchr(buf, '/');
    if(found != NULL) (*(found+1)) = '0';
	strcpy(ch, buf);
#endif
}

void GetMaxUserDirectory(char* ch)
{
#ifdef _WIN32
	if(!SUCCEEDED(SHGetFolderPath(NULL,
								 CSIDL_PERSONAL|CSIDL_FLAG_CREATE,
								 NULL,
								 0,
								 ch)))
	{
		throw std::string("SHGetFolderPath() failed");
		return;
	}
#endif

#if defined (UNIX) || defined(__linux__)
	// struct passwd *pw = getpwuid(getuid());
	const char *homedir =  getenv("HOME"); // pw->pw_dir;
	strcpy(ch, homedir);
#endif
}

void SetCurDir()
{
	char chCore[MAX_PATH];
	GetMaxProcessDirectory(chCore);
	SetMaxCurrentDirectory(chCore);
	printf("Set dir to process: %s", chCore);
}

int get_ndig(int word, int radix)
{
    return (int)(log((double)((1<<word)-1))/log((double)radix)) + 1;
}

void sprint_radix(char* chbuf2, uint32_t nvalue, int ndig, int radix)
{
    int i;
    if(radix == 16) sprintf(chbuf2, "%.*x", ndig, nvalue);
    if(radix == 10) sprintf(chbuf2, "%u", (unsigned int)nvalue);
    if(radix == 8) sprintf(chbuf2, "%.*o", ndig, nvalue);
    if(radix == 2) {
        for(i = 0; i < ndig; i++)
        {
            chbuf2[i] = ((nvalue >> (ndig - i - 1)) & 0x1)?'1':'0';
        }
        chbuf2[i] = 0;
    }
}

int real_radix(char* buf2, char* chbuf2, int ndig, int bradix, int radix)
{
    int nvalue = strtol(buf2, NULL, bradix);
    sprint_radix(chbuf2, nvalue, ndig, radix);
    return nvalue;
}

int change_radix(int bword, int word, int bradix, int radix)
{
    int ndig = get_ndig(word, radix);
    //int bndig = get_ndig(bword, bradix);

    GtkTreeIter iter;
    GtkTreeView* tree_view = gMain->tree_inputs;
	char bDoneOutputs=0;
	gboolean b;
	GtkTreeModel *model;

CHANGERADIX_DOAGAIN:
    model = gtk_tree_view_get_model(tree_view);
    if(model == NULL) return 0;

    g_object_ref(model);

    gtk_tree_view_set_model(tree_view, NULL); /* Detach model from view */

    for(b = gtk_tree_model_get_iter_first(model, &iter);
            b!=FALSE; b=gtk_tree_model_iter_next(model, &iter))
    {
        gchar* buf2;
        char chbuf2[100];
        gtk_tree_model_get(model, &iter, 1, &buf2, -1);

        real_radix(buf2, chbuf2, ndig, bradix, radix);

		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						   1, chbuf2,
						   -1);
    }

    gtk_tree_view_set_model(tree_view, model); /* Detach model from view */

    g_object_unref(model);

    if(!bDoneOutputs)
	{
        tree_view = gMain->tree_outputs;
        bDoneOutputs = 1;
        goto CHANGERADIX_DOAGAIN;
	}
    return 1;
}

int refill(int inputs, int outputs, int word, int radix)
{
    GtkListStore *store = NULL;
	GtkTreeModel *model = NULL;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	int i, j;
	char buf1[100];
	char buf2[100];
	char bDoneOutputs=0;

	if(inputs < 1 || outputs < 1 || word < 1 || radix < 2) return 0;
    int ndig = (int)(log((double)((1<<word)-1))/log((double)radix)) + 1;

	/* First, with the inputs, then with the outputs
	We are going to see if there is a GtkListStore*/

	change_radix(word, word, radix, radix);

	int ncant = inputs;
	GtkTreeView* tree_view = gMain->tree_inputs;
REFILL_DOAGAIN:
	model = gtk_tree_view_get_model(tree_view);
    store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    for(i = 0; i < ncant; i++)
    {
        if(!bDoneOutputs) sprintf(buf1, "Input %d", i+1);
        else sprintf(buf1, "Output %d", i+1);
        /*How much items are here?*/
        if(model != NULL)
        {
            if(gtk_tree_model_iter_nth_child (model,
                               &iter,
                               NULL,
                               i) != FALSE)
            {
                gchar* val = NULL;
                gint value;
                gtk_tree_model_get(model, &iter, 1, &val, 2, &value, -1);
                strcpy(buf2, val);
                if(value == i) goto DONOT_FILL_WITH_BLANK;
            }
        }
        for(j = 0; j < ndig; j++)
            buf2[j] = '0';
        buf2[j] = 0;
        DONOT_FILL_WITH_BLANK:
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
						   0, buf1,
						   1, buf2,
						   2, i,
						   -1);
    }

	/*Finally put the freaking thing*/
	gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(store));
	g_object_unref (G_OBJECT (store));

	/*Making that the columns are ok*/
	GtkTreeViewColumn* col =
		gtk_tree_view_get_column(tree_view, 0);
	if(col == NULL)
	{
        GtkCellRenderer* ren;
		ren = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes ("I/O", ren,
                                                      "text", 0,
                                                      NULL);
		gtk_tree_view_insert_column (tree_view, column, 0);
	}

	col =
		gtk_tree_view_get_column(tree_view, 1);
	if(col == NULL)
	{
		GtkCellRenderer* ren;
		ren = gtk_cell_renderer_text_new ();
		if(bDoneOutputs) g_object_set(ren, "editable", TRUE, NULL);
		column = gtk_tree_view_column_new_with_attributes ("Value", ren,
														"text", 1,
														NULL);
		gtk_tree_view_insert_column (tree_view, column, 1);
		if(bDoneOutputs) g_signal_connect(ren, "edited", (GCallback) on_tree_outputs_value_edited, NULL);
	}

	if(!bDoneOutputs)
	{
        ncant = outputs;
        tree_view = gMain->tree_outputs;
        bDoneOutputs = 1;
        goto REFILL_DOAGAIN;
	}

    return 1;
}

int main(int argc, char **argv)
{
    GtkBuilder *builder;
    GError     *error = NULL;

	gMain = g_slice_new( SMain );
	printf("Welcome to spix I/O handler!\n");

    /* Init GTK+ */
    gtk_init( &argc, &argv );

    /* Create new GtkBuilder object */
    builder = gtk_builder_new();
    /* Load UI from file. If error occurs, report it and quit application.
     * Replace "tut.glade" with your saved project. */
    if( ! gtk_builder_add_from_file( builder, "spixio.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 0 );
    }

    /* Get main window pointer from UI */
    gMain->window_main = GTK_WIDGET( gtk_builder_get_object( builder, "window_main" ) );
    gMain->about_main = (GtkAboutDialog*)( gtk_builder_get_object( builder, "about_main" ) );

    /* Get main spin buttons from UI */
    gMain->spinbutton_word = (GtkSpinButton*)( gtk_builder_get_object( builder, "spinbutton_word" ) );
    gMain->spinbutton_inputs = (GtkSpinButton*)( gtk_builder_get_object( builder, "spinbutton_inputs" ) );
    gMain->spinbutton_outputs = (GtkSpinButton*)( gtk_builder_get_object( builder, "spinbutton_outputs" ) );

    /* Get main tree views*/
	gMain->tree_inputs = GTK_TREE_VIEW( gtk_builder_get_object( builder, "tree_inputs" ) );
	gMain->tree_outputs = GTK_TREE_VIEW( gtk_builder_get_object( builder, "tree_outputs" ) );

	/* Get main widgets from main */
	gMain->toogleaction_auto = (GtkToggleAction *)( gtk_builder_get_object( builder, "toggleaction_auto" ) );

	/* Get label*/
	gMain->label_main = GTK_LABEL( gtk_builder_get_object( builder, "label_main" ) );

	/* Connect signals */
    gtk_builder_connect_signals( builder, gMain );

    /* Show window. All other widgets are automatically shown by GtkBuilder */
    gtk_widget_show( gMain->window_main );

    /*Fill with info*/
    GSPI_WORD = gtk_spin_button_get_value(gMain->spinbutton_word);
    GSPI_INPUTS = gtk_spin_button_get_value(gMain->spinbutton_inputs);
    GSPI_OUTPUTS = gtk_spin_button_get_value(gMain->spinbutton_outputs);
    gMain->radix = 16;

    if(spi_init())
    {
        gtk_toggle_action_set_active(gMain->toogleaction_auto, TRUE);
        //create_timer_auto_update();   // Not necesary
    }

    update_spi_things();

    /* Start main loop */
    gtk_main();

    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );

    /* Cleanup all things*/
    destroy_timer_auto_update();
    spi_close();

	printf("Sucesfully done!\n");
	return 0;
}

