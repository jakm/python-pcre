C_SRC:=src/_pcre
PY_SRC:=src/pcre

all: pcre test

pcre:
	$(MAKE) -C $(C_SRC)
	-cp $(C_SRC)/_pcre.so $(PY_SRC)

test: empty
	test/run_tests.sh

clean:
	-$(MAKE) -C $(PY_SRC) clean
	-$(MAKE) -C $(C_SRC) clean

empty:

.SILENT:
