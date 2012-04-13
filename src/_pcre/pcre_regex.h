#ifndef PCRE_REGEX_H
#define PCRE_REGEX_H

#include <Python.h>
#include <pcre.h>

typedef struct {
	PyObject_HEAD
	/* public members */
	char *pattern;
	int flags;
	PyObject *groupindex;
	int groups;
	int optimize;
	int use_jit;
	int jit_stack_init;
	int jit_stack_max;
	/* private members */
	pcre *re;
	pcre_extra *study;
	pcre_jit_stack *jit_stack;
} pcre_RegexObject;

extern PyTypeObject pcre_RegexType;

#endif /* PCRE_REGEX_H */
