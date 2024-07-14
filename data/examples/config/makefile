CTC := K:/github/cthulhu/build/src/frontend/cli/cli.exe
CC := cl.exe

src = main.ct config.ct win32.ct cstdlib.ct

all: main.exe

run: main.exe
	main.exe

main.exe: sources
	$(CC) /nologo out/module.c /Fe:main.exe /DEBUG:FULL /Zi

sources: $(src)
	$(CTC) $(src) /file-layout:pair
