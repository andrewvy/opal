export SODIUM_STATIC := yes

CC=cc
SRCDIR := src
BUILDDIR := build
DEPSDIR := deps
BINDIR := bin
TARGET := bin/opal

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CC_LIB := -I/usr/local/include
CFLAGS := -g $(CC_LIB)

LIB := -L/usr/local/lib -lcrypto -Bstatic -lsodium -lleveldb
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(BINDIR)
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

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

.PHONY: clean

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
