all: options unicode wishing

.PHONY: options unicode wishing

options:
	cd options && make check

unicode:
	cd unicode && make check

wishing:
	cd wishing && make check

clean:
	rm -f options/test_base32 options/test_options unicode/test_unicode wishing/test_wishing
