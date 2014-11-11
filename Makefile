LDFLAGS = -lgc
CFLAGS = -g

jubil: jubil.o builtins.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -rf jubil *.o
