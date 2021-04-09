SUBDIRS = Blitter Copper

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

.PHONY: all clean subdirs

all: $(OBJ) subdirs
	@echo > /dev/null
	
subdirs:
	@for dir in $(SUBDIRS); do \
		echo "Entering ${CURDIR}/$$dir"; \
		$(MYMAKE) -C $$dir; \
	done

clean:
	@echo "Cleaning up $(CURDIR)"
	@rm -f *.o
	@for dir in $(SUBDIRS); do \
		$(MYMAKE) -C $$dir clean; \
	done

%.o: %.cpp
	@echo "Compiling $<"
	@$(MYCC) $(MYFLAGS) -c -o $@ $< $(CFLAGS)
