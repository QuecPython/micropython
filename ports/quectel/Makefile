TOP := ../..
ROOT := $(TOP)/../..

include ../../py/mkenv.mk
include mpconfigport.mk
include quectel.mk

CFLAGS = $(INC) $(QUEC_MOD_CFLAGS) $(PLAT_CFLAGS) $(PLAT_DFLAGS) $(COPT)

CSUPEROPT = -Os # save some code space

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1


FROZEN_MANIFEST = $(QUEC_PY_MOD)

ifeq ($(HOST),win32)
export MICROPY_MPYCROSS = tools/notepad++.exe
else
export MICROPY_MPYCROSS = tools/mpy-cross
endif

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

CROSS_COMPILE ?= arm-none-eabi-


INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += $(MICROPYTHON_CFLAGS_INC)
INC += $(QUEC_INC)

# Tune for Debugging or Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Flags for optional C++ source code
CXXFLAGS += $(filter-out -std=c99,$(CFLAGS))
CXXFLAGS += $(CXXFLAGS_MOD)

# Flags for user C modules
CFLAGS += $(CFLAGS_MOD)
LDFLAGS += $(LDFLAGS_MOD)

LIBS =

SHARED_SRC_C = shared/libc/printf.c \
	shared/readline/readline.c \
	shared/runtime/pyexec.c \
	shared/runtime/sys_stdio_mphal.c \
	shared/runtime/stdout_helpers.c \
	shared/netutils/netutils.c \
	shared/runtime/interrupt_char.c \
	shared/timeutils/timeutils.c

EXTMOD_SRC_C = extmod/vfs.c \
	extmod/vfs_lfs.c \
	extmod/vfs_blockdev.c \
	extmod/modrandom.c \
	extmod/modjson.c \
	extmod/modbinascii.c \
	extmod/vfs_reader.c \
	extmod/modre.c \
	extmod/modhashlib.c \
	extmod/moductypes.c

EXTMOD_SRC_C += $(SRC_THIRDPARTY_C)

SRC_MOD += $(QUEC_SRC_MOD)

SRC_C += $(SRC_MOD) \
		 $(SHARED_SRC_C) \
		 $(EXTMOD_SRC_C) \
		 $(QUEC_SRC) \
		 $(BUILD)/frozen_content.c

SRC_CXX += $(SRC_MOD_CXX)

SRC_QSTR += $(SRC_MOD) $(SRC_MOD_CXX) $(SHARED_SRC_C) $(EXTMOD_SRC_C)

OBJ += $(PY_CORE_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))

all: $(BUILD)/firmware.a

$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
	$(ECHO) "MISC freezing bytecode"
	$(Q)$(PYTHON) $(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.a: $(OBJ)
	$(ECHO) "AR $@"
	$(Q)$(AR) -cr $@ $^


# Run emulation build on a POSIX system with suitable terminal settings
#run:
#	stty raw opost -echo
#	@echo Resetting terminal...
# This sleep is useful to spot segfaults
#	sleep 1
#	reset

#test: $(BUILD)/firmware.elf
#	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

include $(TOP)/py/mkrules.mk
