
.PHONY: clean dosclean dos

fdspatch: fdspatch.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) -f fdspatch

dosclean:
	erase fdspatch.exe

fdspatch.exe: fdspatch.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

dos: fdspatch.exe
