# -*- encoding: utf8 -*-

#  Copyright (c) 2012, Jakub Matys <matys.jakub@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the
#  Free Software Foundation; either version 2 of the License,
#  or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
#  for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA



import sys
import sre_compile

import re
import _pcre

# public symbols
__all__ = [ "match", "search", "sub", "subn", "split", "findall",
    "compile", "purge", "template", "escape", "I", "L", "M", "S", "X",
    "U", "IGNORECASE", "LOCALE", "MULTILINE", "DOTALL", "VERBOSE",
    "UNICODE", "error", "finditer" ]

__version__ = "0.1"

# TODO: fix LOCALE,UNICODE,VERBOSE
# flags
I = IGNORECASE = _pcre.PCRE_CASELESS # ignore case
L = LOCALE = sre_compile.SRE_FLAG_LOCALE # assume current 8-bit locale
U = UNICODE = sre_compile.SRE_FLAG_UNICODE # assume unicode locale
M = MULTILINE = _pcre.PCRE_MULTILINE # make anchors look for newline
S = DOTALL = _pcre.PCRE_DOTALL # make dot match newline
X = VERBOSE = sre_compile.SRE_FLAG_VERBOSE # ignore whitespace and comments

# pcre exception
error = _pcre.PcreError

# --------------------------------------------------------------------
# public interface

def match(pattern, string, flags=0):
    """Try to apply the pattern at the start of the string, returning
    a match object, or None if no match was found."""
    return _compile(pattern, flags).match(string)

def search(pattern, string, flags=0):
    """Scan through string looking for a match to the pattern, returning
    a match object, or None if no match was found."""
    return _compile(pattern, flags).search(string)

def sub(pattern, repl, string, count=0, flags=0):
    """Return the string obtained by replacing the leftmost
    non-overlapping occurrences of the pattern in string by the
    replacement repl.  repl can be either a string or a callable;
    if a string, backslash escapes in it are processed.  If it is
    a callable, it's passed the match object and must return
    a replacement string to be used."""
    return _compile(pattern, flags).sub(repl, string, count)

def subn(pattern, repl, string, count=0, flags=0):
    """Return a 2-tuple containing (new_string, number).
    new_string is the string obtained by replacing the leftmost
    non-overlapping occurrences of the pattern in the source
    string by the replacement repl.  number is the number of
    substitutions that were made. repl can be either a string or a
    callable; if a string, backslash escapes in it are processed.
    If it is a callable, it's passed the match object and must
    return a replacement string to be used."""
    return _compile(pattern, flags).subn(repl, string, count)

def split(pattern, string, maxsplit=0, flags=0):
    """Split the source string by the occurrences of the pattern,
    returning a list containing the resulting substrings."""
    return _compile(pattern, flags).split(string, maxsplit)

def findall(pattern, string, flags=0):
    """Return a list of all non-overlapping matches in the string.

    If one or more groups are present in the pattern, return a
    list of groups; this will be a list of tuples if the pattern
    has more than one group.

    Empty matches are included in the result."""
    return _compile(pattern, flags).findall(string)

def finditer(pattern, string, flags=0):
    """Return an iterator over all non-overlapping matches in the
    string.  For each match, the iterator returns a match object.

    Empty matches are included in the result."""
    return _compile(pattern, flags).finditer(string)

def compile(pattern, flags=0):
    "Compile a regular expression pattern, returning a pattern object."
    return _compile(pattern, flags)

def purge():
    "Clear the regular expression cache"
    _cache.clear()
    _cache_repl.clear()

_alphanum = {}
for c in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890':
    _alphanum[c] = 1
del c

def escape(pattern):
    "Escape all non-alphanumeric characters in pattern."
    s = list(pattern)
    alphanum = _alphanum
    for i, c in enumerate(pattern):
        if c not in alphanum:
            if c == "\000":
                s[i] = "\\000"
            else:
                s[i] = "\\" + c
    return pattern[:0].join(s)

# --------------------------------------------------------------------
# internals

_cache = {}

_MAXCACHE = 100

def _compile(*key):
    # internal: compile pattern
    cachekey = (type(key[0]),) + key
    p = _cache.get(cachekey)
    if p is not None:
        return p
    pattern, flags = key
    if isinstance(pattern, _pcre.RegexObject):
        if flags:
            raise ValueError('Cannot process flags argument with a compiled pattern')
        return pattern
    if not sre_compile.isstring(pattern):
        raise TypeError('First argument must be string or compiled pattern')
    try:
        p = _pcre.RegexObject(pattern, flags)
    except error, v:
        raise error, v # invalid expression
    if len(_cache) >= _MAXCACHE:
        _cache.clear()
    _cache[cachekey] = p
    return p