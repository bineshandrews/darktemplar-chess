#include "chess_basic.h"

#ifndef __CHESS_H__
#define __CHESS_H__

class CChessGame
{
  private:
      CHESSBOARD game;
      PLAYER_TYPE playerType[2];
      GAME_STATE gState;
      CDLList chessmenList[2];
      CDLList moveList;
      CDLList capturedPiecesList[2];
      char initFEN[FEN_STRING_LENGTH];
      char openingBookFile[FILENAME_STRING_LENGTH];
      //CDLList undoMoveList[2];
      
      //GeneratedMoves
      GENERATED_MOVELIST **generatedMoves;
      
      //FreeList
      CDLList freeMoveList;
      CDLList freeCapturedPiecesList;
      CDLList freeChessmenList;

  public:
      
      CChessGame();
      CChessGame(PLAYER_TYPE, PLAYER_TYPE);
      CChessGame(const char *, PLAYER_TYPE, PLAYER_TYPE);
      CChessGame(CChessGame *);
      ~CChessGame();

      void initGame(const char *, BOOL, PLAYER_TYPE, PLAYER_TYPE);
      void initGeneratedMoves();
      void initFreeLists();
      void deInitFreeLists();
      void deInitGeneratedMoves();
      void cleanupLists();
      void displayBoard();
      void printMoveList();
      
      void setOpeningBook(char *);
      BOOL getBestMoveFromBook(UINT64, BOOK_MOVE *);

      BOARD_POSITION convertPosition2Absolute(BOARD_POSITION); // make this friend
      BOARD_POSITION invertPosition(BOARD_POSITION); // make this friend

      inline void insertPiece(BOARD_POSITION, CHESSPIECE);
      inline void clearPiece(BOARD_POSITION);
      inline CHESSPIECE getPiece(BOARD_POSITION);
      
      void setInitFEN(const char *fenString);
      char *getInitFEN();

      STATUS makeMove(BOARD_POSITION, BOARD_POSITION, CHESSPIECE_GENERAL, STATUS);
      STATUS makeGeneratedMove(GENERATED_MOVELIST move);
      STATUS makeGeneratedMove(MOVELIST move);
      STATUS playMoveOnBoard(BOARD_POSITION, BOARD_POSITION, CHESSPIECE_GENERAL, MOVE_CLAIM);
      STATUS unmakeMove(void);
      STATUS validateMove(BOARD_POSITION, BOARD_POSITION, VALIDATION_TYPE);
      BOOL insufficientMatingMaterial(char *fenString);
      MOVE_OFFER handleOfferDraw();

      BOOL isEndGame();
      CHECK_STATUS scan4Check(BOARD_POSITION, BOARD_POSITION, CHECK_TYPE);
      BOOL isKingExposed(BOARD_POSITION, BOARD_POSITION, STATUS);
      BOOL isCastlingLegal(BOARD_POSITION, BOARD_POSITION);
	  
      void getValidMoveList(BOARD_POSITION, GENERATED_MOVELIST *, UINT32 *);
      void getValidMoveList(GENERATED_MOVELIST *, UINT32 *);
      
      UINT8 getValidMoveCount(BOARD_POSITION);
      UINT8 getValidMoveCount();
      
      STATUS getRandomMove(MOVELIST *);
      STATUS getBestMove(MOVELIST *);
      STATUS recFindBestMove(UINT32, UINT32, GENERATED_MOVELIST *, INT32, INT32);      

      CHESSPIECE strToPiece(char);
      char* toString(CHESSPIECE);
      void updateFENString(char *);
      char* getLastFENString();
      void updateCANString(MOVELIST *); //Coordinate arithmetic notation
      void updateSANString(MOVELIST *, BOOL);
      STATUS decodeMoveString(char *, MOVELIST *); //Get init, dest, status and newPiece
      STATUS decodeSANString(char *, MOVELIST *); //Get init, dest, status and newPiece
      BOARD_STATUS getBoardStatus();
      void invertBoard();
      inline void toggleTurn();
      void generateChessmenList();
      void updateChessmenList();
      void duplicateMoveList(CDLList);
      
      inline MOVELIST *newMoveListEntry();
      inline CAPTUREDLIST *newCapturedListEntry();
      inline CHESSMENLIST *newChessmenListEntry();
      inline void deleteMoveListEntry(MOVELIST *);
      inline void deleteCapturedListEntry(CAPTUREDLIST *);
      inline void deleteChessmenListEntry(CHESSMENLIST *);
      BOARD_POSITION getKingPos(GAME_SIDE);

      inline CHESSMENLIST *getChessmenListFirst(GAME_SIDE);
      inline CHESSMENLIST *getChessmenListNext(GAME_SIDE, CHESSMENLIST *);

      MOVE_TURN getCurrentTurn();
      MOVE_TURN getPreviousTurn();
      void setNextTurn(MOVE_TURN);
      PLAYER_TYPE getCurrentPlayerType();
      PLAYER_TYPE getPreviousPlayerType();
      PLAYER_TYPE getPlayerType(MOVE_TURN);
      void updatePlayerTypes(PLAYER_TYPE, PLAYER_TYPE);
      GAME_PROGRESS getGameProgress();
      void updateGameProgress(GAME_PROGRESS);

      void initPieceValues(void);      
      INT32 evaluateStaticBoardPosition(UINT8 side);
      INT32 evaluatePawnStructure(UINT8 side);
};

//inline functions
inline void CChessGame :: insertPiece(BOARD_POSITION bp, CHESSPIECE piece)
{
    game.board[bp.y][bp.x] = piece;

    return;
}

inline void CChessGame :: clearPiece(BOARD_POSITION bp)
{
    game.board[bp.y][bp.x] = INVALID_CHESSPIECE;

    return;
}

inline CHESSPIECE CChessGame :: getPiece(BOARD_POSITION bp)
{
    return game.board[bp.y][bp.x];
}

inline void CChessGame:: toggleTurn()
{
    gState.turn = gState.turn == MT_WHITE ? MT_BLACK : MT_WHITE;
}


inline CHESSMENLIST * CChessGame :: getChessmenListFirst(GAME_SIDE side)
{
    return (CHESSMENLIST*)chessmenList[side].getHead();
}

inline CHESSMENLIST * CChessGame :: getChessmenListNext(GAME_SIDE side, CHESSMENLIST *node)
{
    if(NULL == node)
        return (CHESSMENLIST *)NULL;

    return (CHESSMENLIST *)chessmenList[side].getNext((DLL_NODE *)node);
}

inline MOVELIST * CChessGame :: newMoveListEntry()
{
    return (MOVELIST *)freeMoveList.removeHead();
}

inline CAPTUREDLIST * CChessGame :: newCapturedListEntry()
{
    return (CAPTUREDLIST *)freeCapturedPiecesList.removeHead();
}

inline CHESSMENLIST * CChessGame :: newChessmenListEntry()
{
    return (CHESSMENLIST *)freeChessmenList.removeHead();
}

inline void CChessGame :: deleteMoveListEntry(MOVELIST *entry)
{
     freeMoveList.insertAtTail((DLL_NODE *)entry);
}

inline void CChessGame :: deleteCapturedListEntry(CAPTUREDLIST *entry)
{
     freeCapturedPiecesList.insertAtTail((DLL_NODE *)entry);     
}

inline void CChessGame :: deleteChessmenListEntry(CHESSMENLIST *entry)
{
     freeChessmenList.insertAtTail((DLL_NODE *)entry);
}

#endif
