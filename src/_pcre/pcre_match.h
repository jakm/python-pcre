#ifndef PCRE_MATCH_H
#define PCRE_MATCH_H

#include <Python.h>

typedef struct {
	PyObject_HEAD
	/* public members */
	PyObject *re;
	char *subject;
	/* private members */
	int *offsetvector;

} pcre_MatchObject;

extern PyTypeObject pcre_MatchType;

#endif /* PCRE_MATCH_H */
