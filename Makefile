all: pcre test

pcre:
	(cd src/_pcre;$(MAKE);cp _pcre.so ../pcre)

test: empty
	test/run_tests.sh

clean:
	-(cd src/pcre;$(RM) -f *.pyc *.pyo *.so)
	-(cd src/_pcre;$(MAKE) clean)

empty:

.SILENT:
