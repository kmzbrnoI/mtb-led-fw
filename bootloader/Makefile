MCU = atmega328pb
CRC_POS = 0x6f80
F_CPU = 14745600
FORMAT = ihex
TARGET = build/mtb-led-bootloader
BUILDDIR = build
TARGET = $(BUILDDIR)/mtb-led-bootloader
OBJDIR = obj
SRC = $(wildcard src/*.c) $(wildcard lib/*.c)
OPT = s
CSTANDARD = c99
CDEFS = -DF_CPU=$(F_CPU)UL
#DEBUG = dwarf-2

#CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS)
CFLAGS += -O$(OPT)
CFLAGS += -Wall
CFLAGS += -pedantic
CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += -std=$(CSTANDARD)

LDFLAGS = -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += -Wl,-Ttext=0x7000
LDFLAGS += -Wl,-section-start=.fwattr=$(CRC_POS)

#---------------- Programming Options (avrdude) ----------------

AVRDUDE_PROGRAMMER = stk500
AVRDUDE_PORT = /dev/ttyUSB0    # programmer connected to serial device
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex

AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

#============================================================================

# Define programs and commands.
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVEDIR = rm -rf


# Define Messages
# English
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:


OBJ = $(SRC:%.c=$(OBJDIR)/%.o)
LST = $(SRC:%.c=$(OBJDIR)/%.lst)
GENDEPFLAGS = -MMD -MP -MF .dep/$(@F).d

ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)

all: sizebefore build sizeafter

build: $(TARGET).elf $(TARGET).hex $(TARGET).lss $(TARGET).sym

HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) --mcu=$(MCU) --format=avr $(TARGET).elf

sizebefore:
	@if test -f $(TARGET).elf; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); \
	2>/dev/null; echo; fi

sizeafter:
	@if test -f $(TARGET).elf; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); \
	2>/dev/null; echo; fi

program: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

$(TARGET)_with_bootloader.hex: $

$(BUILDDIR)/%.hex: $(BUILDDIR)/%.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@
	dos2unix $@

$(BUILDDIR)/%.lss: $(BUILDDIR)/%.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $< > $@

$(BUILDDIR)/%.sym: $(BUILDDIR)/%.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

$(OBJDIR)/%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@

%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@

clean:
	$(REMOVEDIR) $(BUILDDIR)
	$(REMOVEDIR) $(OBJDIR)
	$(REMOVEDIR) .dep

$(shell mkdir -p $(dir $(OBJ)) 2>/dev/null)
$(shell mkdir -p $(dir $(TARGET)) 2>/dev/null)

-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY : all sizebefore sizeafter build clean program
