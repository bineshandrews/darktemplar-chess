#include "../include/chess_basic.h"

#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

class CZobrist
{
  private:
    UINT8 cBoard[MAXY][MAXX];
    BOOL castling[4];
    BOOL enpOn;
    UINT8 enpFile;
    MOVE_TURN turn;
    UINT64 zobristKey;

  public:    
    CZobrist();
    CZobrist(CHESSBOARD *, GAME_STATE *, MOVELIST *, MOVE_TURN *);
    CZobrist(char *fen);
    void updateBoardPosition(CHESSBOARD *, GAME_STATE *, 
                             MOVELIST *, MOVE_TURN *);
    void updateBoardPosition(const char *fen);
    UINT64 calculateZobristHash();
    UINT64 getZobristHash();
};

#endif
