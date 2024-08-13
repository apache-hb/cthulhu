CTC := $(CURDIR)/../../../build/src/frontend/cli/cli

ifeq ($(OS),Windows_NT)
CC := cl.exe
else
CC := cc
endif

src = main.ct config.ct win32.ct cstdlib.ct

BUILDDIR := build
MAIN := $(BUILDDIR)/main

SOURCES = $(BUILDDIR)/module.c $(BUILDDIR)/module.h

all: $(MAIN)

clean:
	rd /Q /S $(BUILDDIR)

run: $(MAIN)
	$(MAIN)

ifeq ($(OS),Windows_NT)
$(MAIN): $(SOURCES)
	$(CC) /nologo $(BUILDDIR)/module.c /TC /Fe:$(MAIN) /DEBUG:FULL /Zi /Fo:$(BUILDDIR)/main.obj /Fd:$(BUILDDIR)/ /fsanitize=address
else
$(MAIN): $(SOURCES)
	$(CC) $(BUILDDIR)/module.c -o$(BUILDDIR)/main -Wno-format-security -Wno-format-contains-nul
endif

$(SOURCES): $(src) $(BUILDDIR)
	$(CTC) $(src) /file-layout:pair /output-dir:$(BUILDDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)
