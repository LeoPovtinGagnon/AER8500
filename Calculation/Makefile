# Compiler et linker OpenCV avec g++
CC = g++
CFLAGS = -I/opt/homebrew/include/opencv4 -L/opt/homebrew/lib -std=c++11
LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc

# Fichiers source et output
SRC = calculator.cpp  server.cpp A429.cpp 
OBJ = calculator.o server.o A429.o 
OUT = mon_programme

# Règles de compilation
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $(OUT) $(OBJ) $(CFLAGS) $(LIBS)	

$(OBJ): $(SRC)
	$(CC) -c $(SRC) $(CFLAGS)	

clean:
	rm -f $(OBJ) $(OUT)