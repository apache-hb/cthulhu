CTC := K:/github/cthulhu/build/src/frontend/cli/cli.exe
CC := cl.exe

src = main.ct config.ct win32.ct cstdlib.ct

all: compile

run: compile
	main.exe

compile: sources
	$(CC) /nologo out/module.c /Fe:main.exe

sources: $(src)
	$(CTC) $(src) /file-layout:pair
