
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "mnb-launcher-tree.h"

#define ICON_SIZE 48

int
main (int     argc,
      char  **argv)
{
  GtkIconTheme    *theme;
  MnbLauncherTree *tree;
  GSList          *directories;
  GSList const    *directory_iter;

  gtk_init (&argc, &argv);

  theme = gtk_icon_theme_get_default ();
  tree = mnb_launcher_tree_create ();
  directories = mnb_launcher_tree_list_entries (tree);

  for (directory_iter = directories; 
       directory_iter;
       directory_iter = directory_iter->next)
    {
      MnbLauncherDirectory  *directory;
      GSList                *entry_iter;

      directory = (MnbLauncherDirectory *) directory_iter->data;
      printf ("%s\n", directory->name);

      for (entry_iter = directory->entries;
            entry_iter;
            entry_iter = entry_iter->next)
        {
          MnbLauncherEntry  *entry;
          GtkIconInfo       *info;
          const gchar       *generic_name, *icon_name, *icon_file;
          gchar             *exec;
          gboolean           is_fallback;

          entry = entry_iter->data;
          info = NULL;
          icon_file = NULL;
          is_fallback = FALSE;

          generic_name = mnb_launcher_entry_get_name (entry);
          exec = mnb_launcher_entry_get_exec (entry);
          icon_name = mnb_launcher_entry_get_icon (entry);
          if (icon_name)
            {
              info = gtk_icon_theme_lookup_icon (theme,
                                                 icon_name,
                                                 ICON_SIZE,
                                                 GTK_ICON_LOOKUP_GENERIC_FALLBACK);
            }            
          if (!info)
            {
              info = gtk_icon_theme_lookup_icon (theme,
                                                 "application-x-executable", 
                                                 ICON_SIZE,
                                                 GTK_ICON_LOOKUP_GENERIC_FALLBACK);
              is_fallback = TRUE;
            }
          if (info)
              icon_file = gtk_icon_info_get_filename (info);

          if (generic_name && exec && icon_file)
            {
              printf ("  '%s' icon: %s\n", mnb_launcher_entry_get_name (entry),
                                         is_fallback ? "fallback" : icon_file);
            }
          else
            {
              printf ("  ('%s' exec: %s, icon: %s)\n", 
                      generic_name ? generic_name : "*name missing*",
                      exec ? "ok" : "not found",
                      icon_file ? 
                        is_fallback ? 
                          "fallback" :
                          icon_file :
                        "not found");
            }

          if (exec)
            g_free (exec);
          if (info)
            gtk_icon_info_free (info);            
        }
    }

  mnb_launcher_tree_free_entries (directories);
  mnb_launcher_tree_free (tree);

  return EXIT_SUCCESS;
}
