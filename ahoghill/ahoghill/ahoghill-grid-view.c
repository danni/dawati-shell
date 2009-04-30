#include <config.h>

#include <glib.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <bickley/bkl.h>
#include <bickley/bkl-source-client.h>
#include <bickley/bkl-source-manager-client.h>

#include <bognor/br-queue.h>

#include <src/mnb-entry.h>

#include "ahoghill-grid-view.h"
#include "ahoghill-results-model.h"
#include "ahoghill-results-pane.h"
#include "ahoghill-search-pane.h"

enum {
    PROP_0,
};

typedef struct _Source {
    BklSourceClient *source;
    BklDB *db;

    GPtrArray *items;
    GPtrArray *index;
    GHashTable *uri_to_item;
} Source;

struct _AhoghillGridViewPrivate {
    ClutterActor *search_pane;
    ClutterActor *results_pane;
    ClutterActor *playqueues_pane;

    AhoghillResultsModel *model;
    GtkRecentManager *recent_manager;

    GPtrArray *dbs;

    BrQueue *local_queue;

    BklSourceManagerClient *source_manager;

    guint32 search_id;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AHOGHILL_TYPE_GRID_VIEW, AhoghillGridViewPrivate))
G_DEFINE_TYPE (AhoghillGridView, ahoghill_grid_view, NBTK_TYPE_TABLE);

#define SEARCH_TIMEOUT 500

static void
ahoghill_grid_view_finalize (GObject *object)
{
    G_OBJECT_CLASS (ahoghill_grid_view_parent_class)->finalize (object);
}

static void
ahoghill_grid_view_dispose (GObject *object)
{
    G_OBJECT_CLASS (ahoghill_grid_view_parent_class)->dispose (object);
}

static void
ahoghill_grid_view_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    switch (prop_id) {

    default:
        break;
    }
}

static void
ahoghill_grid_view_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    switch (prop_id) {

    default:
        break;
    }
}

static void
ahoghill_grid_view_class_init (AhoghillGridViewClass *klass)
{
    GObjectClass *o_class = (GObjectClass *) klass;

    o_class->dispose = ahoghill_grid_view_dispose;
    o_class->finalize = ahoghill_grid_view_finalize;
    o_class->set_property = ahoghill_grid_view_set_property;
    o_class->get_property = ahoghill_grid_view_get_property;

    g_type_class_add_private (klass, sizeof (AhoghillGridViewPrivate));
}

static Source *
create_source (BklSourceClient *s)
{
    Source *source;
    GError *error = NULL;
    int i;

    source = g_new0 (Source, 1);

    source->source = s;
    source->db = bkl_source_client_get_db (s);
    if (source->db == NULL) {
        g_warning ("%s: Error getting DB for %s",
                   G_STRLOC, bkl_source_client_get_path (s));

        g_object_unref (source->source);
        g_free (source);
        return NULL;
    }

    source->items = bkl_db_get_items (source->db, FALSE, &error);
    if (source->items == NULL) {
        g_warning ("Error getting items: %s", error->message);
        bkl_db_free (source->db);
        g_free (source);
        return NULL;
    }

    source->uri_to_item = g_hash_table_new (g_str_hash, g_str_equal);
    for (i = 0; i < source->items->len; i++) {
        BklItem *item = source->items->pdata[i];
        g_hash_table_insert (source->uri_to_item,
                             (char *) bkl_item_get_uri (item), item);
    }

    /* Index is a sorted array of strings */
    source->index = bkl_db_get_index_words (source->db);
    return source;
}

static BklItem *
find_item (AhoghillGridView *view,
           const char       *uri)
{
    AhoghillGridViewPrivate *priv = view->priv;
    int i;

    for (i = 0; i < priv->dbs->len; i++) {
        Source *source = priv->dbs->pdata[i];
        BklItem *item;

        item = g_hash_table_lookup (source->uri_to_item, uri);
        if (item) {
            return item;
        }
    }

    return NULL;
}

static void
set_recent_items (AhoghillGridView *view)
{
    AhoghillGridViewPrivate *priv;
    GList *recent_items, *r;

    priv = view->priv;
    priv->source_manager = g_object_new (BKL_TYPE_SOURCE_MANAGER_CLIENT, NULL);
    sources = bkl_source_manager_client_get_sources (priv->source_manager,
                                                     &error);
    if (error != NULL) {
        g_warning ("Error getting sources\n");
    }

    /* +1 for the local */
    priv->dbs = g_ptr_array_sized_new (g_list_length (sources) + 1);

    /* Local source first */
    client = bkl_source_client_new (BKL_LOCAL_SOURCE_PATH);
    source = create_source (client);
    if (source) {
        g_ptr_array_add (priv->dbs, source);
    } else {
        g_warning ("%s: Error, no local database\n", G_STRLOC);
    }

    for (s = sources; s; s = s->next) {
        client = bkl_source_client_new (s->data);
        source = create_source (client);

        if (source) {
            g_ptr_array_add (priv->dbs, source);
        }
    }

    recent_items = gtk_recent_manager_get_items (priv->recent_manager);
    if (recent_items) {
        /* Freeze and clear the old results before adding anything new */
        ahoghill_results_model_freeze (priv->model);
        ahoghill_results_model_clear (priv->model);

        for (r = recent_items; r; r = r->next) {
            GtkRecentInfo *info = r->data;
            const char *mimetype;
            const char *uri;
            BklItem *item;

            mimetype = gtk_recent_info_get_mime_type (info);
            if (g_str_has_prefix (mimetype, "audio/") == FALSE &&
                g_str_has_prefix (mimetype, "video/") == FALSE &&
                g_str_has_prefix (mimetype, "image/") == FALSE) {
                gtk_recent_info_unref (info);
                continue;
            }

            uri = gtk_recent_info_get_uri (info);

            item = find_item (view, uri);
            if (item == NULL) {
                gtk_recent_info_unref (info);
                continue;
            }

            ahoghill_results_model_add_item (priv->model, item);

            gtk_recent_info_unref (info);
        }

        g_list_free (recent_items);

        /* And thaw the results */
        ahoghill_results_model_thaw (priv->model);
    }
}

static void
init_bickley (gpointer data)
{
    AhoghillGridView *view = (AhoghillGridView *) data;
    AhoghillGridViewPrivate *priv = view->priv;
    BklSourceClient *client;
    Source *source;
    GList *sources, *s;
    GError *error = NULL;

    priv->source_manager = g_object_new (BKL_TYPE_SOURCE_MANAGER_CLIENT, NULL);
    sources = bkl_source_manager_client_get_sources (priv->source_manager,
                                                     &error);
    if (error != NULL) {
        g_warning ("Error getting sources\n");
    }

    priv->dbs = g_ptr_array_sized_new (g_list_length (sources) + 1);

    /* Local source first */
    client = bkl_source_client_new (BKL_LOCAL_SOURCE_PATH);
    source = create_source (client);
    g_ptr_array_add (priv->dbs, source);

    for (s = sources; s; s = s->next) {
        client = bkl_source_client_new (s->data);
        source = create_source (client);

        g_ptr_array_add (priv->dbs, source);
    }

    set_recent_items (view);
}

static void
init_bognor (AhoghillGridView *grid)
{
    AhoghillGridViewPrivate *priv = grid->priv;

    priv->local_queue = g_object_new (BR_TYPE_QUEUE,
                                      "object-path", BR_LOCAL_QUEUE_PATH,
                                      NULL);
}

static gboolean
finish_init (gpointer data)
{
    AhoghillGridView *grid = data;

    init_bognor (grid);
    init_bickley (grid);

    return FALSE;
}

struct _QueryData {
    int word_count;
    GList *results;
};

static void
get_results (gpointer key,
             gpointer value,
             gpointer userdata)
{
    struct _QueryData *qd = (struct _QueryData *) userdata;
    int count = GPOINTER_TO_INT (value);

    if (count == qd->word_count) {
        qd->results = g_list_prepend (qd->results, key);
    }
}

static GList *
search_for_words (KozoDB *db,
                  char  **words)
{
    struct _QueryData *qd;
    GError *error = NULL;
    GHashTable *set;
    int i;

    set = g_hash_table_new (g_str_hash, g_str_equal);
    for (i = 0; words[i]; i++) {
        GList *results, *r;

        results = kozo_db_index_lookup (db, words[i], &error);
        if (error) {
            g_warning ("Error searching for %s in %s: %s", words[i],
                       kozo_db_get_name (db), error->message);
            g_error_free (error);

            continue;
        }

        for (r = results; r; r = r->next) {
            gpointer key, value;

            if (g_hash_table_lookup_extended (set, r->data, &key, &value)) {
                guint count = GPOINTER_TO_INT (value);
                g_hash_table_insert (set, r->data, GINT_TO_POINTER (count + 1));
            } else {
                g_hash_table_insert (set, r->data, GINT_TO_POINTER (1));
            }
        }
    }

    qd = g_newa (struct _QueryData, 1);
    qd->word_count = i;
    qd->results = NULL;

    g_hash_table_foreach (set, get_results, qd);
    g_hash_table_destroy (set);

    return qd->results;
}

#if 0
static void
search_clicked_cb (MnbEntry         *entry,
                   AhoghillGridView *grid)
{
    AhoghillGridViewPrivate *priv = grid->priv;
    const char *text;
    char *search_text;
    char **search_words;
    GPtrArray *items;
    int i;

    text = mnb_entry_get_text (entry);
    if (text == NULL) {
        return;
    }

    search_text = g_ascii_strup (text, -1);
    search_words = g_strsplit (search_text, " ", -1);

    g_object_set (G_OBJECT (priv->results_pane),
                  "title", _("Results"),
                  NULL);

    items = g_ptr_array_new ();
    for (i = 0; i < priv->dbs->len; i++) {
        Source *source = priv->dbs->pdata[i];
        GList *results;

        results = search_for_words (source->db->db, search_words);

        if (results) {
            GList *r;

            for (r = results; r; r = r->next) {
                BklItem *item;

                item = g_hash_table_lookup (source->uri_to_item, r->data);
                if (item != NULL) {
                    g_ptr_array_add (items, item);
                }

                g_free (r->data);
            }

            g_list_free (results);
        }
    }

    g_strfreev (search_words);
    g_free (search_text);

    ahoghill_results_pane_set_results
        (AHOGHILL_RESULTS_PANE (priv->results_pane), items);

    g_ptr_array_free (items, TRUE);
}
#endif

static void
item_clicked_cb (AhoghillResultsPane *pane,
                 BklItem             *item,
                 AhoghillGridView    *grid)
{
    AhoghillGridViewPrivate *priv = grid->priv;
    GError *error = NULL;

    if (bkl_item_get_item_type (item) != BKL_ITEM_TYPE_AUDIO) {
        return;
    }

    br_queue_add_uri (priv->local_queue, bkl_item_get_uri (item), &error);
    if (error != NULL) {
        g_warning ("%s: Error adding %s to queue: %s", G_STRLOC,
                   bkl_item_get_uri (item), error->message);
        g_error_free (error);
        return;
    }

    br_queue_play (priv->local_queue, &error);
    if (error != NULL) {
        g_warning ("%s: Error playing local queue: %s", G_STRLOC,
                   error->message);
        g_error_free (error);
    }
}

static gboolean
do_search_cb (gpointer data)
{
    AhoghillGridView *view = (AhoghillGridView *) data;
    AhoghillGridViewPrivate *priv = view->priv;
    NbtkWidget *entry;
    const char *text;

    entry = ahoghill_search_pane_get_entry (AHOGHILL_SEARCH_PANE (priv->search_pane));
    text = mnb_entry_get_text ((MnbEntry *) entry);

    if (text == NULL || *text == 0) {
        g_print ("Setting recent items\n");
        set_recent_items (view);
    } else {
        g_print ("Searching for %s\n", text);
    }

    priv->search_id = 0;
    return FALSE;
}

static void
search_text_changed (MnbEntry         *entry,
                     AhoghillGridView *view)
{
    AhoghillGridViewPrivate *priv;

    priv = view->priv;
    if (priv->search_id != 0) {
        /* Reset timeout */
        g_source_remove (priv->search_id);
    }

    priv->search_id = g_timeout_add (SEARCH_TIMEOUT, do_search_cb, view);
}

static void
ahoghill_grid_view_init (AhoghillGridView *self)
{
    NbtkTable *table = (NbtkTable *) self;
    AhoghillGridViewPrivate *priv = GET_PRIVATE (self);
    NbtkWidget *entry;
    ClutterColor blue = { 0x00, 0x00, 0xff, 0xff };

    bkl_init ();

    priv->model = g_object_new (AHOGHILL_TYPE_RESULTS_MODEL, NULL);

    clutter_actor_set_name (CLUTTER_ACTOR (self), "media-pane-vbox");
    clutter_actor_set_size (CLUTTER_ACTOR (self), 1024, 500);

    self->priv = priv;

    priv->recent_manager = gtk_recent_manager_get_default ();

    priv->search_pane = g_object_new (AHOGHILL_TYPE_SEARCH_PANE, NULL);
    clutter_actor_set_size (priv->search_pane, 1024, 50);
    nbtk_table_add_actor_with_properties (table, priv->search_pane,
                                          0, 0,
                                          "col-span", 4,
                                          "x-fill", FALSE,
                                          "y-expand", FALSE,
                                          "y-fill", FALSE,
                                          "x-align", 0.0,
                                          "y-align", 0.0,
                                          NULL);

    entry = ahoghill_search_pane_get_entry (AHOGHILL_SEARCH_PANE (priv->search_pane));
    g_signal_connect (entry, "text-changed",
                      G_CALLBACK (search_text_changed), self);

    priv->results_pane = (ClutterActor *) ahoghill_results_pane_new (priv->model);
    g_object_set (priv->results_pane,
                  "title", _("Recent"),
                  NULL);
    clutter_actor_set_size (priv->results_pane, 800, 400);
    nbtk_table_add_actor_with_properties (table, priv->results_pane,
                                          1, 0,
                                          "row-span", 1,
                                          "col-span", 3,
                                          "x-align", 0.0,
                                          "y-align", 0.0,
                                          NULL);
    g_signal_connect (priv->results_pane, "item-clicked",
                      G_CALLBACK (item_clicked_cb), self);

    /* priv->playqueues_pane = g_object_new (AHOGHILL_TYPE_PLAYQUEUES_PANE, */
    /*                                        NULL); */
    priv->playqueues_pane = clutter_rectangle_new_with_color (&blue);
    clutter_actor_set_size (priv->playqueues_pane, 150, 400);
    nbtk_table_add_actor_with_properties (table, priv->playqueues_pane,
                                          1, 3,
                                          "x-expand", FALSE,
                                          "x-fill", FALSE,
                                          "y-expand", FALSE,
                                          "x-align", 0.0,
                                          "y-align", 0.0,
                                          NULL);

    /* Init Bickley and Bognor lazily */
    g_idle_add (finish_init, self);
}

void
ahoghill_grid_view_clear (AhoghillGridView *view)
{
    AhoghillGridViewPrivate *priv = view->priv;
    NbtkWidget *entry;

    entry = ahoghill_search_pane_get_entry (AHOGHILL_SEARCH_PANE (priv->search_pane));
    mnb_entry_set_text (MNB_ENTRY (entry), "");
}

void
ahoghill_grid_view_focus (AhoghillGridView *view)
{
    AhoghillGridViewPrivate *priv = view->priv;
    NbtkWidget *entry;

    entry = ahoghill_search_pane_get_entry (AHOGHILL_SEARCH_PANE (priv->search_pane));
    clutter_actor_grab_key_focus (CLUTTER_ACTOR (entry));
}

void
ahoghill_grid_view_unfocus (AhoghillGridView *view)
{
    AhoghillGridViewPrivate *priv = view->priv;
    ClutterStage *stage = CLUTTER_STAGE (clutter_stage_get_default ());
    ClutterActor *entry;
    ClutterActor *current_focus;

    /*
     * If the entry is focused, than release the focus.
     */
    entry = CLUTTER_ACTOR (
     ahoghill_search_pane_get_entry (AHOGHILL_SEARCH_PANE (priv->search_pane)));

    current_focus = clutter_stage_get_key_focus (stage);

    if (current_focus == entry)
      clutter_stage_set_key_focus (stage, NULL);
}
