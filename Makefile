TARGET     := osk-sdl
VERSION	   := 0.62.1

SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LIBS  := $(shell sdl2-config --libs)

CXX        ?= g++
CXXFLAGS   := -std=c++14 -Wall -g $(CXXFLAGS) $(SDL2_CFLAGS)

LIBS       := -lcryptsetup $(SDL2_LIBS) -lSDL2_ttf

DOCB	   := scdoc

SRC_DIR    := src
BIN_DIR    := bin
OBJ_DIR    := obj
DOC_DIR	   := doc
DESTDIR    :=

SOURCES    := ${wildcard $(SRC_DIR)/*.cpp}
OBJECTS    := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

ifeq ("$(V)", "1")
	Q :=
	E := @true
else
	Q := @
	E := @echo
endif

all: directories $(BIN_DIR)/$(TARGET) $(DOC_DIR)/osk-sdl.1

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(E) CC $<
	$(Q)$(CXX) -c -o $@ $< $(CXXFLAGS) $(CPPFLAGS)

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	$(E) LD $<
	$(Q)$(CXX) -o $@ $^ $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS)

$(DOC_DIR)/osk-sdl.1:
	$(Q)sed -i "s:@@VERSION@@:$(VERSION):" $@.scd
	$(E) SCDOC $<.scd
	$(Q)$(DOCB) < $@.scd > $@
	$(Q)sed -i "s:$(VERSION):@@VERSION@@:" $@.scd

.PHONY: clean

.PHONY: directories

clean:
	-rm -rfv $(OBJ_DIR) $(BIN_DIR)
	-rm -rfv $(DOC_DIR)/osk-sdl.1

directories:
	@mkdir -p ./obj
	@mkdir -p ./bin

check:
	# Note: tests depend on a specific screen size!
	@xvfb-run -s "-ac -screen 0 480x800x24" sh ./test/test_functional.sh

install:
	install -Dm755 bin/osk-sdl 	"$(DESTDIR)/usr/bin/osk-sdl"
	install -Dm644 osk.conf 	"$(DESTDIR)/etc/osk.conf"
	install -Dm644 doc/osk-sdl.1 	"$(DESTDIR)/usr/share/man/man1/osk-sdl.1"
