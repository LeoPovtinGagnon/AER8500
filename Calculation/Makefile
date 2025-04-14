# Compiler et linker OpenCV avec g++ de façon portable
CC = g++
CFLAGS = -std=c++11 `pkg-config --cflags opencv4`
LIBS = `pkg-config --libs opencv4`

# Fichiers source et output
SRC = calculator.cpp  server.cpp A429.cpp AFDX.cpp
OBJ = calculator.o server.o A429.o AFDX.o
OUT = server

# Règles de compilation
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $(OUT) $(OBJ) $(CFLAGS) $(LIBS)	

$(OBJ): $(SRC)
	$(CC) -c $(SRC) $(CFLAGS)	

clean:
	rm -f $(OBJ) $(OUT)