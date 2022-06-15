include src/formats/build.mk

FLUXENGINE_SRCS = \
	src/fe-analysedriveresponse.cc \
	src/fe-analyselayout.cc \
	src/fe-inspect.cc \
	src/fe-rawread.cc \
	src/fe-rawwrite.cc \
	src/fe-read.cc \
	src/fe-rpm.cc \
	src/fe-seek.cc \
	src/fe-testbandwidth.cc \
	src/fe-testvoltages.cc \
	src/fe-write.cc \
	src/fluxengine.cc \
 
FLUXENGINE_OBJS = $(patsubst %.cc, $(OBJDIR)/%.o, $(FLUXENGINE_SRCS))
$(FLUXENGINE_SRCS): | $(PROTO_HDRS)
FLUXENGINE_BIN = $(OBJDIR)/fluxengine.exe
$(FLUXENGINE_BIN): $(FLUXENGINE_OBJS)

$(call use-pkgconfig, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), fmt)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), AGG)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), LIBARCH)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), LIBFLUXENGINE)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), LIBFORMATS)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), LIBUSBP)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), PROTO)
$(call use-library, $(FLUXENGINE_BIN), $(FLUXENGINE_OBJS), STB)

all: fluxengine$(EXT)

fluxengine$(EXT): $(FLUXENGINE_BIN)
	@echo CP $@
	@cp $< $@

