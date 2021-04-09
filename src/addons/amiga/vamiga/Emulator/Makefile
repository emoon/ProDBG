SUBDIRS = \
Agnus Base CIA CPU Denise Drive FileSystems Files LogicBoard Memory Paula \
Peripherals RetroShell xdms

MYCC = g++ -std=c++17 -Wfatal-errors

MYFLAGS = \
-Wall \
-I $(CURDIR)/.. \
-I $(CURDIR)/../Utilities \
-I $(CURDIR) \
-I $(CURDIR)/Agnus \
-I $(CURDIR)/Agnus/Blitter \
-I $(CURDIR)/Agnus/Copper \
-I $(CURDIR)/CIA \
-I $(CURDIR)/CPU \
-I $(CURDIR)/CPU/Moira \
-I $(CURDIR)/Denise \
-I $(CURDIR)/Drive \
-I $(CURDIR)/Files \
-I $(CURDIR)/Files/DiskFiles \
-I $(CURDIR)/Files/RomFiles \
-I $(CURDIR)/FileSystems \
-I $(CURDIR)/Base \
-I $(CURDIR)/LogicBoard \
-I $(CURDIR)/Memory \
-I $(CURDIR)/Paula \
-I $(CURDIR)/Paula/Audio \
-I $(CURDIR)/Paula/DiskController \
-I $(CURDIR)/Paula/UART \
-I $(CURDIR)/Peripherals \
-I $(CURDIR)/RetroShell \
-I $(CURDIR)/xdms

export MYCC MYFLAGS

DEPS =
OBJ = Amiga.o

.PHONY: all prebuild subdirs clean bin

all: prebuild $(OBJ) subdirs
	@echo > /dev/null
	
prebuild:
	@echo "Entering ${CURDIR}"
		
subdirs:
	@for dir in $(SUBDIRS); do \
		echo "Entering ${CURDIR}/$$dir"; \
		$(MAKE) -C $$dir; \
	done

clean:
	@echo "Cleaning up $(CURDIR)"
	@rm -f vAmiga *.o
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

bin:
	@echo "Linking vAmiga"
	@g++ -pthread -o vAmiga *.o */*.o */*/*.o

%.o: %.cpp $(DEPS)
	@echo "Compiling $<"
	@$(MYCC) $(MYFLAGS) -c -o $@ $<
