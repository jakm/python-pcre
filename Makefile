all: pcre test

pcre:
	python setup.py build

test: empty
	test/run_tests.sh

clean:
	-rm -r build

empty:

.SILENT:
