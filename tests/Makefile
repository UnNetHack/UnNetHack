all: options/test_base32 options/test_options unicode/test_unicode wishing/test_wishing

clean:
	rm -f `find */test_* -executable`

%: %.c
	echo `pkg-config --libs check`
	$(CC) -Wall -I../include -I../src -g $(CFLAGS) $@.c `pkg-config --libs check` -o $@ && ./$@
