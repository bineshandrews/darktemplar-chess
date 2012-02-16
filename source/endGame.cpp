#include "../include/chess.h"

BOOL CChessGame :: isEndGame()
{
    /* if it is a CHECK and no valid moves ==> CHECKMATE,
     * else if it is not a check and no valid moves ==> STALEMATE
     */

    UINT8 vMCount;
    
    toggleTurn();
	/* update this to make processing faster.. return on first valid move found! */
    vMCount = getValidMoveCount();
    toggleTurn();

    if(vMCount == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CChessGame :: isKingExposed(BOARD_POSITION init, BOARD_POSITION dest,
                                 STATUS status)
{
    BOARD_POSITION kPosPlayingSide, *pbPos, enpPawnSq;
    CHESSMENLIST *node;
    CHESSPIECE cpInit, cpDest, enpCp = INVALID_CHESSPIECE;
    //CHESSPIECE_GENERAL cpType;    
    BOOL isExposed = FALSE;
    
    //Special moves:
    // Castling.. not required... already checks for moving into check
    // Pawn promotion.. not required.. 
    //EnPassent - yes.. remove the extra piece
        
    cpDest = getPiece(dest);
    cpInit = getPiece(init);

    /* Perform minimal actions required to make the move...
     * No need to update chessmen list.. 
     */
    insertPiece(dest, cpInit);
    clearPiece(init);
    
    if(status & MOVE_EN_PASSENT)
    {
        //Clear the enpassent square as well...
        enpPawnSq = BOARD_POSITION(init.y, dest.x);
        enpCp = getPiece(enpPawnSq); // Always a pawn... still.. :-)
        clearPiece(enpPawnSq);
    }
    
    /* Get the friend king's position */
    if(CHESSPIECE_TYPE(cpInit) == KING)
    {
        //King moved.. we have to check whether King is moving into check..
        kPosPlayingSide = dest;
    }
    else
    {
        // Get from chessmen list
        kPosPlayingSide = getKingPos(gState.turn);
    }

    /* Go through the enemy pieces.. and check for a valid move to the friend
     * king's square...
     */
    toggleTurn();
    node = getChessmenListFirst(gState.turn);
   
    while(node)
    {
        pbPos  = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));
        //cpType = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(getPiece(*pbPos)));
       
        /* avoid pieces captured in the recent move.. */
        if(*(UINT8*)&dest == *(UINT8*)pbPos)
        {
            node = getChessmenListNext(gState.turn, node);
            continue;
        }
           
        if(status & MOVE_EN_PASSENT && *(UINT8*)&enpPawnSq == *(UINT8*)pbPos)
        {
            node = getChessmenListNext(gState.turn, node);
            continue;
        }
        
        if(validateMove(*pbPos, kPosPlayingSide, 
               MVT_GENERAL | MVT_SKIP_EXPOSED_4_CHECK_CASE) != MOVE_ILLEGAL)    
        {
            //cout<<"\nYour King is Exposed 2 CHECK with that move!\n";
            isExposed = TRUE;
            break;
        }
        
        node = getChessmenListNext(gState.turn, node);
    }

    /* Restore the adjusted pieces.. */
    insertPiece(init, cpInit);
    insertPiece(dest, cpDest);
    if(status & MOVE_EN_PASSENT)
    {
        insertPiece(enpPawnSq, enpCp);
    }
        
    toggleTurn();
    
    return isExposed;
}


CHECK_STATUS CChessGame :: scan4Check(BOARD_POSITION init, BOARD_POSITION dest, CHECK_TYPE cType)
{
    BOARD_POSITION  kPosNonPlayingSide, *pbPos;
    CHESSMENLIST *node;

    kPosNonPlayingSide = getKingPos(GET_OTHER_SIDE(getCurrentTurn()));

    if(cType & CHECK || cType & CHECKMATE)
    {
        node = getChessmenListFirst(getCurrentTurn());
        
        while(node)
        {
            pbPos  = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));

            if(validateMove(*pbPos, kPosNonPlayingSide, MVT_GENERAL | MVT_SKIP_EXPOSED_4_CHECK_CASE) != MOVE_ILLEGAL)    
	        {
	            if(cType & CHECKMATE)
		        {
		            if(isEndGame())
		            {
		                return CHECKMATE;
                    }
		            else
		            {
		                return CHECK;
                    }
		        }
		        else
		        {
		            return CHECK;
                }
	        }

            node = getChessmenListNext(getCurrentTurn(), node);
        }
    }
    
    return CHECK_NONE;
}
