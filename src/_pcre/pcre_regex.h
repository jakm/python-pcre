/*
 *  Copyright (c) 2012, Jakub Matys <matys.jakub@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

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
