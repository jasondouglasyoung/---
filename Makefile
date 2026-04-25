CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic -Wa,-mbig-obj
INCLUDES := -Iinclude -Iexternal/exprtk
OBJDIR   := build

# CLI target
TARGET_CLI := integral
SRCS_CLI   := src/main.cpp

# GUI target (Win32)
TARGET_GUI := integral_gui
SRCS_GUI   := src/gui.cpp
GUI_LIBS   := -lgdi32 -lcomctl32 -mwindows

# Shared sources
SRCS_COMMON := src/integrator.cpp
OBJS_COMMON := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SRCS_COMMON))

.PHONY: all clean run run-gui help

all: $(TARGET_CLI) $(TARGET_GUI)

# CLI executable
$(TARGET_CLI): $(OBJS_COMMON) $(OBJDIR)/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# GUI executable
$(TARGET_GUI): $(OBJS_COMMON) $(OBJDIR)/gui.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GUI_LIBS)

# Generic rule: compile any .cpp -> .o
$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET_CLI).exe $(TARGET_CLI) $(TARGET_GUI).exe $(TARGET_GUI)

run: $(TARGET_CLI)
	./$(TARGET_CLI)

run-gui: $(TARGET_GUI)
	./$(TARGET_GUI)

help:
	@echo "Targets:"
	@echo "  all        - Build both CLI and GUI"
	@echo "  integral   - CLI version"
	@echo "  integral_gui - GUI version (Win32)"
	@echo "  run        - Build and run CLI"
	@echo "  run-gui    - Build and run GUI"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this message"
