#include "t_config.h"
#include "t_backend.h"
#include "t_file.h"
#include "t_tune.h"
#include "t_file_compat.h"

static const char libid[] = "does not matter.";

struct t_file *
t_file_new(const char *path)
{
	struct t_tune tune;
	struct t_file *file = NULL;

	assert_not_null(path);

	if (t_tune_init(&tune, path) == 0) {
		T_FILE_NEW(file, path, tune);
		file->libid = tune.backend->libid;
	}

	return (file);
}


void
t_file_destroy(struct t_file *file)
{
	struct t_tune *tune;

	assert_not_null(file);
	assert_not_null(file->data);

	tune = file->data;

	t_tune_clear(tune);
	T_FILE_DESTROY(file);
}


bool
t_file_save(struct t_file *file)
{
	struct t_tune *tune;

	assert_not_null(file);
	assert_not_null(file->data);

	tune = file->data;

	return (t_tune_save(tune) == 0);
}


struct t_taglist *
t_file_get(struct t_file *file, const char *key)
{
	struct t_tune *tune;
	const struct t_tag *t;
	const struct t_taglist *tlist;
	struct t_taglist *ret;

	assert_not_null(file);

	tune  = file->data;
	tlist = t_tune_tags(tune);
	if (key == NULL)
		return (t_taglist_clone(tlist));
	if (tlist == NULL)
		return (NULL);
	if ((ret = t_taglist_new()) == NULL)
		return (NULL);
	TAILQ_FOREACH(t, tlist->tags, entries) {
		if (strcasecmp(t->key, key) == 0) {
			char *k = strdup(t->key);
			if (k == NULL || t_taglist_insert(ret, t_strtolower(k), t->val) == -1) {
				t_taglist_delete(ret);
				return (NULL);
			}
			free(k);
		}
	}
	return (ret);
}
