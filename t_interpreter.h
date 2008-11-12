#ifndef T_INTERPRETER_H
#define T_INTERPRETER_H
/*
 * t_interpreter.c
 *
 * the tagutil's filter interpreter.
 */

#include <taglib/tag_c.h>
#include <stdbool.h>

#include "config.h"
#include "t_parser.h"

#define is_int_tkeyword(t) ((t) == TTRACK || (t) == TYEAR)

bool eval(const char *restrict filename,
        const TagLib_Tag *restrict tag,
        const struct ast *restrict filter);

#endif /* not T_INTERPRETER_H */
