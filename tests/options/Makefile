srcdir = .

check: test_options test_base32
	./test_options && \
	 ./test_base32

test_options: $(srcdir)/test_options.c
	$(CC) -Wall -I../../include -I../../src -g $(CFLAGS) test_options.c -lcheck -o test_options

test_base32: $(srcdir)/test_base32.c ../../src/base32.c
	$(CC) -Wall -I../../include -I../../src -g $(CFLAGS) test_base32.c -lcheck -o test_base32
