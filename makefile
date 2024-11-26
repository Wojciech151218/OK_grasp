# Target executable (no extension)
TARGET = ok

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3

# List of source files
SRCS = main.cpp DataPoint.cpp DataLoader.cpp Problem.cpp Graph.cpp Solution.cpp utils.cpp

# List of object files (replace .cpp with .o)
OBJS = $(SRCS:.cpp=.o)

# Executable name based on OS
RM = rm -f

ifeq ($(OS),Windows_NT)
    EXE = $(TARGET).exe
else
    EXE = $(TARGET)
endif

# Default rule
all: $(EXE)

# Link object files to create the executable
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:

	$(RM) $(OBJS) $(EXE)


.PHONY: all clean
