bin=prep

# Set the following to '0' to disable log messages:
debug=1

CFLAGS += -Wall -g -DDEBUG=$(debug)
LDFLAGS +=

src=prep.c
obj=$(src:.c=.o)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

prep.o: prep.c debug.h

clean:
	rm -f $(bin) $(obj)


# Tests --

test: $(bin) ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	rm -rf ./tests
	git clone https://github.com/usf-cs326-fa19/P4-Tests.git tests

testclean:
	rm -rf tests
