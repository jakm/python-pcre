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
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_getendpos(pcre_MatchObject *self, void *closure)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_getlastindex(pcre_MatchObject *self, void *closure)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_getlastgroup(pcre_MatchObject *self, void *closure)
{
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
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_group(pcre_MatchObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_groups(pcre_MatchObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_groupdict(pcre_MatchObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;;
}

static PyObject *
pcre_MatchObject_start(pcre_MatchObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_end(pcre_MatchObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
}

static PyObject *
pcre_MatchObject_span(pcre_MatchObject* self)
{
	PyErr_SetNone(PyExc_NotImplementedError);
	return NULL;
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
