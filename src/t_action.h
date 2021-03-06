#ifndef T_ACTION_H
#define T_ACTION_H
/*
 * t_action.h
 *
 * tagutil actions.
 */
#include "t_config.h"
#include "t_toolkit.h"
#include "t_tune.h"


/* type of action that can be performed */
enum t_actionkind {
	/* user options */
	T_ACTION_ADD,		/* add:TAG=VALUE	add tag */
	T_ACTION_BACKEND,	/* backend		show backend */
	T_ACTION_CLEAR,		/* clear:TAG		clear tag */
	T_ACTION_EDIT,		/* edit			edit with $EDITOR */
	T_ACTION_LOAD,		/* load:PATH		load file */
	T_ACTION_PRINT,		/* print		display tags */
	T_ACTION_RENAME,	/* rename:PATTERN	rename files */
	T_ACTION_SET,		/* set:TAG=VALUE	set tags */
};

/* action with (or without) argument to proceed */
struct t_action {
	enum t_actionkind kind;
	void	*opaque; /* argument of the action */
	int	write; /* 1 if the action need write access, 0 otherwise */
	int (*apply)(struct t_action *self, struct t_tune *tune);
	TAILQ_ENTRY(t_action)	entries;
};
/* action queue head */
TAILQ_HEAD(t_actionQ, t_action);

/*
 * Create a queue of action based on argc/argv.
 *
 * @param argc_p
 *   A pointer to argc. Cannot be NULL.
 *
 * @param argv_p
 *   A pointer to argv. Cannot be NULL.
 *
 * @return
 *   A new t_actionQ that resulted from argc/argv parsing. On error, NULL is
 *   returned: either errno is set to ENOMEM then malloc(3) failed, or there was
 *   an error in option parsing (argv is malformed). On error argc_p and argv_p
 *   are left unmodified.
 */
struct t_actionQ	*t_actionQ_new(int *argc_p, char ***argv_p);

/*
 * destroy (free memory) of an action queue.
 */
void	t_actionQ_delete(struct t_actionQ *aQ);
#endif /* ndef T_ACTION_H */
