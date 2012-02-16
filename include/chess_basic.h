#include "datatypes.h"
#include "../libh/dllist.h"

#ifndef __CHESS_MAIN_H__
#define __CHESS_MAIN_H__

#define COLORMASK 0x40

#define WHITE 0
#define BLACK 1

#define MAXX 8
#define MAXY 8

#define CAN_STRING_LENGTH 7
#define FEN_STRING_LENGTH 90
#define SAN_STRING_LENGTH 10
#define FILENAME_STRING_LENGTH 100

#define MAKE_CHESSMEN(__color__, __piece__) ((__color__*COLORMASK) | __piece__)

#define CHESSPIECE_COLOR(__x__) (__x__ & COLORMASK ? BLACK : WHITE)
#define CHESSPIECE_COLOR_INV(__x__) (__x__ & COLORMASK ? WHITE : BLACK)
#define CHESSPIECE_TYPE(__x__) (__x__ & (~COLORMASK))

#define GET_OTHER_SIDE(x) ((x+1)%2)
#define GET_OTHER_TURN(x) (x == MT_WHITE ? MT_BLACK : MT_WHITE)

#define PACKPOS(__x__,__y__) (((__y__)&0xFF)<<4 |( (__x__)&0xFF))
#define UNPACKPOS(__packedxy__,__x__,__y__) __y__ = ((__packedxy__) & 0xF0) >> 4; \
                                            __x__ = ((__packedxy__)&0x0F);
                                            
#define IS_VALID_CHESSPIECE(__x__) ((__x__) ==  PAWN ||  (__x__) ==  CASTLE || (__x__) ==  KNIGHT || (__x__) ==  BISHOP || (__x__) ==  QUEEN || (__x__) ==  KING) 

#define DEFAULT_FEN_STRING "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define MAX_REC_DEPTH 16
#define MAX_MOVES_PER_POSITION 100

#define MAX_PLAY_DEPTH 4
#define MAX_QUIES_SEARCH_DEPTH 4

#define MAX_FREE_MOVELIST_ENTRIES 1024

typedef char CHESSMEN;


enum CHESSPIECE_GENERAL {
                              PAWN   = 0x01, 
                              CASTLE = 0x02, 
                              KNIGHT = 0x04, 
                              BISHOP = 0x08, 
                              QUEEN  = 0x10, 
                              KING   = 0x20
                        };
                              
enum CHESSPIECE {
                       W_PAWN   = MAKE_CHESSMEN(WHITE, PAWN), 
                       W_CASTLE = MAKE_CHESSMEN(WHITE, CASTLE), 
                       W_KNIGHT = MAKE_CHESSMEN(WHITE, KNIGHT),
                       W_BISHOP = MAKE_CHESSMEN(WHITE, BISHOP), 
                       W_QUEEN  = MAKE_CHESSMEN(WHITE, QUEEN), 
                       W_KING   = MAKE_CHESSMEN(WHITE, KING),
                       
                       B_PAWN   = MAKE_CHESSMEN(BLACK, PAWN), 
                       B_CASTLE = MAKE_CHESSMEN(BLACK, CASTLE), 
                       B_KNIGHT = MAKE_CHESSMEN(BLACK, KNIGHT), 
                       B_BISHOP = MAKE_CHESSMEN(BLACK, BISHOP), 
                       B_QUEEN  = MAKE_CHESSMEN(BLACK, QUEEN), 
                       B_KING   = MAKE_CHESSMEN(BLACK, KING),
                       INVALID_CHESSPIECE  = 0x00
                 };

enum GAME_PROGRESS {
                        GP_INITIAL        = 0x0000,
                        GP_IN_PROGRESS    = 0x0001,
                        GP_ENDED          = 0x0002,
                        GP_PAUSED         = 0x0004,
                        GP_STOPPED        = 0x0008
                   };
                
enum BOARD_STATUS {
                          BS_NORMAL = 0x00,
                          BS_INVERTED = 0x01
                  };
                  
enum MOVE_STATUS {
		             MOVE_NONE                 = 0x0000,
                     MOVE_SUCCESS              = 0x0001,
                     MOVE_EN_PASSENT_ACTIVE     = 0x0002,  
                     MOVE_CAPTURE_PIECE        = 0x0004,
                     MOVE_CASTLING             = 0x0008,
                     MOVE_EN_PASSENT           = 0x0010,
		             MOVE_CHECK                = 0x0020,
		             MOVE_CHECKMATE            = 0x0040,
                     MOVE_STALEMATE            = 0x0080,
		             MOVE_PAWN_PROMOTED        = 0x0100,
		             MOVE_ILLEGAL              = 0x0200,
                     MOVE_AMBIGUOUS            = 0x0400,
                     MOVE_DRAW_50MOVE_ACTIVE   = 0x0800,
                     MOVE_DRAW_3MOVE_ACTIVE    = 0x1000,
                     MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE = 0x2000
                 };                     

enum MOVE_TURN {
                     MT_WHITE            = WHITE,
                     MT_BLACK            = BLACK
                 };
                      
enum PLAYER_TYPE  {
                     PT_HUMAN            = 0x01,
                     PT_COMPUTER         = 0x02
                  };

enum MOVE_TYPES
{
    MT_VALIDATE = 0x01,
    MT_BLIND   = 0x02
};
 
enum MOVE_VALIDATION_TYPES
{
    MVT_GENERAL                 = 0x00,
    MVT_ALLOW_OUT_OF_TURN_MOVES = 0x01,
    MVT_SKIP_EXPOSED_4_CHECK_CASE = 0x02
};

typedef UINT8 GAME_SIDE;

typedef UINT8 MOVE_TYPE;

enum CHECK_STATUS
{
     CHECK_NONE         = 0x00,
     CHECK              = 0x01,
     CHECKMATE          = 0x02,
     CHECK_KING_EXPOSED = 0x04
};

enum MOVE_CLAIM
{
    CLAIM_NONE             = 0x00,
    CLAIM_CHECK            = 0x01,
    CLAIM_CHECKMATE        = 0x02,
    CLAIM_STALEMATE        = 0x03,
    CLAIM_DRAW_3MOVE_RULE  = 0x04,
    CLAIM_DRAW_50MOVE_RULE = 0x05,
    CLAIM_SURRENDER        = 0x06
};

enum MOVE_OFFER
{
    OFFER_ACCEPTED         = 0x00,
    OFFER_REJECTED         = 0x01
};

typedef UINT8 CHECK_TYPE;
typedef UINT8 VALIDATION_TYPE;

typedef struct
{
    CHESSPIECE board[8][8];
} CHESSBOARD;

typedef struct tag_boardPosition
{
    UINT8 y : 4;
    UINT8 x : 4;
    
    tag_boardPosition(int _y = 0, int _x = 0)
    {
        y = _y;
        x = _x;
    }
    
}BOARD_POSITION;

typedef struct
{
    DLL_NODE header;
    BOARD_POSITION data;
}CHESSMENLIST;

typedef struct tag_moveList
{
    DLL_NODE header;
    BOARD_POSITION init;
    BOARD_POSITION dest;
    STATUS status;
    UINT64 zobristHash; // 3 move repetition
    UINT8 seqNum; // for transposition table entry validity
    char CAN[CAN_STRING_LENGTH];    // Coordinate algebraic notation eg:a1a3
    char SAN[SAN_STRING_LENGTH];    // Standard arithmetic notation eg: Nf3
    char FEN[FEN_STRING_LENGTH];    // Forsyth Edwards notation for board pos

    tag_moveList()
    {
        status = MOVE_NONE;
        CAN[0] = '\0';
        SAN[0] = '\0';
        FEN[0] = '\0';
    }
	
}MOVELIST;

typedef struct tag_genMoveList
{
    DLL_NODE header;
    BOARD_POSITION init;
    BOARD_POSITION dest;
    STATUS status;
    INT32 boardVal;

    tag_genMoveList()
    {
        status = MOVE_NONE;
        boardVal = 0;
    }
	
}GENERATED_MOVELIST;

typedef struct tag_bookMove
{
    BOARD_POSITION init;
    BOARD_POSITION dest;
    CHESSPIECE_GENERAL newPiece;
    
    tag_bookMove()
    {
        newPiece = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(INVALID_CHESSPIECE));
    }
    
    
}BOOK_MOVE;

typedef struct
{
    DLL_NODE header;
    CHESSPIECE piece;

}CAPTUREDLIST;

typedef struct tag_gameState
{
    MOVE_TURN turn;
    GAME_PROGRESS gameProgress;
    BOARD_STATUS boardStatus;
    BOOL wKingMoved;
    BOOL bKingMoved;
    BOOL bRCastleMoved;
    BOOL bLCastleMoved;
    BOOL wRCastleMoved;
    BOOL wLCastleMoved;
    UINT32 draw50HalfMoveCount;
    UINT8 seqNum;

    tag_gameState(MOVE_TURN t = MT_WHITE, GAME_PROGRESS gp = GP_IN_PROGRESS, 
                  BOARD_STATUS bs = BS_NORMAL, BOOL wkm = FALSE, 
                  BOOL wrcm = FALSE, BOOL wlcm = FALSE, BOOL bkm = FALSE, 
                  BOOL brcm = FALSE, BOOL blcm = FALSE)
    {
        turn = t;
        gameProgress = GP_IN_PROGRESS;
	    boardStatus = bs;
	    wKingMoved = wkm;
	    bKingMoved = bkm;
	    bRCastleMoved = brcm;
	    bLCastleMoved = blcm;
	    wRCastleMoved = wrcm;
	    wLCastleMoved = wlcm;
        draw50HalfMoveCount = 0;
        seqNum = 0;
    }
}GAME_STATE;

//hash table for evaluated moves

typedef struct tag_transpositionTable
{
    UINT64 zobristHash;
    UINT8 seqNum;
    UINT8 depth;
    UINT16 frx    : 3;
    UINT16 fry    : 3;
    UINT16 tox    : 3;
    UINT16 toy    : 3;
    UINT16 newCp  : 4;  
    INT32 score;  
}TRANS_TABLE;

#endif
