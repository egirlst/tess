CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
INCLUDES = -Iinclude
SRCDIR = src
OBJDIR = build
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/tess
TARGET_TS = $(BINDIR)/ts

# Detect Windows
ifeq ($(OS),Windows_NT)
    LIBS = -lwinhttp
    EXE_EXT = .exe
    # Use standard shell commands if possible, but handle cmd.exe idiosyncrasies
    RM = rmdir /s /q
    # copy command on windows expects backslashes
    CP = copy /Y
    MKDIR = -mkdir
    
    # Fix paths for Windows commands if necessary
    TARGET_WIN = $(subst /,\,$(TARGET))
    TARGET_TS_WIN = $(subst /,\,$(TARGET_TS))
else
    # Linux/Unix defaults
    LIBS = 
    EXE_EXT = 
    RM = rm -rf
    CP = cp
    MKDIR = mkdir -p
    
    # Check for curl
    HAS_CURL := $(shell pkg-config --exists libcurl 2>/dev/null && echo yes)
    ifeq ($(HAS_CURL),yes)
        CFLAGS += -DHAVE_CURL
        LIBS += $(shell pkg-config --libs libcurl)
    else
        # Try curl-config
        HAS_CURL_CONFIG := $(shell curl-config --version 2>/dev/null)
        ifneq ($(HAS_CURL_CONFIG),)
            CFLAGS += -DHAVE_CURL
            LIBS += -lcurl
        endif
    endif
    
    TARGET_WIN = $(TARGET)
    TARGET_TS_WIN = $(TARGET_TS)
endif

# Append extension
TARGET := $(TARGET)$(EXE_EXT)
TARGET_TS := $(TARGET_TS)$(EXE_EXT)

.PHONY: all clean directories

all: directories $(TARGET) $(TARGET_TS)

directories:
	$(MKDIR) $(OBJDIR)
	$(MKDIR) $(BINDIR)

$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

$(TARGET_TS): $(TARGET)
	@echo "Creating ts alias..."
ifeq ($(OS),Windows_NT)
	$(CP) $(subst /,\,$(TARGET)) $(subst /,\,$(TARGET_TS))
else
	$(CP) $(TARGET) $(TARGET_TS)
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	-$(RM) $(OBJDIR)
	-$(RM) $(BINDIR)

install: $(TARGET)
	@echo "Installing tess..."
	@echo "Install not implemented for this OS yet"
