/*
 * t_ftgeneric.c
 *
 * a generic tagutil backend, using TagLib.
 */
#include <limits.h>
#include <stdbool.h>
#include <string.h>

/* TagLib headers */
#include <tag_c.h>

#include "t_config.h"
#include "t_toolkit.h"
#include "t_file.h"
#include "t_ftgeneric.h"


struct t_ftgeneric_data {
    TagLib_File *file;
    TagLib_Tag  *tag;
};

_t__nonnull(1)
void t_ftgeneric_destroy(struct t_file *restrict self);
_t__nonnull(1)
bool t_ftgeneric_save(struct t_file *restrict self);

_t__nonnull(1)
struct tag_list * t_ftgeneric_get(struct t_file *restrict self,
        const char *restrict key);
_t__nonnull(1)
bool t_ftgeneric_clear(struct t_file *restrict self, const struct tag_list *restrict T);
_t__nonnull(1) _t__nonnull(2)
bool t_ftgeneric_add(struct t_file *restrict self, const struct tag_list *restrict T);


void
t_ftgeneric_destroy(struct t_file *restrict self)
{
    struct t_ftgeneric_data *d;

    assert_not_null(self);
    assert_not_null(self->data);

    d = self->data;
    taglib_file_free(d->file);
    t_error_clear(self);
    freex(self);
}


bool
t_ftgeneric_save(struct t_file *restrict self)
{
    bool ok;
    struct t_ftgeneric_data *d;

    assert_not_null(self);
    assert_not_null(self->data);
    t_error_clear(self);

    d = self->data;
	ok = taglib_file_save(d->file);
    if (!ok)
        t_error_set(self, "%s error", self->lib);
    return (ok);
}


static const char * _taglibkeys[] = {
    "album", "artist", "comment", "date", "genre", "title", "tracknumber"
};
struct tag_list *
t_ftgeneric_get(struct t_file *restrict self, const char *restrict key)
{
    int i;
    unsigned int uintval;
    struct t_ftgeneric_data *d;
    struct tag_list *T;
    char *value;

    assert_not_null(self);
    assert_not_null(self->data);
    t_error_clear(self);

    d = self->data;
    T = new_tag_list();

    for (i = 0; i < 7; i++) {
        if (key) {
            if (strcasecmp(key, _taglibkeys[i]) != 0)
                continue;
        }
        value = NULL;
        switch (i) {
        case 0:
            value = taglib_tag_album(d->tag);
            break;
        case 1:
            value = taglib_tag_artist(d->tag);
            break;
        case 2:
            value = taglib_tag_comment(d->tag);
            break;
        case 3:
            uintval = taglib_tag_year(d->tag);
            if (uintval > 0)
                (void)xasprintf(&value, "%04u", taglib_tag_year(d->tag));
            break;
        case 4:
            value = taglib_tag_genre(d->tag);
            break;
        case 5:
            value = taglib_tag_title(d->tag);
            break;
        case 6:
            uintval = taglib_tag_track(d->tag);
            if (uintval > 0)
                (void)xasprintf(&value, "%02u", uintval);
            break;
        }
        if (value && strempty(value))
        /* clean value, when TagLib return "" we return NULL */
            freex(value);

        if (value) {
            tag_list_insert(T, _taglibkeys[i], value);
            freex(value);
        }
        if (key)
            break;
    }

    return (T);
}


bool
t_ftgeneric_clear(struct t_file *restrict self, const struct tag_list *restrict T)
{
    int i;
    struct t_ftgeneric_data *d;

    assert_not_null(self);
    assert_not_null(self->data);
    t_error_clear(self);

    d = self->data;

    for (i = 0; i < 7; i++) {
        if (T && tag_list_search(T, _taglibkeys[i]) == NULL)
            continue;
        switch (i) {
        case 0:
            taglib_tag_set_album(d->tag, "");
            break;
        case 1:
            taglib_tag_set_artist(d->tag, "");
            break;
        case 2:
            taglib_tag_set_comment(d->tag, "");
            break;
        case 3:
            taglib_tag_set_year(d->tag, 0);
            break;
        case 4:
            taglib_tag_set_genre(d->tag, "");
            break;
        case 5:
            taglib_tag_set_title(d->tag, "");
            break;
        case 6:
            taglib_tag_set_track(d->tag, 0);
            break;
        }
    }

    return (true);
}


bool
t_ftgeneric_add(struct t_file *restrict self, const struct tag_list *restrict T)
{
    struct t_ftgeneric_data *d;
    unsigned int uintval;
    char *endptr;
    struct ttag  *t;
    struct ttagv *v;
    bool strfunc;
    void (*strf)(TagLib_Tag *, const char *);
    void (*uif)(TagLib_Tag *, unsigned int);

    assert_not_null(T);
    assert_not_null(self);
    assert_not_null(self->data);
    t_error_clear(self);

    d = self->data;

    TAILQ_FOREACH(t, T->tags, next) {
        /* detect key function to use */
        strfunc = true;
        assert_not_null(t->key);
        if (strcmp(t->key, "artist") == 0)
            strf = taglib_tag_set_artist;
        else if (strcmp(t->key, "album") == 0)
            strf = taglib_tag_set_album;
        else if (strcmp(t->key, "comment") == 0)
            strf = taglib_tag_set_comment;
        else if (strcmp(t->key, "genre") == 0)
            strf = taglib_tag_set_genre;
        else if (strcmp(t->key, "title") == 0)
            strf = taglib_tag_set_title;
        else {
            strfunc = false;
            if (strcmp(t->key, "tracknumber") == 0)
                uif = taglib_tag_set_track;
            else if (strcmp(t->key, "date") == 0)
                uif = taglib_tag_set_year;
            else {
                t_error_set(self,
                        "%s backend can't handle `%s' tag", self->lib, t->key);
                return (false);
            }
        }

        if (t->vcount != 1) {
            t_error_set(self,
                    "%s backend can only set one tag %s, got %zd",
                    self->lib, t->key, t->vcount);
            return (false);
        }
        v = TAILQ_FIRST(t->values);
        assert_not_null(v->value);
        if (strfunc)
            strf(d->tag, v->value);
        else {
            const char *msg;
            uintval = (unsigned int)strtonum(v->value, 0, UINT_MAX, &msg);
            if (msg) {
                t_error_set(self, "need Int argument for %s, got: `%s' (%s)",
                        t->key, v->value, msg);
                return (false);
            }
            uif(d->tag, uintval);
        }
    }

    return (true);
}


void
t_ftgeneric_init(void)
{
    char *lcall, *dot;

    /* TagLib specific init */
    lcall = getenv("LC_ALL");
    dot = strchr(lcall, '.');
    if (dot && strcmp(dot + 1, "UTF-8"))
        taglib_set_strings_unicode(true);

    taglib_set_string_management_enabled(false);
}


struct t_file *
t_ftgeneric_new(const char *restrict path)
{
    TagLib_File *f;
    struct t_file *ret;
    size_t size;
    char *s;
    struct t_ftgeneric_data *d;

    assert_not_null(path);

    f = taglib_file_new(path);
    if (f == NULL || !taglib_file_is_valid(f))
        return (NULL);

    size = (strlen(path) + 1) * sizeof(char);
    ret = xmalloc(sizeof(struct t_file) + sizeof(struct t_ftgeneric_data) + size);

    d = (struct t_ftgeneric_data *)(ret + 1);
    d->file  = f;
    d->tag   = taglib_file_tag(f);
    ret->data = d;

    s = (char *)(d + 1);
    (void)strlcpy(s, path, size);
    ret->path = s;

    ret->create   = t_ftgeneric_new;
    ret->save     = t_ftgeneric_save;
    ret->destroy  = t_ftgeneric_destroy;
    ret->get      = t_ftgeneric_get;
    ret->clear    = t_ftgeneric_clear;
    ret->add      = t_ftgeneric_add;

    ret->lib = "TagLib";
    t_error_init(ret);
    return (ret);
}

