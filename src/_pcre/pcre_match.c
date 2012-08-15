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

#include "pcre_module.h";
#include "pcre_match.h"

static void
pcre_MatchObject_dealloc(pcre_MatchObject* self)
{
	free(self->subject);
	free(self->offsetvector);

	Py_XDECREF(self->re);

	self->ob_type->tp_free((PyObject*)self);
}

static int
pcre_MatchObject_init(pcre_MatchObject *self, PyObject *args, PyObject *kwds)
{
	// pcre_MatchObject will be initialized directly from RegexObject.match()
	return 0;
}

static PyObject *
pcre_MatchObject_getpos(pcre_MatchObject *self, void *closure)
{
	return Py_BuildValue("i", self->pos);
}

static PyObject *
pcre_MatchObject_getendpos(pcre_MatchObject *self, void *closure)
{
	return Py_BuildValue("i", self->endpos);
}

static PyObject *
pcre_MatchObject_getlastindex(pcre_MatchObject *self, void *closure)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_getlastgroup(pcre_MatchObject *self, void *closure)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_getre(pcre_MatchObject *self, void *closure)
{
	Py_INCREF(self->re);
	return self->re;
}

static PyObject *
pcre_MatchObject_getstring(pcre_MatchObject *self, void *closure)
{
	return Py_BuildValue("s", self->subject);
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
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_get_substring(pcre_MatchObject* self, int group)
{
	char **substring;
	int rc = pcre_get_substring(self->subject, self->offsetvector, self->stringcount,
								group, substring);
	if (rc < 0) {
		sprintf(message_buffer,
				"Picking of substring exited with an error (group = %d, code = %d).", group, rc);
		PyErr_SetString(PcreError, message_buffer);
		return NULL;
	}

	PyObject *result = Py_BuildValue("s", substring);
	if (result == NULL) {
		PyErr_SetString(PcreError, "An error when building substring object.");
		pcre_free(substring);
		return NULL;
	}

	pcre_free(substring);
	Py_INCREF(result);
	return result;
}

static PyObject *
pcre_MatchObject_group(pcre_MatchObject* self, PyObject *args)
{
	if (PyTuple_Size(args) == (Py_ssize_t) 0)
		PyTuple_SetItem(args, 0, Py_BuildValue("i", 0));

	Py_ssize_t size = PyTuple_GET_SIZE(args);

	PyObject *result;

	// method called without parameters
	if (size == 0) {
		// pcre_MatchObject_get_substring increases counter of returned object
		result = pcre_MatchObject_get_substring(self, 0);
		if (result == NULL)
			return NULL;

		goto RET;
	}

	int *groups = (int *) malloc(size * sizeof(int));
	if (groups == NULL) {
		PyErr_SetString(PcreError, "An error when allocating groups.");
		return NULL;
	}

	for (Py_ssize_t i = 0; i < size; i++) {
		PyObject *group = PyTuple_GET_ITEM(args, i);
		groups[i] = (int) PyInt_AsLong(group);
	}

	if (size == 1) {
		// pcre_MatchObject_get_substring increases counter of returned object
		result = pcre_MatchObject_get_substring(self, groups[0]);
		if (result == NULL)
			goto ERROR_DEALLOC;

		goto RET;
	}

	result = PyTuple_New(size);
	if (result == NULL) {
		PyErr_SetString(PcreError, "An error when allocating tuple object.");
		goto ERROR_DEALLOC;
	}

	Py_ssize_t items_count = 0;
	for (Py_ssize_t i = 0; i < size; i++) {
		PyObject *substring = pcre_MatchObject_get_substring(self, groups[i]);
		if (substring == NULL)
			// pcre_MatchObject_get_substring sets exception
			goto ERROR_DECREF_ITEMS;

		if (PyTuple_SetItem(result, i, substring) != 0) {
			PyErr_SetString(PcreError, "An error when setting tuple item.");
			goto ERROR_DECREF_ITEMS;
		}
		items_count++;
	}

	Py_INCREF(result);

RET:
	free(groups);
	return result;

ERROR_DECREF_ITEMS:
	for (Py_ssize_t i = 0; i < items_count; i++) {
		PyObject *item = PyTuple_GET_ITEM(result, i);
		if (item != NULL)
			Py_XDECREF(item);
	}

ERROR_DEALLOC:
	free(groups);
	return NULL;
}

static PyObject *
pcre_MatchObject_groups(pcre_MatchObject* self)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_groupdict(pcre_MatchObject* self)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;;
}

static PyObject *
pcre_MatchObject_start(pcre_MatchObject* self)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_end(pcre_MatchObject* self)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_span(pcre_MatchObject* self)
{
	// TODO
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyMethodDef pcre_MatchObject_methods[] = {
	{"expand", (PyCFunction)pcre_MatchObject_expand, METH_NOARGS, NULL},
	{"group", (PyCFunction)pcre_MatchObject_group, METH_VARARGS, NULL},
	{"groups", (PyCFunction)pcre_MatchObject_groups, METH_NOARGS, NULL},
	{"groupdict", (PyCFunction)pcre_MatchObject_groupdict, METH_NOARGS, NULL},
	{"start", (PyCFunction)pcre_MatchObject_start, METH_NOARGS, NULL},
	{"end", (PyCFunction)pcre_MatchObject_end, METH_NOARGS, NULL},
	{"span", (PyCFunction)pcre_MatchObject_span, METH_NOARGS, NULL},
	{NULL}  /* Sentinel */
};

PyTypeObject pcre_MatchType = {
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
