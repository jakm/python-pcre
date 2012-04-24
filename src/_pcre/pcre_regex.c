#include <sys/types.h>
#include <limits.h>

#include "pcre_module.h"
#include "pcre_regex.h"
#include "pcre_match.h"

static void
pcre_RegexObject_dealloc(pcre_RegexObject* self)
{
	free(self->pattern);

	Py_XDECREF(self->groupindex);

	if (self->re != NULL)
		pcre_free(self->re);
	if (self->study != NULL)
		pcre_free_study(self->study);
	if (self->jit_stack != NULL)
		pcre_jit_stack_free(self->jit_stack);

	self->ob_type->tp_free((PyObject*)self);
}

static int
pcre_RegexObject_compile(pcre_RegexObject *self)
{
	char *error;
	int erroffset;

	self->re = pcre_compile(self->pattern, self->flags, &error, &erroffset, NULL);
	if (self->re == NULL) {
	  	sprintf(message_buffer, "Pattern compilation error at offset %d: %s", erroffset, error);
		PyErr_SetString(PcreError, message_buffer);
		return 0;
	}

	if (!self->optimize && self->use_jit) {
		PyErr_SetString(PcreError, "Invalid combination of arguments. To enable JIT you must enable pattern optimization.");
		return 0;
	}

	if (!self->optimize)
		return 1;

	int options = 0;

	if (self->use_jit) {
		if (!jit_enabled) {
			PyErr_SetString(PcreError, "Current version of libpcre is compiled without JIT support.");
			return 0;
		}

		options |= PCRE_STUDY_JIT_COMPILE;
	}

	self->study = pcre_study(self->re, options, &error); // can return NULL when success
	if (error != NULL) {
		sprintf(message_buffer, "Pattern study error: %s", error);
		PyErr_SetString(PcreError, message_buffer);
		return 0;
	}

	if (!self->use_jit)
		return 1;

	// TODO: zkontrolovat, jak se to bude chovat s "neobvyklymi" parametry stacku

	self->jit_stack = pcre_jit_stack_alloc(self->jit_stack_init, self->jit_stack_max);
	if (self->jit_stack == NULL) {
		PyErr_SetString(PcreError, "JIT stack allocation exited with an error.");
		return 0;
	}
	pcre_assign_jit_stack(self->study, NULL, self->jit_stack);

	return 1;
}

static int
pcre_RegexObject_getinfo(pcre_RegexObject *self)
{
	int rc, capturecount, namecount, nameentrysize;
	unsigned char *nametable;

	rc = pcre_fullinfo(self->re, self->study, PCRE_INFO_CAPTURECOUNT, &capturecount);
	if (rc != 0) {
		sprintf(message_buffer, "Detecting of number of capturing subpatterns exited with an error (code = %d).", rc);
		PyErr_SetString(PcreError, message_buffer);
		return 0;
	}

	rc = pcre_fullinfo(self->re, self->study, PCRE_INFO_NAMECOUNT, &namecount);
	if (rc != 0) {
		sprintf(message_buffer,
				"Detecting of named capturing subpatterns exited with an error (PCRE_INFO_NAMECOUNT, code = %d).", rc);
		PyErr_SetString(PcreError, message_buffer);
		return 0;
	}

	if (namecount == 0)
		goto DONE;

	rc = pcre_fullinfo(self->re, self->study, PCRE_INFO_NAMEENTRYSIZE, &nameentrysize);
	if (rc != 0) {
		sprintf(message_buffer,
				"Detecting of named capturing subpatterns exited with an error (PCRE_INFO_NAMEENTRYSIZE, code = %d).", rc);
		PyErr_SetString(PcreError, message_buffer);
		return 0;
	}

	rc = pcre_fullinfo(self->re, self->study, PCRE_INFO_NAMETABLE, &nametable);
	if (rc != 0) {
		sprintf(message_buffer,
				"Detecting of named capturing subpatterns exited with an error (PCRE_INFO_NAMETABLE, code = %d).", rc);
		PyErr_SetString(PcreError, message_buffer);
		goto ERROR;
	}

	/*
	 * nametable has namecount entries and each entry is nameentrysize bytes long
	 * first two bytes is position in pattern, the rest of entry is its name terminated by '\0'
	 * example (?? are unused bytes):
	 * 00 01 d  a  t  e  00 ??
	 * 00 05 d  a  y  00 ?? ??
	 * 00 04 m  o  n  t  h  00
	 * 00 02 y  e  a  r  00 ??
	 */

	unsigned char* entry = nametable;
	for (int i = 0; i < namecount; i++) {

		int pos = (entry[0] << 8) | entry[1];

		PyObject *position = PyInt_FromLong(pos);
		if (position == NULL) {
			PyErr_SetString(PcreError, "An error when allocating int object.");
			goto ERROR;
		}

		if (PyDict_SetItemString(self->groupindex, entry + 2, position) < 0) { // a new copy of string is used
			PyErr_SetString(PcreError, "An error when adding entry to dict object.");
			goto ERROR;
		}

		entry += nameentrysize;
	}

DONE:
	self->groups = capturecount;
	return 1;

ERROR:
	free(nametable);
	return 0;
}

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

	if (!pcre_RegexObject_compile(self))
		return -1;

	if (!pcre_RegexObject_getinfo(self))
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
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_RegexObject_finditer(pcre_RegexObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_RegexObject_match(pcre_RegexObject* self, PyObject *args, PyObject *keywds)
{
	char *subject;
	int pos = INT_MIN, endpos = INT_MIN; // values of non-passed parameters

	static char *kwlist[] = {"string", "pos", "endpos", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|ii", kwlist, subject, &pos, &endpos))
		return NULL;

	// negative positions aren't accepted
	if ((pos < 0 && pos != INT_MIN) || (endpos < 0 && endpos != INT_MIN)) {
		Py_RETURN_NONE;
	}

	int lastindex = strlen(subject) - 1;

	if (pos > lastindex || endpos > lastindex) {
		Py_RETURN_NONE;
	}

	// set defaults
	if (pos == INT_MIN)
		pos = 0;
	if (endpos == INT_MIN)
		endpos = lastindex;


	// length of substring
	int len = endpos - pos + 1;

	// FIXME: jak spravne spocitat velikost vektoru???!!!
	int ovector_size = self->groups * 3;
	int *ovector = (int*)malloc(ovector_size * sizeof(int));
	if (ovector == NULL) {
		PyErr_SetString(PcreError, "An error when allocating the offset vector.");
		return NULL;
	}

	int rc = pcre_exec(self->re, self->study, subject, len, pos, 0, ovector, ovector_size);
	if (rc < 0) {
		PyErr_SetString(PcreError, "Match execution exited with an error."); // TODO: rozliseni chybovych kodu
		return NULL;
	}

	pcre_MatchObject *match = PyObject_New(pcre_MatchObject, &pcre_MatchType);
	if (match == NULL) {
		PyErr_SetString(PcreError, "An error when allocating _pcre.MatchObject.");
		return NULL;
	}
	Py_INCREF(match);

	Py_INCREF(self);
	match->re = self;
	match->offsetvector = ovector;

	match->subject = (char *)malloc((lastindex + 2) * sizeof(char)); // TODO: opravit nazvy promennych pro delku retezce
	strcpy(match->subject, subject);

	return match;
}

static PyObject *
pcre_RegexObject_scanner(pcre_RegexObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_RegexObject_search(pcre_RegexObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_RegexObject_split(pcre_RegexObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_RegexObject_sub(pcre_RegexObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_RegexObject_subn(pcre_RegexObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyMethodDef pcre_RegexObject_methods[] = {
	{"findall", (PyCFunction)pcre_RegexObject_findall, METH_NOARGS,
	"Return a list of all non-overlapping matches of pattern in string."},
	{"finditer", (PyCFunction)pcre_RegexObject_finditer, METH_NOARGS,
	"Return an iterator over all non-overlapping matches for the RE pattern in string. For each match, the iterator returns a match object."},
	{"match", (PyCFunction)pcre_RegexObject_match, METH_VARARGS | METH_KEYWORDS,
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

PyTypeObject pcre_RegexType = {
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
