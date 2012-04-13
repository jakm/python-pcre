all: pcre

CC = gcc

CFLAGS = -g -Wall -std=gnu99

pcre_module.o:
	$(CC) $(CFLAGS) -c -fPIC -I/usr/include/python2.7 -I/usr/local/include pcre_module.c -o pcre_module.o

pcre_regex.o:
	$(CC) $(CFLAGS) -c -fPIC -I/usr/include/python2.7 -I/usr/local/include pcre_regex.c -o pcre_regex.o

pcre_match.o:
	$(CC) $(CFLAGS) -c -fPIC -I/usr/include/python2.7 pcre_match.c -o pcre_match.o

pcre: pcre_module.o pcre_regex.o pcre_match.o
	$(CC) $(CFLAGS) -shared -L/usr/local/lib -lpcre pcre_module.o pcre_regex.o pcre_match.o -o _pcre.so

clean:
	-rm *.o *.so