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

#include "pcre_module.h"

int jit_enabled;
char message_buffer[150];

/*
 * EXCEPTIONS
 */

PyObject *PcreError;

/*
 * CLASSES
 */

#include "pcre_regex.h"
#include "pcre_match.h"

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
	if (pcre_config(PCRE_CONFIG_JITTARGET, &jit_target) < 0)
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

	// libpcre constants
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

	// _pcre constants
	PyModule_AddIntConstant(m, "JIT_STACK_INIT_SIZE", JIT_STACK_INIT_DEFAULT);
	PyModule_AddIntConstant(m, "JIT_STACK_MAX_SIZE", JIT_STACK_MAX_DEFAULT);

	// run-time checking of utf8 support
	const int utf8_enabled;
	if (pcre_config(PCRE_CONFIG_UTF8, &utf8_enabled) < 0) {
		PyErr_SetString(PcreError, "Error when querying PCRE_CONFIG_UTF8.");
		return;
	}
	if (!utf8_enabled) {
		PyErr_SetString(PcreError, "Current version of libpcre is compiled without UTF8 support.");
		return;
	}

	// run-time checking of jit support
	if (pcre_config(PCRE_CONFIG_JIT, &jit_enabled) < 0)
		jit_enabled = 0;

	Py_INCREF(&pcre_RegexType);
	PyModule_AddObject(m, "RegexObject", (PyObject *)&pcre_RegexType);

	Py_INCREF(&pcre_MatchType);
	PyModule_AddObject(m, "MatchObject", (PyObject *)&pcre_MatchType);
}
