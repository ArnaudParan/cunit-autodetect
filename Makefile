.DEFAULT_GOAL=.DEFAULT
CC=clang
BINDIR=bin
OBJDIR=obj
SDIR=src
TDIR=tests
CFLAGS=-Wall -iquote ./$(SDIR) -I /usr/include
LIBS=

CFILES=$(shell find "$(SDIR)" -name "*.c")
HFILES=$(shell find "$(SDIR)" -name "*.h")
ROBJECTS=$(foreach path,$(CFILES),$(shell sed "s/$(SDIR)/$(OBJDIR)\/release/g" <<< "$(path:c=o)"))
DOBJECTS=$(foreach path,$(CFILES),$(shell sed "s/$(SDIR)/$(OBJDIR)\/debug/g" <<< "$(path:c=o)"))

TCFILES=$(shell find "$(TDIR)" -name "*.c")
THFILES=$(shell find "$(TDIR)" -name "*.h")
TOBJECTS=$(TCFILES:c=o)
TBIN=$(BINDIR)/test

BINNAME=test-maker
BIN=test-maker

debug: CFLAGS += -DDEBUG -ggdb
debug: $(BINDIR)/debug/$(BIN)

release: $(BINDIR)/release/$(BIN)

$(OBJDIR)/release/%.o: $(LIBS) $(HFILES)
$(OBJDIR)/release/%.o: $(SDIR)/%.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(OBJDIR)/debug/%.o: $(LIBS) $(HFILES)
$(OBJDIR)/debug/%.o: $(SDIR)/%.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(BINDIR)/release/$(BIN): $(ROBJECTS) | $(BINDIR)/release
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/debug/$(BIN): $(DOBJECTS) | $(BINDIR)/debug
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(TDIR)/%.o: $(TDIR)/%.c $(LIBS) $(HFILES) $(THFILES)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(TBIN): LIBS += -lcunit
$(TBIN): CFLAGS += -DEBUG -ggdb -iquote ./ -I /usr/include/CUnit
$(TBIN): $(DOBJECTS) $(TOBJECTS) $(LIBS) | $(BINDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

check: $(TBIN)
	$(TBIN)
$(OBJDIR):
	install -d $(OBJDIR)/release
	install -d $(OBJDIR)/debug
$(BINDIR):
	install -d $@
$(BINDIR)/release:
	install -d $@
$(BINDIR)/debug:
	install -d $@

.SUFFIXES: .o .c .h
.DEFAULT: release

.PHONY: clean
.PHONY: clean-all
.PHONY: release
.PHONY: debug
.PHONY: all
.PHONY: default
.PHONY: check

clean:
	rm -rf $(OBJDIR)
	rm -f $(TOBJECTS)

clean-all: clean
	rm -rf $(BINDIR)
	rm -f $(TBIN)
