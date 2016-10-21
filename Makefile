LDFLAGS = -lm
CFLAGS = -g

jubil: jubil.o reader.o writer.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -rf jubil *.o
