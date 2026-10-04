.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO, for example /opt/devkitpro")
endif
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM, for example $(DEVKITPRO)/devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

SOURCES := source source/camera source/ui source/settings source/ride source/effects source/performance source/memory source/runtime source/compat
INCLUDES := include include/compat source source/gameplay
DIAGNOSTIC ?= 0
THREEGXTOOL_FLAGS := -s
ifeq ($(DIAGNOSTIC),1)
TARGET := Gen7FieldFollowerDiagnostic
BUILD := build-diagnostic
PLGINFO := Gen7FieldFollowerDiagnostic.plgInfo
else
TARGET := Gen7FieldFollower
BUILD := build
PLGINFO := Gen7FieldFollower.plgInfo
THREEGXTOOL_FLAGS += --discard-symbols
endif

CTRPFLIB ?= $(DEVKITPRO)/libctrpf
THREEGXTOOL ?= $(DEVKITPRO)/tools/bin/3gxtool

ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := $(ARCH) -O2 -mword-relocations -fomit-frame-pointer \
  -ffunction-sections -fdata-sections -fno-strict-aliasing \
  -Wall -Wextra -D__3DS__
CFLAGS += $(INCLUDE)
CFLAGS += -DFOLLOWER_3GX_DIAGNOSTIC=$(DIAGNOSTIC)
CFLAGS += -DFOLLOWER_3GX_INTERACTION_FEATURES=1
CFLAGS += -DFOLLOWER_3GX_PERFORMANCE_FEATURES=1
CFLAGS += -DFOLLOWER_CARRIER_DEDICATED_ARENA=1
CXXFLAGS := $(CFLAGS) -std=gnu++11 -fno-rtti -fno-exceptions \
  -fno-threadsafe-statics
ASFLAGS := $(ARCH) $(INCLUDE)
LDFLAGS := -T $(TOPDIR)/3gx.ld $(ARCH) -O2 \
  -Wl,--gc-sections,--strip-discarded,--strip-debug \
  -Wl,-Map,$(TOPDIR)/$(TARGET).map
LIBS := -lctrpf -lctru -lm
LIBDIRS := $(CTRPFLIB) $(CTRULIB) $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES_UPPER := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

export LD := $(CXX)
export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
  $(SFILES:.s=.o) $(SFILES_UPPER:.S=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
  -I$(CURDIR)/generated \
  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
  -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean re package $(BUILD)

all: $(BUILD)

$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

package: all
	./package.sh

clean:
	@rm -rf $(BUILD) $(TARGET).3gx $(TARGET).elf $(TARGET).map $(TARGET).lst

re: clean all

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).3gx: $(OFILES)
$(OUTPUT).elf: $(OFILES)

.PRECIOUS: %.elf
%.3gx: %.elf
	@echo creating $(notdir $@)
	@$(THREEGXTOOL) $(THREEGXTOOL_FLAGS) $< $(TOPDIR)/$(PLGINFO) $@

-include $(DEPENDS)

endif
