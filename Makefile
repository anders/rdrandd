CFLAGS=-Wall -O2 -g -mrdrnd

rdrandd: rdrandd.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm rdrandd

.PHONY : clean

