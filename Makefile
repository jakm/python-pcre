all: pcre

CC = gcc

CFLAGS = -g -Wall

pcre.o:
	$(CC) $(CFLAGS) -c -fPIC -I/usr/include/python2.7 -I/usr/local/include/pcre.h $(@D)/src/pcre/_pcre.c -o $(@D)/src/pcre/_pcre.o

pcre: pcre.o
	$(CC) $(CFLAGS) -shared -L/usr/local/lib -lpcre $(@D)/src/pcre/_pcre.o -o $(@D)/src/pcre/_pcre.so

clean:
	-rm $(@D)/src/pcre/_pcre.o $(@D)/src/pcre/_pcre.so