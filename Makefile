export SODIUM_STATIC := yes

CC=cc
SRCDIR := src
BUILDDIR := build
TESTDIR := test
DEPSDIR := deps
BINDIR := bin
TARGET := bin/opal

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TEST_SOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TEST_OBJECTS := $(patsubst $(TESTDIR)/%,$(BUILDDIR)/$(TESTDIR)/%,$(TEST_SOURCES:.$(SRCEXT)=.o))

CC_LIB := -I/usr/local/include
CFLAGS = -g $(CC_LIB)

LIB := -L/usr/local/lib -lcrypto -Bstatic -lsodium -lleveldb
INC := -I include

all: opal

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

libsodium:
	@mkdir -p $(DEPSDIR)
	[ -d $(DEPSDIR)/libsodium ] || git clone https://github.com/jedisct1/libsodium $(DEPSDIR)/libsodium
	set -ex && cd $(DEPSDIR)/libsodium && \
		git fetch && \
		git checkout origin/stable && \
		rm -rf lib && \
		./autogen.sh && \
		./configure --disable-shared && \
		$(MAKE) && \
		$(MAKE) install

test: CFLAGS += -DOPAL_TEST
test: $(OBJECTS) $(TEST_OBJECTS)
	@mkdir -p $(BINDIR)
	@echo " $(CC) $(OBJECTS) $(TEST_OBJECTS) -o $(BINDIR)/opaltest $(LIB)"; $(CC) $(OBJECTS) $(TEST_OBJECTS) -o $(BINDIR)/opaltest $(LIB)
	@echo " Running tests...";
	./bin/opaltest -v

$(BUILDDIR)/$(TESTDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)/$(TESTDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

opal: $(OBJECTS)
	@mkdir -p $(BINDIR)
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

.PHONY: clean

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET) $(BINDIR)/opaltest"; $(RM) -r $(BUILDDIR) $(TARGET) $(BINDIR)/opaltest
