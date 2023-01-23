# name of this modules dir
MODULE=$(shell basename $(CURDIR))
VPATH+=../../sub/librote

all: moddir \
	$(BINDIR)/$(MODULE)/disp.pio.h \
	$(BINDIR)/$(MODULE)/disp.o \
	$(BINDIR)/$(MODULE)/pwm.o \
	$(BINDIR)/$(MODULE)/dma.o \
	$(BINDIR)/$(MODULE)/pframe.o

moddir:
	$(Q) mkdir -p $(BINDIR)/$(MODULE)

$(BINDIR)/$(MODULE)/%.o: %.c
	@echo CC $<
	$(Q) $(CC) $(CFLAGS) $(INCLUDES) -I$(BINDIR)/$(MODULE) -c -o $@ $<

include $(RIOTMAKE)/tools/pioasm.inc.mk

$(BINDIR)/$(MODULE)/%.pio.h: %.pio
	@echo PIOASM $<
	$(Q) $(RIOTTOOLS)/pioasm/pioasm -o RIOT $< $@
