
.PHONY: clean

fdspatch: fdspatch.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) -f fdspatch
