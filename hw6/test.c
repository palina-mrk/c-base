#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

# define BUFFER 1000

enum
{
   ITEM_COLUMN,
   N_COLUMNS
};

//рекурсивный обход директориий и запись имён в дерево
static void
build_subdirectories (GtkTreeStore* store, 
                      GtkTreeIter*  parent_iter,
                      GDir*         current_dir){
  GtkTreeIter current_iter;
  GDir*  subdir;
  gchar* next_record_name;
  gchar* current_path = g_get_current_dir();
  
  /*write the next record*/
  while((next_record_name = g_dir_read_name(current_dir)) != NULL){
    /*записываем очередное имя*/
    gtk_tree_store_append (store, &current_iter, parent_iter); 
    gtk_tree_store_set (store, &current_iter,
                        ITEM_COLUMN, next_record_name,
                        -1);

    /*если это директория, нужно туда зайти и сделать рекурсивный вызов*/
    if(g_file_test(next_record_name, G_FILE_TEST_IS_DIR)){
      subdir = g_dir_open(next_record_name, 0, NULL);
      g_chdir (next_record_name);
      build_subdirectories(store, &current_iter, subdir);
      g_chdir (current_path);
      g_dir_close (subdir);
    }
    
  }
  return;
}

static void
populate_tree_model (GtkTreeStore *store){
  gchar* current_dir_path;
  GDir*  current_dir;

  // получаем указатель на текущую директорию
  current_dir_path = g_get_current_dir();
  current_dir = g_dir_open(current_dir_path, 0, NULL);
  free(current_dir_path);
 
  // вызываем рекурсивную функцию для заполнения дерева поддиректориями
  build_subdirectories(store, NULL, current_dir);
  g_dir_close(current_dir);
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkTreeStore *store;
  GtkWidget *tree;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  char* init_path = (char*) g_get_current_dir();

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);


   /* Create a model.  We are using the store model for now, though we
    * could use any other GtkTreeModel */
   store = gtk_tree_store_new (N_COLUMNS,
                               G_TYPE_STRING );

   /* Create a view */
   tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

   /* The view now holds a reference.  We can get rid of our own
    * reference */
   g_object_unref (G_OBJECT (store));

   /* Create a cell render and arbitrarily make it red for demonstration
    * purposes */
   renderer = gtk_cell_renderer_text_new ();
   g_object_set (G_OBJECT (renderer),
                 "foreground", "blue",
                 NULL);

   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   column = gtk_tree_view_column_new_with_attributes (init_path, renderer,
                                                      "text", ITEM_COLUMN,
                                                      NULL);
   /* custom function to fill the model with data */
   populate_tree_model (store);
   /* Add the column to the view. */
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

   /* Now we can manipulate the view just like any other GTK widget */
   //...
  gtk_window_set_child (GTK_WINDOW (window), tree);
  gtk_window_present (GTK_WINDOW (window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
