CTC := K:/github/cthulhu/build/src/frontend/cli/cli.exe
CC := cl.exe

src = main.ct config.ct win32.ct cstdlib.ct

BUILDDIR := build
MAIN := $(BUILDDIR)/main.exe

all: $(MAIN)

clean:
	rd /Q /S $(BUILDDIR)

run: $(MAIN)
	$(MAIN)

$(MAIN): sources
	$(CC) /nologo $(BUILDDIR)/module.c /TC /Fe:$(MAIN) /DEBUG:FULL /Zi /Fo:$(BUILDDIR)/main.obj /Fd:$(BUILDDIR)/

sources: $(src) $(BUILDDIR)
	$(CTC) $(src) /file-layout:pair /output-dir:$(BUILDDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)
