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

#ifndef PCRE_MODULE_H
#define PCRE_MODULE_H

#include <Python.h>

#define JIT_STACK_INIT_DEFAULT 32*1024
#define JIT_STACK_MAX_DEFAULT 512*1024

extern int jit_enabled;
extern char message_buffer[150];

extern PyObject *PcreError;

#endif /* PCRE_MODULE_H */
