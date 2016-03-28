/* $Id$ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005-2009  Markus Dahms <mad@automagically.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdlib.h>
#include <sys/param.h>	/* PATH_MAX */

#include <string.h>
#include <unistd.h>

#include <g3d/config.h>
#include <g3d/stream.h>
#include <g3d/types.h>
#include <g3d/plugins.h>

#include <curl/curl.h>

static void plugins_free_plugin(G3DPlugin *plugin)
{
	if(plugin->name)
		g_free(plugin->name);
	if(plugin->path)
		g_free(plugin->path);
	if(plugin->extensions)
		g_strfreev(plugin->extensions);

	if(plugin->module)
		g_module_close(plugin->module);

	g_free(plugin);
}

#define PLUGIN_GET_SYMBOL(symbol, pointer) \
	do { \
		if(g_module_symbol(plugin->module, symbol, \
			(gpointer *)&(pointer)) != TRUE) \
			pointer = NULL; \
	} while(0);

static gboolean plugins_loaddirectory(G3DContext *context,
	const gchar *dirname)
{
	GDir *plugindir;
	G3DPlugin *plugin;
	gchar **ext, **exts;
	gchar *path;
	const gchar *filename;
	guint32 type = G3D_PLUGIN_UNKNOWN;

	plugindir = g_dir_open(dirname, 0, NULL);
	if(!plugindir)
		return FALSE;

	while((filename = g_dir_read_name(plugindir)) != NULL) {
#ifdef G_OS_WIN32
		if(g_strcasecmp(filename + strlen(filename) - 4, ".dll") == 0) {
#else
/*		if(g_strcasecmp(filename + strlen(filename) - 3, ".la") == 0) { */
		if(g_strcasecmp(filename + strlen(filename) - 3, ".so") == 0) {
#endif
			plugin = g_new0(G3DPlugin, 1);

			plugin->name = g_strdup(filename);
			plugin->path = g_strdup(dirname);

			path = g_strdup_printf("%s%c%s", dirname, G_DIR_SEPARATOR, filename);

			g_warning("libg3d: plugins: loading %s", path);
			plugin->module = g_module_open(path, 0);
			if(plugin->module == NULL) {
				g_warning("libg3d: plugins: failed to load %s: %s", path, g_module_error());

				plugins_free_plugin(plugin);
			} else {
				PLUGIN_GET_SYMBOL("plugin_extensions", plugin->ext_func);
				PLUGIN_GET_SYMBOL("plugin_description", plugin->desc_func);
				PLUGIN_GET_SYMBOL("plugin_init", plugin->init_func);
				PLUGIN_GET_SYMBOL("plugin_cleanup", plugin->cleanup_func);
				PLUGIN_GET_SYMBOL("plugin_load_model_from_stream",
					plugin->loadmodelstream_func);
				PLUGIN_GET_SYMBOL("plugin_load_image_from_stream",
					plugin->loadimagestream_func);
				
				if (plugin->loadimagestream_func)
				    type = G3D_PLUGIN_IMAGE;
				if (plugin->loadmodelstream_func)
				    type = G3D_PLUGIN_IMPORT;
				plugin->type = type;

				/* append plugin to list */
				context->plugins = g_slist_append(context->plugins, plugin);

				/* handle managed extensions */
				if(plugin->ext_func) {
					ext = exts = plugin->ext_func(context);
					while(*ext != NULL) {
						if(plugin->type == G3D_PLUGIN_IMAGE)
							g_hash_table_insert(context->exts_image,
								*ext, plugin);
						else if(plugin->type == G3D_PLUGIN_IMPORT)
							g_hash_table_insert(context->exts_import,
								*ext, plugin);

						ext ++;
					}
					plugin->extensions = exts;
				}

				/* init plugin if possible */
				if(plugin->init_func)
				{
					plugin->user_data = plugin->init_func(context);
				}

			} /* plugin->module != NULL */

			g_free(path);
		} /* .la file */

	} /* while g_dir_read_name(...) */

	g_dir_close(plugindir);

	return TRUE;
}

#undef PLUGIN_GET_SYMBOL

#ifdef USE_LIBMAGIC

static gboolean plugins_magic_init(G3DContext *context)
{
	context->magic_cookie = magic_open(
		MAGIC_SYMLINK
#if 0
		| MAGIC_CHECK
#endif
#if DEBUG > 2
		| MAGIC_DEBUG
#endif
		);

#if DEBUG > 0
	g_debug("checking and loading %s", MAGIC_FILENAME);
#endif

	if(context->magic_cookie == NULL) {
		g_warning("magic_open() failed");
		return FALSE;
	}

	if(magic_load(context->magic_cookie, MAGIC_FILENAME) != 0) {
		g_warning("magic_load() failed: %s (%d)",
			magic_error(context->magic_cookie),
			magic_errno(context->magic_cookie));
		magic_close(context->magic_cookie);
		context->magic_cookie = NULL;
		return FALSE;
	}

	return TRUE;
}

static void plugins_magic_cleanup(G3DContext *context)
{
	if(context->magic_cookie != NULL)
		magic_close(context->magic_cookie);
	context->magic_cookie = NULL;
}

static G3DPlugin *plugins_magic_lookup(G3DContext *context,
	const gchar *filename)
{
	gchar *name;
	const gchar *type;
	GSList *item;
	G3DPlugin *tp, *plugin = NULL;

	if(context->magic_cookie == NULL)
		return NULL;

	type = magic_file(context->magic_cookie, filename);
	if((type == NULL) || (strlen(type) == 0))
		return NULL;

	name = g_strdup_printf("imp_%s.la", type);
	for(item = context->plugins; item != NULL; item = item->next) {
		tp = (G3DPlugin *)item->data;
		if(tp->type != G3D_PLUGIN_IMPORT)
			continue;

		if(strcmp(name, tp->name) == 0)
			plugin = tp;
	}
	g_free(name);

#if DEBUG > 0
	if(plugin != NULL)
		g_debug("libmagic detected plugin %s for %s",
			plugin->name, filename);
#endif

	return plugin;
}

#endif /* USE_LIBMAGIC */

EAPI
gboolean g3d_plugins_init(G3DContext *context)
{
	context->exts_import = g_hash_table_new(g_str_hash, g_str_equal);
	context->exts_image = g_hash_table_new(g_str_hash, g_str_equal);

#ifdef G_OS_WIN32
	plugins_loaddirectory(context, "plugins");
#else
	plugins_loaddirectory(context, PLUGIN_DIR);
#endif

#ifdef USE_LIBMAGIC
	plugins_magic_init(context);
#endif

	return TRUE;
}

/**
 * g3d_plugins_cleanup:
 * @context: an initialized context
 *
 * Tries to free any memory allocated during g3d_plugins_init.
 */

EAPI
void g3d_plugins_cleanup(G3DContext *context)
{
	GSList *plist;
	G3DPlugin *plugin;
	gchar **pext;

#ifdef USE_LIBMAGIC
	plugins_magic_cleanup(context);
#endif

	plist = context->plugins;
	while(plist)
	{
		plugin = (G3DPlugin *)plist->data;

#if DEBUG > 2
		g_debug("cleaning up plugin '%s'", plugin->name);
#endif

		/* cleanup plugin-specific data */
		if(plugin->cleanup_func)
			plugin->cleanup_func(plugin->user_data);

		/* remove extensions from hash tables */
		pext = plugin->extensions;
		while(*pext)
		{
			if(plugin->type == G3D_PLUGIN_IMAGE)
				g_hash_table_remove(context->exts_image, *pext);
			else if(plugin->type == G3D_PLUGIN_IMPORT)
				g_hash_table_remove(context->exts_import, *pext);

			pext ++;
		}

		/* cleanup struct data */
		plugins_free_plugin(plugin);

		/* free list item */
		plist = g_slist_remove(plist, plugin);
	}

	/* remove hash tables */
	g_hash_table_destroy(context->exts_image);
	g_hash_table_destroy(context->exts_import);
}

gchar *g3d_plugins_get_filetype(const gchar *filename)
{
	gchar *ext, *lcext;

	ext = strrchr(filename, '.');
	if(ext == NULL) {
		g_warning("can't determine file type: %s", filename);
		return NULL;
	}
	ext ++;
	if(strlen(ext) == 0) {
		g_warning("zero length file extension: %s", filename);
		return NULL;
	}

	lcext = g_strdup(ext);
	g_strdown(lcext);

	return lcext;
}

static G3DPlugin *get_plugin_for_type(G3DContext *context,
	const gchar *filename)
{
	G3DPlugin *plugin = NULL;
	gchar *lcext;

#ifdef USE_LIBMAGIC
	plugin = plugins_magic_lookup(context, filename);
#endif

	if(plugin == NULL)
	{
		/* try to get type by extension */
		lcext = g3d_plugins_get_filetype(filename);
		if(lcext == NULL)
			return NULL;

		plugin = g_hash_table_lookup(context->exts_import, lcext);

		g_free(lcext);
	}

	if(plugin == NULL)
	{
		g_warning("no handler for file '%s' found", filename);
		return NULL;
	}

	return plugin;
}

struct httpBuffer
{
    void *buffer;
    size_t size;
    size_t max;
};

static size_t curlWrite(void *ptr, size_t size, size_t nmemb, void *userData)
{
	struct httpBuffer *result = (struct httpBuffer *) userData;
	size_t bytes = size * nmemb;

	if ((result->size + bytes) > result->max)
	{
	    result->max    += bytes;
	    result->buffer = realloc(result->buffer, result->max);
	}
	if (result->buffer)
	{
	    memcpy(result->buffer + result->size, ptr, bytes);
	    result->size += bytes;
	}
	else
	    g_warning("curlWrite() ran out of memory.");

	return nmemb;
}

EAPI
gboolean g3d_plugins_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model)
{
	G3DPlugin *plugin = NULL;
	G3DStream *stream;
	gboolean retval = FALSE;

	plugin = get_plugin_for_type(context, filename);
	if(plugin == NULL)
		return FALSE;

	if (strncmp(filename, "http://", 6) == 0)
	{
	    struct httpBuffer result;
#if 0
	    char curlErrorBuffer[CURL_ERROR_SIZE];
	    CURLcode curlSuccess;
	    long httpStatus = 499;
	    CURL* curlp = curl_easy_init();

	    result.size   = 0;
	    result.max    = 0;
	    result.buffer = NULL;

	    curl_easy_setopt(curlp, CURLOPT_NOSIGNAL, 1);	/* don't use SIGALRM for timeouts */
	    curl_easy_setopt(curlp, CURLOPT_TIMEOUT, 30);	/* seconds */
	    curl_easy_setopt(curlp, CURLOPT_WRITEFUNCTION, curlWrite);
	    curl_easy_setopt(curlp, CURLOPT_WRITEDATA, &result);
	    curl_easy_setopt(curlp, CURLOPT_URL, filename);
	    curl_easy_setopt(curlp, CURLOPT_ERRORBUFFER, curlErrorBuffer);
	    curl_easy_setopt(curlp, CURLOPT_FAILONERROR, 1);

	    curlSuccess = curl_easy_perform(curlp);
	    curl_easy_getinfo(curlp, CURLINFO_RESPONSE_CODE, &httpStatus);
	    if (curlSuccess != 0)
		g_warning("CURL ERROR (HTTP Status %ld): %s", httpStatus, curlErrorBuffer);
	    else if (httpStatus != 200)
		g_warning("HTTP Error %ld, but no Curl error.", httpStatus);
	    else
		g_debug("CURL got the file %s", filename);

	    stream = g3d_stream_from_buffer(result.buffer, result.size, filename, FALSE);
	    if(stream)
	    {
		model->stream = stream;
		retval = plugin->loadmodelstream_func(context, stream, model, plugin->user_data);
		g3d_stream_close(stream);
	    }
	    else
		g_warning("failed to open '%s'", filename);

	    curl_easy_cleanup(curlp);
	    if (result.buffer)
		free(result.buffer);
#endif
	}
	else /* It's a file. */
	{
	    gchar *basename, *dirname;
	    gchar *olddir;

	    basename = g_path_get_basename(filename);
	    dirname = g_path_get_dirname(filename);

	    olddir = g_get_current_dir();
	    /* TODO: since glib 2.8 there is a g_chdir() wrapper, use it if
	     * for some reason a glib >= 2.8 is required */
	    chdir(dirname);

	    if(plugin->loadmodelstream_func != NULL) {
		    /* try to load the model via the more generic G3DStream interface */
		    stream = g3d_stream_open_file(basename, "rb");
		    if(stream) {
			    model->stream = stream;
			    retval = plugin->loadmodelstream_func(context, stream, model,
				    plugin->user_data);
			    g3d_stream_close(stream);
		    }
		    else {
			    g_warning("failed to open '%s'", basename);
		    }
	    }
	    else
		g_warning("No plugin_load_model_from_stream() for '%s'", basename);
	    g_free(basename);
	    g_free(dirname);

	    chdir(olddir);
	    g_free(olddir);
	}

	    if(retval)
		    model->plugin = plugin;

	return retval;
}

EAPI
gboolean g3d_plugins_load_model_from_stream(G3DContext *context,
	G3DStream *stream, G3DModel *model)
{
	G3DPlugin *plugin = NULL;

/*	g_return_val_if_fail(stream != NULL, FALSE); */

	plugin = get_plugin_for_type(context, stream->uri);
	if(plugin == NULL)
		return FALSE;

	if((plugin->loadmodelstream_func != NULL) && plugin->loadmodelstream_func(
		context, stream, model, plugin->user_data)) {
		model->plugin = plugin;
		return TRUE;
	}
	return FALSE;
}

EAPI
gboolean g3d_plugins_load_image_from_stream(G3DContext *context,
	G3DStream *stream, G3DImage *image)
{
	G3DPlugin *plugin;
	gchar *lcext;

	lcext = g3d_plugins_get_filetype(stream->uri);
    if(lcext == NULL)
		return FALSE;

	plugin = g_hash_table_lookup(context->exts_image, lcext);
	if(plugin == NULL) {
		g_warning("no handler for filetype '.%s' found", lcext);
		g_free(lcext);
		return FALSE;
	}
	g_free(lcext);

	if(plugin->loadimagestream_func == NULL) {
		g_warning("can't find symbol 'plugin_load_image_from_stream' in %s",
			plugin->name);
		return FALSE;
	}

	return plugin->loadimagestream_func(context, stream, image,
		plugin->user_data);
}

struct plugins_ext_list {
	guint32 pos;
	gchar **list;
};

static void plugins_list_ext(gpointer key, gpointer value, gpointer user_data)
{
	struct plugins_ext_list *data;

	data = (struct plugins_ext_list *)user_data;

	data->list[data->pos] = g_strdup(key);
	data->pos ++;
}

EAPI
gchar **g3d_plugins_get_image_extensions(G3DContext *context)
{
	gchar **list;
	guint32 n;
	struct plugins_ext_list data;

	n = g_hash_table_size(context->exts_image);
	list = g_new0(gchar *, n + 1);

	data.pos = 0;
	data.list = list;

	/* fill extension list */
	g_hash_table_foreach(context->exts_image, plugins_list_ext, &data);

	return list;
}

