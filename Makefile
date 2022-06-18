CC=gcc
CFLAGS=-x c

ps4fancontrol: ps4fancontrol.c
	$(CC) $(CFLAGS) ps4fancontrol.c -o ps4fancontrol

clean:
	rm -f ps4fancontrol
