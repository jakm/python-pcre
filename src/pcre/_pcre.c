/*
 * 	Copyright (c) 2012, Jakub Matys <matys.jakub@gmail.com>
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions are met:
 *		* Redistributions of source code must retain the above copyright
 *		  notice, this list of conditions and the following disclaimer.
 *		* Redistributions in binary form must reproduce the above copyright
 *		  notice, this list of conditions and the following disclaimer in the
 *		  documentation and/or other materials provided with the distribution.
 *		* Neither the name of the <organization> nor the
 *		  names of its contributors may be used to endorse or promote products
 *		  derived from this software without specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <Python.h>
#include <pcre.h>

static int jit_enabled;

/*
 * EXCEPTIONS
 */

static PyObject *PcreError;

/*
 * CLASSES
 */

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
	// TODO: tezko rict jak to bude vypadat dale, ale pokud to budu potrebovat predavat i do match objektu, tak bude lepsi udelat
	// explicitni strukturu
	pcre *re;
	pcre_extra *extra;
	pcre_jit_stack *jit_stack;
} pcre_RegexObject;

static void
pcre_RegexObject_dealloc(pcre_RegexObject* self)
{
	free(self->pattern);

	Py_XDECREF(self->groupindex);

	if (self->re != NULL)
		pcre_free(self->re);
	if (self->extra != NULL)
		pcre_free_study(self->extra);
	if (self->jit_stack != NULL)
		pcre_jit_stack_free(self->jit_stack);

	self->ob_type->tp_free((PyObject*)self);
}

static int
pcre_RegexObject_initpcre(pcre_RegexObject *self)
{
	const char *error;
	int erroffset;

	self->re = pcre_compile(self->pattern, self->flags, &error, &erroffset, NULL);
	if (self->re == NULL) {
	  	// TODO: Check for errors - error, erroffset
		return 0;
	}

	if (!self->optimize && self->use_jit) {
		PyErr_SetString(PcreError, "Invalid combination of arguments. To enable JIT you must enable pattern optimization.");
		return -1;
	}

	if (!self->optimize)
		return 1;

	int options = 0;

	if (self->use_jit) {
		if (!jit_enabled) {
			PyErr_SetString(PcreError, "Current version of libpcre is compiled without JIT support.");
			return -1;
		}

		options |= PCRE_STUDY_JIT_COMPILE;
	}

	self->extra = pcre_study(self->re, options, &error);
	if (error != NULL) {
		// TODO: error
		return 0;
	}

	if (!self->use_jit)
		return 1;

	// TODO: zkontrolovat, jak se to bude chovat s "neobvyklymi" parametry stacku

	self->jit_stack = pcre_jit_stack_alloc(self->jit_stack_init, self->jit_stack_max);
	if (self->jit_stack == NULL) {
		// TODO: Check for error (NULL)
		return 0;
	}
	pcre_assign_jit_stack(self->extra, NULL, self->jit_stack);

	return 1;
}

#define JIT_STACK_INIT_DEFAULT 32*1024
#define JIT_STACK_MAX_DEFAULT 512*1024

static int
pcre_RegexObject_init(pcre_RegexObject *self, PyObject *args, PyObject *kwds)
{
	self->jit_stack_init = JIT_STACK_INIT_DEFAULT;
	self->jit_stack_max = JIT_STACK_MAX_DEFAULT;
	self->groupindex = Py_BuildValue("{}"); // FIXME: zjistit, zda tyto funkce zvysuji pocitadlo objektu!!!
	if (self->groupindex == NULL)
		return -1;

	static char *kwlist[] = {"pattern", "flags", "optimize", "use_jit", "jit_stack_init", "jit_stack_max", NULL};

	char *tmp;
	if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|iiiii", kwlist, &tmp, &self->flags, &self->optimize,
			&self->use_jit, &self->jit_stack_init, &self->jit_stack_max))
		return -1;

	int len = strlen(tmp) + 1;
	self->pattern = (char *)malloc(len * sizeof(char));
	strcpy(self->pattern, tmp);

	if (!pcre_RegexObject_initpcre(self))
		return -1;

	return 0;
}

static PyObject *
pcre_RegexObject_getflags(pcre_RegexObject *self, void *closure)
{
	return Py_BuildValue("i", self->flags);
}

static PyObject *
pcre_RegexObject_getgroupindex(pcre_RegexObject *self, void *closure)
{
	Py_INCREF(self->groupindex);
	return self->groupindex;
}

static PyObject *
pcre_RegexObject_getgroups(pcre_RegexObject *self, void *closure)
{
	return Py_BuildValue("i", self->groups);
}

static PyObject *
pcre_RegexObject_getpattern(pcre_RegexObject *self, void *closure)
{
	return Py_BuildValue("s", self->pattern);
}

static PyObject *
pcre_RegexObject_getoptimized(pcre_RegexObject *self, void *closure)
{
	return Py_BuildValue("i", self->optimize);
}

static PyObject *
pcre_RegexObject_getusejit(pcre_RegexObject *self, void *closure)
{
	return Py_BuildValue("i", self->use_jit);
}

// TODO: doplnit docstringy
static PyGetSetDef pcre_RegexObject_getseters[] = {
	{"flags", (getter)pcre_RegexObject_getflags, NULL, NULL, NULL},
	{"groupindex", (getter)pcre_RegexObject_getgroupindex, NULL, NULL, NULL},
	{"groups", (getter)pcre_RegexObject_getgroups, NULL, NULL, NULL},
	{"pattern", (getter)pcre_RegexObject_getpattern, NULL, NULL, NULL},
	{"optimized", (getter)pcre_RegexObject_getoptimized, NULL, NULL, NULL},
	{"use_jit", (getter)pcre_RegexObject_getusejit, NULL, NULL, NULL},
	{NULL}  /* Sentinel */
};

static PyObject *
pcre_RegexObject_findall(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_finditer(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_match(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_scanner(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_search(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_split(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_sub(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_RegexObject_subn(pcre_RegexObject* self)
{
	Py_RETURN_NONE;
}

static PyMethodDef pcre_RegexObject_methods[] = {
	{"findall", (PyCFunction)pcre_RegexObject_findall, METH_NOARGS,
	"Return a list of all non-overlapping matches of pattern in string."},
	{"finditer", (PyCFunction)pcre_RegexObject_finditer, METH_NOARGS,
	"Return an iterator over all non-overlapping matches for the RE pattern in string. For each match, the iterator returns a match object."},
	{"match", (PyCFunction)pcre_RegexObject_match, METH_NOARGS,
	"Matches zero or more characters at the beginning of the string."},
	{"scanner", (PyCFunction)pcre_RegexObject_scanner, METH_NOARGS, NULL},
	{"search", (PyCFunction)pcre_RegexObject_search, METH_NOARGS,
	"Scan through string looking for a match, and return a corresponding MatchObject instance. Return None if no position in the string matches."},
	{"split", (PyCFunction)pcre_RegexObject_split, METH_NOARGS,
	"Split string by the occurrences of pattern."},
	{"sub", (PyCFunction)pcre_RegexObject_sub, METH_NOARGS,
	"Return the string obtained by replacing the leftmost non-overlapping occurrences of pattern in string by the replacement repl."},
	{"subn", (PyCFunction)pcre_RegexObject_subn, METH_NOARGS,
	"Return the tuple (new_string, number_of_subs_made) found by replacing the leftmost non-overlapping occurrences of pattern with the replacement repl."},
	{NULL}  /* Sentinel */
};

static PyTypeObject pcre_RegexType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"_pcre.RegexObject",       /*tp_name*/
	sizeof(pcre_RegexObject),  /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pcre_RegexObject_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	"Compiled regular expression objects", /* tp_doc */
	0,		                   /* tp_traverse */
	0,		                   /* tp_clear */
	0,		                   /* tp_richcompare */
	0,		                   /* tp_weaklistoffset */
	0,		                   /* tp_iter */
	0,		                   /* tp_iternext */
	pcre_RegexObject_methods,  /* tp_methods */
	0,                         /* tp_members */
	pcre_RegexObject_getseters,/* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)pcre_RegexObject_init, /* tp_init */
	0,                         /* tp_alloc */
	PyType_GenericNew,         /* tp_new */
};

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
	// TODO: tezko rict jak to bude vypadat dale, ale pokud to budu potrebovat predavat i do match objektu, tak bude lepsi udelat
	// explicitni strukturu
	pcre *re;
	pcre_extra *extra;
	pcre_jit_stack *jit_stack;
} pcre_MatchObject;

static void
pcre_MatchObject_dealloc(pcre_MatchObject* self)
{

}

static int
pcre_MatchObject_init(pcre_MatchObject *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

static PyObject *
pcre_MatchObject_getpos(pcre_MatchObject *self, void *closure)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_getendpos(pcre_MatchObject *self, void *closure)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_getlastindex(pcre_MatchObject *self, void *closure)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_getlastgroup(pcre_MatchObject *self, void *closure)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_getre(pcre_MatchObject *self, void *closure)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_getstring(pcre_MatchObject *self, void *closure)
{
	Py_RETURN_NONE;
}

// TODO: doplnit docstringy
static PyGetSetDef pcre_MatchObject_getseters[] = {
	{"pos", (getter)pcre_MatchObject_getpos, NULL, NULL, NULL},
	{"endpos", (getter)pcre_MatchObject_getendpos, NULL, NULL, NULL},
	{"lastindex", (getter)pcre_MatchObject_getlastindex, NULL, NULL, NULL},
	{"lastgroup", (getter)pcre_MatchObject_getlastgroup, NULL, NULL, NULL},
	{"re", (getter)pcre_MatchObject_getre, NULL, NULL, NULL},
	{"string", (getter)pcre_MatchObject_getstring, NULL, NULL, NULL},
	{NULL}  /* Sentinel */
};

static PyObject *
pcre_MatchObject_expand(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_group(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_groups(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_groupdict(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_start(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_end(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyObject *
pcre_MatchObject_span(pcre_MatchObject* self)
{
	Py_RETURN_NONE;
}

static PyMethodDef pcre_MatchObject_methods[] = {
	{"expand", (PyCFunction)pcre_MatchObject_expand, METH_NOARGS, NULL},
	{"group", (PyCFunction)pcre_MatchObject_group, METH_NOARGS, NULL},
	{"groups", (PyCFunction)pcre_MatchObject_groups, METH_NOARGS, NULL},
	{"groupdict", (PyCFunction)pcre_MatchObject_groupdict, METH_NOARGS, NULL},
	{"start", (PyCFunction)pcre_MatchObject_start, METH_NOARGS, NULL},
	{"end", (PyCFunction)pcre_MatchObject_end, METH_NOARGS, NULL},
	{"span", (PyCFunction)pcre_MatchObject_span, METH_NOARGS, NULL},
	{NULL}  /* Sentinel */
};

static PyTypeObject pcre_MatchType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"_pcre.MatchObject",       /*tp_name*/
	sizeof(pcre_MatchObject),  /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pcre_MatchObject_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	"Regular expression match objects", /* tp_doc */
	0,		                   /* tp_traverse */
	0,		                   /* tp_clear */
	0,		                   /* tp_richcompare */
	0,		                   /* tp_weaklistoffset */
	0,		                   /* tp_iter */
	0,		                   /* tp_iternext */
	pcre_MatchObject_methods,  /* tp_methods */
	0,                         /* tp_members */
	pcre_MatchObject_getseters,/* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)pcre_MatchObject_init, /* tp_init */
	0,                         /* tp_alloc */
	PyType_GenericNew,         /* tp_new */
};

/*
 * FUNCTIONS
 */

static PyObject *
pcre_jit_enabled(PyObject *self, PyObject *args)
{
	return PyBool_FromLong(jit_enabled);
}

static PyObject *
pcre_jit_target(PyObject *self, PyObject *args)
{
	const char *jit_target;
	if(pcre_config(PCRE_CONFIG_JITTARGET, &jit_target) < 0)
		jit_target = NULL;
	return Py_BuildValue("s", jit_target);
}

static PyObject *
pcre_lib_version(PyObject *self, PyObject *args)
{
	const char *version = pcre_version();
	return Py_BuildValue("s", version);
}

static PyMethodDef pcre_functions[] = {
	{"jit_enabled",  pcre_jit_enabled, METH_NOARGS, "Return True when JIT compilation is enabled."},
	{"jit_target",  pcre_jit_target, METH_NOARGS, "Return the target architecture of JIT compilation."},
	{"version",  pcre_lib_version, METH_NOARGS, "Return the version of PCRE library."},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

/*
 * MODULE DEFINITION
 */

PyMODINIT_FUNC
init_pcre(void)
{
	PyObject* m;

	if (PyType_Ready(&pcre_RegexType) < 0)
		return;

	if (PyType_Ready(&pcre_MatchType) < 0)
		return;

	m = Py_InitModule("_pcre", pcre_functions);
	if (m == NULL)
		return;

	PcreError = PyErr_NewException("_pcre.PcreError", NULL, NULL);
	Py_INCREF(PcreError);
	PyModule_AddObject(m, "PcreError", PcreError);

	PyModule_AddIntConstant(m, "PCRE_CASELESS", PCRE_CASELESS);
	PyModule_AddIntConstant(m, "PCRE_MULTILINE", PCRE_MULTILINE);
	PyModule_AddIntConstant(m, "PCRE_DOTALL", PCRE_DOTALL);
	PyModule_AddIntConstant(m, "PCRE_EXTENDED", PCRE_EXTENDED);
	PyModule_AddIntConstant(m, "PCRE_ANCHORED", PCRE_ANCHORED);
	PyModule_AddIntConstant(m, "PCRE_DOLLAR_ENDONLY", PCRE_DOLLAR_ENDONLY);
	PyModule_AddIntConstant(m, "PCRE_EXTRA", PCRE_EXTRA);
	PyModule_AddIntConstant(m, "PCRE_NOTBOL", PCRE_NOTBOL);
	PyModule_AddIntConstant(m, "PCRE_NOTEOL", PCRE_NOTEOL);
	PyModule_AddIntConstant(m, "PCRE_UNGREEDY", PCRE_UNGREEDY);
	PyModule_AddIntConstant(m, "PCRE_NOTEMPTY", PCRE_NOTEMPTY);
	PyModule_AddIntConstant(m, "PCRE_UTF8", PCRE_UTF8);
	PyModule_AddIntConstant(m, "PCRE_UTF16", PCRE_UTF16);
	PyModule_AddIntConstant(m, "PCRE_NO_AUTO_CAPTURE", PCRE_NO_AUTO_CAPTURE);
	PyModule_AddIntConstant(m, "PCRE_NO_UTF8_CHECK", PCRE_NO_UTF8_CHECK);
	PyModule_AddIntConstant(m, "PCRE_NO_UTF16_CHECK", PCRE_NO_UTF16_CHECK);
	PyModule_AddIntConstant(m, "PCRE_AUTO_CALLOUT", PCRE_AUTO_CALLOUT);
	PyModule_AddIntConstant(m, "PCRE_PARTIAL_SOFT", PCRE_PARTIAL_SOFT);
	PyModule_AddIntConstant(m, "PCRE_PARTIAL", PCRE_PARTIAL);
	PyModule_AddIntConstant(m, "PCRE_DFA_SHORTEST", PCRE_DFA_SHORTEST);
	PyModule_AddIntConstant(m, "PCRE_DFA_RESTART", PCRE_DFA_RESTART);
	PyModule_AddIntConstant(m, "PCRE_FIRSTLINE", PCRE_FIRSTLINE);
	PyModule_AddIntConstant(m, "PCRE_DUPNAMES", PCRE_DUPNAMES);
	PyModule_AddIntConstant(m, "PCRE_NEWLINE_CR", PCRE_NEWLINE_CR);
	PyModule_AddIntConstant(m, "PCRE_NEWLINE_LF", PCRE_NEWLINE_LF);
	PyModule_AddIntConstant(m, "PCRE_NEWLINE_CRLF", PCRE_NEWLINE_CRLF);
	PyModule_AddIntConstant(m, "PCRE_NEWLINE_ANYCRLF", PCRE_NEWLINE_ANYCRLF);
	PyModule_AddIntConstant(m, "PCRE_BSR_ANYCRLF", PCRE_BSR_ANYCRLF);
	PyModule_AddIntConstant(m, "PCRE_BSR_UNICODE", PCRE_BSR_UNICODE);
	PyModule_AddIntConstant(m, "PCRE_JAVASCRIPT_COMPAT", PCRE_JAVASCRIPT_COMPAT);
	PyModule_AddIntConstant(m, "PCRE_NO_START_OPTIMIZE", PCRE_NO_START_OPTIMIZE);
	PyModule_AddIntConstant(m, "PCRE_NO_START_OPTIMISE", PCRE_NO_START_OPTIMISE);
	PyModule_AddIntConstant(m, "PCRE_PARTIAL_HARD", PCRE_PARTIAL_HARD);
	PyModule_AddIntConstant(m, "PCRE_NOTEMPTY_ATSTART", PCRE_NOTEMPTY_ATSTART);
	PyModule_AddIntConstant(m, "PCRE_UCP", PCRE_UCP);


	const int utf8_enabled;
	if(pcre_config(PCRE_CONFIG_UTF8, &utf8_enabled) < 0) {
		PyErr_SetString(PyExc_RuntimeError, "Error when querying PCRE_CONFIG_UTF8.");
		return;
	}
	if(!utf8_enabled) {
		PyErr_SetString(PyExc_RuntimeError, "Current version of libpcre is compiled without UTF8 support.");
		return;
	}

	// run-time checking of jit support
	if(pcre_config(PCRE_CONFIG_JIT, &jit_enabled) < 0)
		jit_enabled = 0;

	Py_INCREF(&pcre_RegexType);
	PyModule_AddObject(m, "RegexObject", (PyObject *)&pcre_RegexType);

	Py_INCREF(&pcre_MatchType);
	PyModule_AddObject(m, "MatchObject", (PyObject *)&pcre_MatchType);
}
