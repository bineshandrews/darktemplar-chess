#include "../include/chess.h"

BOOL CChessGame :: isCastlingLegal(BOARD_POSITION init, BOARD_POSITION dest)
{
    UINT8 side, i;
    INT8 moveDir, castleBase;
    CHESSMENLIST *node;
    MOVE_TURN currentTurn;

    side = CHESSPIECE_COLOR(getPiece(init));
    moveDir = (dest.x > init.x) ? 1 : -1;
    castleBase = moveDir > 0 ? 7 : 0;

    if(side == WHITE)
    {
        if(gState.wKingMoved == TRUE)
	    return FALSE;

	if(moveDir > 0 && gState.wRCastleMoved == TRUE)
	    return FALSE;

	if(moveDir < 0 && gState.wLCastleMoved == TRUE)
	    return FALSE;
    }
    
    else 
    {
        if(gState.bKingMoved == TRUE)
	    return FALSE;

	if(moveDir > 0 && gState.bRCastleMoved == TRUE)
	    return FALSE;

	if(moveDir < 0 && gState.bLCastleMoved == TRUE)
	    return FALSE;
    }

    for(i = MIN(init.x + moveDir, castleBase - moveDir); i <= MAX(init.x + moveDir, castleBase - moveDir); i++)
    {
        if(getPiece(BOARD_POSITION(init.y, i)) != INVALID_CHESSPIECE)
	    return FALSE;
    }
    
    currentTurn = getCurrentTurn();
    setNextTurn(MOVE_TURN(GET_OTHER_SIDE(side)));    
    node = (CHESSMENLIST *)chessmenList[GET_OTHER_SIDE(side)].getHead();

    while(node)
    {
        for(i = MIN(init.x, dest.x); i <= MAX(init.x, dest.x); i++)
    	{
	        if(validateMove(node->data, BOARD_POSITION(init.y, i), MVT_GENERAL) & MOVE_SUCCESS)
	        {
                setNextTurn(currentTurn);
                return FALSE;
            }
	    }
	
	    node = (CHESSMENLIST *)chessmenList[GET_OTHER_SIDE(side)].getNext((DLL_NODE*)node);
    }

    setNextTurn(currentTurn);
    return TRUE;
}
