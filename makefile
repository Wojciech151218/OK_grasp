# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11 -O3 -DNDEBUG

# Target executable
TARGET = ok

# List of source files (only .cpp files)
SRCS = main.cpp DataPoint.cpp DataLoader.cpp Problem.cpp Graph.cpp Solution.cpp utils.cpp

# Object files (replace .cpp with .o)
OBJS = $(SRCS:.cpp=.o)
RM = rm -f

# OS-specific adjustments
ifeq ($(OS), Windows_NT)
    EXE = .exe
else
    EXE =
endif

# Default target
all: $(TARGET)$(EXE)

# Link object files to create the executable
$(TARGET)$(EXE): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)$(EXE)

# Compile each .cpp file into an .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	$(RM) $(OBJS) $(TARGET)$(EXE)
