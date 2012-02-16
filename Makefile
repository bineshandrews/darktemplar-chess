# Project: Chess
# Makefile by Binesh Andrews

#CUSTOM DEFINES
TARGET = bin/linux
LIB_DIR = libs
PLATFORM_DIR = platform
TEXTUAL_DIR = $(PLATFORM_DIR)/textual
SOURCE_DIR = source
LIB_TARGET = $(LIB_DIR)/$(TARGET)
TEXTUAL_TARGET = $(TEXTUAL_DIR)/$(TARGET)
SOURCE_TARGET = $(SOURCE_DIR)/$(TARGET)

DLLIST_SRC = $(LIB_DIR)/dllist.cpp
ZOBRIST_SRC = $(LIB_DIR)/zobrist.cpp
HELPERUTIL_SRC = $(LIB_DIR)/helperUtil.cpp
USERINTF_SRC = $(TEXTUAL_DIR)/userInterface.cpp
CASTLING_SRC = $(SOURCE_DIR)/castling.cpp
CHESS_SRC = $(SOURCE_DIR)/chess.cpp
CHESSUTIL_SRC = $(SOURCE_DIR)/chessUtil.cpp
ENDGAME_SRC = $(SOURCE_DIR)/endGame.cpp
MOVES_SRC = $(SOURCE_DIR)/moves.cpp
EVAL_SRC  = $(SOURCE_DIR)/evaluation.cpp
OPENINGBOOK_SRC = $(SOURCE_DIR)/openingBook.cpp

DLLIST_OBJ = $(LIB_TARGET)/dllist.o
ZOBRIST_OBJ = $(LIB_TARGET)/zobrist.o
HELPERUTIL_OBJ = $(LIB_TARGET)/helperUtil.o
USERINTF_OBJ = $(TEXTUAL_TARGET)/userInterface.o
CASTLING_OBJ = $(SOURCE_TARGET)/castling.o
CHESS_OBJ  = $(SOURCE_TARGET)/chess.o
CHESSUTIL_OBJ = $(SOURCE_TARGET)/chessUtil.o
ENDGAME_OBJ = $(SOURCE_TARGET)/endGame.o
MOVES_OBJ = $(SOURCE_TARGET)/moves.o
EVAL_OBJ = $(SOURCE_TARGET)/evaluation.o
OPENINGBOOK_OBJ = $(SOURCE_TARGET)/openingBook.o

OUTPUT_NAME = DarkTemplar_v01
COMPILE_FLAGS = -ansi -Wall
OPTIMIZE = -O3
#-std=c99 
#-pedantic

CPP  = g++
CC   = gcc
#WINDRES = windres.exe
WINDRES = 
RES  = 
OBJ  = $(DLLIST_OBJ) $(ZOBRIST_OBJ) $(HELPERUTIL_OBJ) $(USERINTF_OBJ) $(CASTLING_OBJ) $(CHESS_OBJ) $(CHESSUTIL_OBJ) $(ENDGAME_OBJ) $(OPENINGBOOK_OBJ) $(MOVES_OBJ) $(EVAL_OBJ) $(RES)
LINKOBJ  = $(DLLIST_OBJ) $(ZOBRIST_OBJ) $(HELPERUTIL_OBJ) $(USERINTF_OBJ) $(CASTLING_OBJ) $(CHESS_OBJ) $(CHESSUTIL_OBJ) $(ENDGAME_OBJ) $(OPENINGBOOK_OBJ) $(MOVES_OBJ) $(EVAL_OBJ) $(RES)
LIBS =
INCS = 
CXXINCS = 
BIN  = $(TARGET)/$(OUTPUT_NAME)
CXXFLAGS = $(COMPILE_FLAGS) $(OPTIMIZE) $(CXXINCS)   
CFLAGS = $(COMPILE_FLAGS) $(OPTIMIZE) $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	$(RM) $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

$(DLLIST_OBJ): $(DLLIST_SRC)
	$(CPP) -c $(DLLIST_SRC) -o $(DLLIST_OBJ) $(CXXFLAGS)

$(ZOBRIST_OBJ): $(ZOBRIST_SRC)
	$(CPP) -c $(ZOBRIST_SRC) -o $(ZOBRIST_OBJ) $(CXXFLAGS)

$(HELPERUTIL_OBJ): $(HELPERUTIL_SRC)
	$(CPP) -c $(HELPERUTIL_SRC) -o $(HELPERUTIL_OBJ) $(CXXFLAGS)
    
$(USERINTF_OBJ): $(USERINTF_SRC)
	$(CPP) -c $(USERINTF_SRC) -o $(USERINTF_OBJ) $(CXXFLAGS)

$(CASTLING_OBJ): $(CASTLING_SRC)
	$(CPP) -c $(CASTLING_SRC) -o $(CASTLING_OBJ) $(CXXFLAGS)

$(CHESS_OBJ): $(CHESS_SRC)
	$(CPP) -c $(CHESS_SRC) -o $(CHESS_OBJ) $(CXXFLAGS)

$(CHESSUTIL_OBJ): $(CHESSUTIL_SRC)
	$(CPP) -c $(CHESSUTIL_SRC) -o $(CHESSUTIL_OBJ) $(CXXFLAGS)

$(ENDGAME_OBJ): $(ENDGAME_SRC)
	$(CPP) -c $(ENDGAME_SRC) -o $(ENDGAME_OBJ) $(CXXFLAGS)

$(OPENINGBOOK_OBJ): $(OPENINGBOOK_SRC)
	$(CPP) -c $(OPENINGBOOK_SRC) -o $(OPENINGBOOK_OBJ) $(CXXFLAGS)
    
$(MOVES_OBJ): $(MOVES_SRC)
	$(CPP) -c $(MOVES_SRC) -o $(MOVES_OBJ) $(CXXFLAGS)

$(EVAL_OBJ): $(EVAL_SRC)
	$(CPP) -c $(EVAL_SRC) -o $(EVAL_OBJ) $(CXXFLAGS)