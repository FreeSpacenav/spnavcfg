PREFIX = /usr/local

obj = spnavcfg.o front.o back.o ../spacenavd/cfgfile.o
bin = spnavcfg

warn = -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast

CC = gcc
INSTALL = install
CFLAGS = -pedantic $(warn) -g -I../spacenavd `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0`

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: install
install:
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -m 4775 $(bin) $(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(bin)
