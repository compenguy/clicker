CC = gcc
BINS = clicker
BINSRELEASE = $(BINS:%=%.release)
BINSDEBUG = $(BINS:%=%.debug)
CFLAGS = -g -O2
clicker_PKGS = x11 xtst xext glib-2.0
clicker_OBJS = clicker.o main.o
make_PKG_CFLAGS = $(foreach _pkg,$($(1)_PKGS),$(shell pkg-config --cflags $(_pkg)))
make_PKG_LDFLAGS = $(foreach _pkg,$($(1)_PKGS),$(shell pkg-config --libs $(_pkg)))
main_CFLAGS = $(call make_PKG_CFLAGS,clicker)
clicker_CFLAGS = $(call make_PKG_CFLAGS,clicker)
clicker_LDFLAGS = $(call make_PKG_LDFLAGS,clicker)

%.o: %.c
	@echo $@ '	<--' $^
	@$(CC) -o $@ -c $^ $(CFLAGS) $(CPPFLAGS) $($(notdir $*)_CFLAGS)

%: %.o
	@echo $@ '  	<--' $^
	@$(CC) -o $@ $^ $(LOADLIBES) $(LDLIBS) $(LDFLAGS) $($(notdir $*)_LDFLAGS)

%.release: %
	@echo $@ '  	<--' $^
	@objcopy --strip-unneeded $< $@

%.debug: %
	@echo $@ '  	<--' $^
	@objcopy --only-keep-debug $< $@

all: $(BINS) $(BINSRELEASE) $(BINSDEBUG)

clicker: $(clicker_OBJS)
clicker.release: clicker
clicker.debug: clicker

clean:
	@rm -f $(BINS) $(BINSRELEASE) $(BINSDEBUG) $(foreach _bin,$(BINS),$($(_bin)_OBJS))
