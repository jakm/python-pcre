#ifndef PCRE_MODULE_H
#define PCRE_MODULE_H

#include <Python.h>

#define JIT_STACK_INIT_DEFAULT 32*1024
#define JIT_STACK_MAX_DEFAULT 512*1024

extern int jit_enabled;
extern char message_buffer[150];

extern PyObject *PcreError;

#endif /* PCRE_MODULE_H */
