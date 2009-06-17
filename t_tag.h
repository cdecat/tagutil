#ifndef T_TAG_H
#define T_TAG_H
/*
 * t_tag.h
 *
 * tagutil's tag structures/functions.
 */
#include "t_config.h"

#include <sys/queue.h>

#include <stdbool.h>

#include "t_toolkit.h"


/* tag value */
struct ttagv {
    size_t vlen;
    const char *value;
    TAILQ_ENTRY(ttagv) next;
};
TAILQ_HEAD(ttagv_q, ttagv);

/* tag key/values */
struct ttag {
    size_t keylen, vcount;
    const char *key;
    struct ttagv_q *values;
    TAILQ_ENTRY(ttag) next;
};
TAILQ_HEAD(ttag_q, ttag);


/*
 * return the ttagv value at index idx.
 * if idx >= t->vcount (i.e. idx is a bad index) NULL is returned.
 */
_t__unused _t__nonnull(1)
static inline struct ttagv * ttag_value_by_idx(struct ttag *restrict t,
        size_t idx);


/*
 * a tag list, abstract structure for a music file's tags.
 * It's designed to handle several key with each several possibles values.
 *
 * errors macros defined in t_toolkit.h can (and should) be used on this
 * structure.
 */
struct tag_list {
    size_t tcount;
    struct ttag_q *tags;

    ERROR_MSG_MEMBER;
};


/*
 * create a new tag_list.
 * returned value has to be free()d (see destroy_tag_list()).
 */
struct tag_list * new_tag_list(void);

/*
 * insert a new key/value in T.
 * value can be NULL (for example, if you need to use the tag_list for a
 * clear() call).
 */
_t__nonnull(1) _t__nonnull(2)
void tag_list_insert(struct tag_list *restrict T,
        const char *restrict key, const char *restrict value);

/*
 * find a ttag in T matching given key.
 *
 * return the matching ttag if found, or NULL otherwhise.
 */
_t__nonnull(1) _t__nonnull(2)
struct ttag * tag_list_search(const struct tag_list *restrict T,
        const char *restrict key);


/*
 * free the tag_list structure and all the ttag/ttagv.
 */
_t__nonnull(1)
void destroy_tag_list(const struct tag_list *T);

/********************************************************************************/

static inline struct ttagv *
ttag_value_by_idx(struct ttag *restrict t, size_t idx)
{
    struct ttagv *ret;

    assert_not_null(t);

    ret = NULL;
    if (idx < t->vcount) {
        ret = TAILQ_FIRST(t->values);
        while (idx--)
            ret = TAILQ_NEXT(ret, next);
    }

    return (ret);
}
#endif /* not T_TAG_H */
