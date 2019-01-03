CFLAGS=-Wall -O2 -mrdrnd
PREFIX?=/usr/local

rdrandd: rdrandd.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm rdrandd

install: rdrandd
	install -m755 rdrandd $(DESTDIR)$(PREFIX)/sbin/rdrandd

.PHONY : clean install

