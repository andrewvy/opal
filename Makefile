CC=cc
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := bin/opal

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CC_LIB := -I/usr/local/include -I/usr/local/opt/openssl/include
CFLAGS := -g $(CC_LIB)

LIB := -L/usr/local/lib -L/usr/local/opt/openssl/lib -lcrypto
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(BINDIR)
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

start:
	@echo " Starting..."
	./bin/opal

.PHONY: clean

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
