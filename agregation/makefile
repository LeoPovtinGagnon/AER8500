# Compiler et linker OpenCV avec g++ de façon portable
CC = g++
CFLAGS = -std=c++11 `pkg-config --cflags opencv4`
LIBS = `pkg-config --libs opencv4`

# Fichiers source et output
SRC = agregator.cpp  client.cpp A429.cpp UI.cpp AFDX.cpp
OBJ = agregator.o client.o A429.o UI.o AFDX.o
OUT = client

# Règles de compilation
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $(OUT) $(OBJ) $(CFLAGS) $(LIBS)	

$(OBJ): $(SRC)
	$(CC) -c $(SRC) $(CFLAGS)	

clean:
	rm -f $(OBJ) $(OUT)