#include "../include/chess.h"
#include "../libh/zobrist.h"

extern CHESSPIECE_GENERAL getNewPieceFromUser();
extern STATUS compareChessmen(void*, void*);

extern const BOARD_POSITION wKingPos = BOARD_POSITION(0, 4);
extern const BOARD_POSITION bKingPos = BOARD_POSITION(7, 4);
const BOARD_POSITION wLCastlePos = BOARD_POSITION(0, 0);
const BOARD_POSITION wRCastlePos = BOARD_POSITION(0, 7);
const BOARD_POSITION bLCastlePos = BOARD_POSITION(7, 0);
const BOARD_POSITION bRCastlePos = BOARD_POSITION(7, 7);

CChessGame :: CChessGame()
{
    chessmenList[WHITE].setCompare(compareChessmen);
    chessmenList[BLACK].setCompare(compareChessmen);
    initFEN[0] = '\0';

    initFreeLists();
    setInitFEN((char *)DEFAULT_FEN_STRING);
    initGame(getInitFEN(), TRUE, PT_HUMAN, PT_HUMAN);
    initPieceValues();
    initGeneratedMoves();

    return;
}

CChessGame :: CChessGame(PLAYER_TYPE white, PLAYER_TYPE black)
{
    chessmenList[WHITE].setCompare(compareChessmen);
    chessmenList[BLACK].setCompare(compareChessmen);
    initFEN[0] = '\0';

    initFreeLists();
    setInitFEN((char *)DEFAULT_FEN_STRING);
    initGame(getInitFEN(), TRUE, white, black);
    initPieceValues();
    initGeneratedMoves();

    return;
}


CChessGame :: CChessGame(const char *fenStr, PLAYER_TYPE white, PLAYER_TYPE black)
{
    chessmenList[WHITE].setCompare(compareChessmen);
    chessmenList[BLACK].setCompare(compareChessmen);
    initFEN[0] = '\0';
    
    initFreeLists();
    setInitFEN(fenStr);
    initGame(getInitFEN(), TRUE, white, black);
    initPieceValues();
    initGeneratedMoves();
        
    return;
}

CChessGame :: CChessGame(CChessGame *cGame)
                                                         
{
    /* copy evthg except the movelist n chessmen lists */

    chessmenList[WHITE].setCompare(compareChessmen);
    chessmenList[BLACK].setCompare(compareChessmen);

    UINT8 i, j;

    for(i = 0; i < MAXY; i++)
    {
        for(j = 0; j < MAXX; j++)
        {
            this->game.board[i][j] = cGame->game.board[i][j];
        }
    }

    this->gState = cGame->gState;

    return;
}

CChessGame :: ~CChessGame()
{
    cleanupLists();
    deInitGeneratedMoves();
    deInitFreeLists();
    
    return;
}

STATUS CChessGame :: playMoveOnBoard(BOARD_POSITION init, BOARD_POSITION dest, 
                                     CHESSPIECE_GENERAL newCp,
                                     MOVE_CLAIM mClaim = CLAIM_NONE)
{
    STATUS status;
    CHECK_STATUS cStatus;
    MOVELIST *mList, *tempList;
    CZobrist zob;
    UINT64 currentHash;
    UINT8 count = 1, currentSeqNum;
    
    status = validateMove(init, dest, MVT_GENERAL);

    if((status & MOVE_PAWN_PROMOTED) && 
        newCp == CHESSPIECE_TYPE(INVALID_CHESSPIECE))
    {
        //pawn promotion should provide new piece also
        if(playerType[gState.turn] != PT_COMPUTER)
        {
            newCp = getNewPieceFromUser();
        }
        else
        {
            status = MOVE_ILLEGAL;
        }
    }
    
    if(status & MOVE_SUCCESS)
    {
        (void)makeMove(init, dest, newCp, status);
        
        mList = (MOVELIST *)moveList.getTail();
        
        if(mList != NULL)
        {
            //status must have got updated after makemove... 
            status = mList->status;
    	    cStatus = scan4Check(init, dest, CHECK);

    	    if(isEndGame())
    	    {
                if(cStatus == CHECK)
                {
                    status |= MOVE_CHECKMATE;
    	           //printf("Checkmate\n");
                }
                else
                {
                    status |= MOVE_STALEMATE;
		            //printf("Stalemate\n");
                }
            }
            else if(cStatus == CHECK)
            {
                    status |= MOVE_CHECK;
    	            //printf("Itz a check!!\n");
            }

            /* Form the SAN notation for the MOVE */
            mList->status = status; //This is needed for SAN
            (void)updateCANString(mList);
            (void)updateSANString(mList, TRUE);
            (void)updateFENString(mList->FEN);

            //check if 50 move repetition happened
            if(gState.draw50HalfMoveCount >= 50)
            {
                mList->status |= MOVE_DRAW_50MOVE_ACTIVE;
            }
            
            //check for insufficient material draw
            tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)mList);
            
            if((tempList != NULL && tempList->status & MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE) ||
               insufficientMatingMaterial(getLastFENString()) == TRUE)
            {
                mList->status |= MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE;
            }

            // Update zobrist Hash
            zob.updateBoardPosition(getLastFENString());
            zob.calculateZobristHash();
            mList->zobristHash = zob.getZobristHash();
            currentHash = mList->zobristHash;
            currentSeqNum = gState.seqNum;
            
            //check if 3 move repetition happened
            for(tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)mList);
                tempList != NULL;
                tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)tempList))
            {
                if(tempList->seqNum != currentSeqNum)
                {
                    break;
                }
                
                if(tempList->zobristHash == currentHash)
                {
                    //add more detailed check using FEN string if required
                    count++;
                }
                
                if(count == 3) 
                {
                    break;
                }
            }
            
            if(count == 3)
            {
                mList->status |= MOVE_DRAW_3MOVE_ACTIVE;
            }
            
            status = mList->status;    
        }
        
        toggleTurn();
    }
    

    return status;
    
}

STATUS CChessGame :: makeGeneratedMove(GENERATED_MOVELIST move)
{
    STATUS status;
    MOVELIST *mList, *tempList;
    CHESSPIECE_GENERAL newCp = CHESSPIECE_GENERAL(CHESSPIECE_TYPE((move.status >> 24)));
    CZobrist zob;
    UINT64 currentHash;
    UINT8 count = 1, currentSeqNum;
 
    status = makeMove(move.init, move.dest, newCp, move.status);
    
    if(!(status & MOVE_SUCCESS))
        return status;
    
    mList = (MOVELIST *)moveList.getTail();
    
    (void)updateFENString(mList->FEN);

    //check if 50 move repetition happened
    if(gState.draw50HalfMoveCount >= 50)
    {
        mList->status |= MOVE_DRAW_50MOVE_ACTIVE;
    }
            
    //check for insufficient material draw
    tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)mList);
        
    if((tempList != NULL && tempList->status & MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE) ||
       insufficientMatingMaterial(getLastFENString()) == TRUE)
    {
        mList->status |= MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE;
    }

    // Update zobrist Hash
    zob.updateBoardPosition(getLastFENString());
    zob.calculateZobristHash();
    mList->zobristHash = zob.getZobristHash();
    currentHash = mList->zobristHash;
    currentSeqNum = gState.seqNum;
            
    //check if 3 move repetition happened
    for(tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)mList);
        tempList != NULL;
        tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)tempList))
    {
        if(tempList->seqNum != currentSeqNum)
            break;
            
        if(tempList->zobristHash == currentHash)
        {
            //add more detailed check using FEN string if required
            count++;
        }
        
        if(count == 3) 
        {
            break;
        }
    }
            
    if(count == 3)
    {
        mList->status |= MOVE_DRAW_3MOVE_ACTIVE;
    }     
 
    status = mList->status;
    return status;
}


STATUS CChessGame :: makeGeneratedMove(MOVELIST move)
{
    STATUS status;
    CHECK_STATUS cStatus;
    MOVELIST *mList, *tempList;
    UINT64 currentHash;
    UINT8 count = 1, currentSeqNum;
    CZobrist zob;
    CHESSPIECE_GENERAL newCp = CHESSPIECE_GENERAL(CHESSPIECE_TYPE((move.status >> 24)));

    status = makeMove(move.init, move.dest, newCp, move.status);

    mList = (MOVELIST *)moveList.getTail();
        
    if(mList != NULL)
    {
        cStatus = scan4Check(move.init, move.dest, CHECK);
    	    
        if(isEndGame())
        {
            if(cStatus == CHECK)
            {
                status |= MOVE_CHECKMATE;
               //printf("Checkmate\n");
            }
            else
            {
                status |= MOVE_STALEMATE;
	            //printf("Stalemate\n");
            }
        }
        else if(cStatus == CHECK)
        {
                status |= MOVE_CHECK;
                //printf("Itz a check!!\n");
        }

        /* Form the SAN notation for the MOVE */
        mList->status = status; //This is needed for SAN
        (void)updateCANString(mList);
        (void)updateSANString(mList, TRUE);
        (void)updateFENString(mList->FEN);
        
        //check if 50 move repetition happened
        if(gState.draw50HalfMoveCount >= 50)
        {
            mList->status |= MOVE_DRAW_50MOVE_ACTIVE;
        }
            
        //check for insufficient material draw
        tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)mList);
        
        if((tempList != NULL && tempList->status & MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE) ||
           insufficientMatingMaterial(getLastFENString()) == TRUE)
        {
            mList->status |= MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE;
        }

        // Update zobrist Hash
        zob.updateBoardPosition(getLastFENString());
        zob.calculateZobristHash();
        mList->zobristHash = zob.getZobristHash();
        currentHash = mList->zobristHash;
        currentSeqNum = gState.seqNum;
            
        //check if 3 move repetition happened
        for(tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)mList);
            tempList != NULL;
            tempList = (MOVELIST *)moveList.getPrev((DLL_NODE *)tempList))
        {
            if(tempList->seqNum != currentSeqNum)
                break;
                
            if(tempList->zobristHash == currentHash)
            {
                //add more detailed check using FEN string if required
                count++;
            }
        
            if(count == 3) 
            {
                break;
            }
        }
            
        if(count == 3)
        {
            mList->status |= MOVE_DRAW_3MOVE_ACTIVE;
        }
        
        status = mList->status;     
    }
    
    return status;
}


STATUS CChessGame :: makeMove(BOARD_POSITION init, BOARD_POSITION dest, CHESSPIECE_GENERAL newCp,
                              STATUS status)
{
    MOVELIST *mList; 
    CHESSMENLIST temp, *cList;
    CAPTUREDLIST *captList;
    CHESSPIECE oldPiece, userPiece, initPiece;   
    INT8 moveDir = (dest.x > init.x) ? 1 : -1;
    UINT8 castleBase = (moveDir > 0) ? 7 : 0;
    UINT8 packPos = 0;
    BOOL castlingRightsChanged = FALSE;

    //printf("\n\nMaking Move.... ");
    
    if(!(status & MOVE_SUCCESS))
    {
        //printf("Illegal move! \n\n");                
        return ERROR;
    }
        //printf("Success!! \n\n");
        
    initPiece = getPiece(init);
    
	if(status & MOVE_EN_PASSENT)
	{
        oldPiece = getPiece(BOARD_POSITION(init.y, dest.x));
    }
	else
	{
            oldPiece = getPiece(dest);
    }

    status |= (oldPiece << 16);

	if(status & MOVE_EN_PASSENT_ACTIVE)
	{
	    packPos = PACKPOS(dest.x, ((init.y + dest.y)/2));
	    status |= (packPos << 24);
	}	
        
	if(status & MOVE_PAWN_PROMOTED)
    {
        userPiece = CHESSPIECE(MAKE_CHESSMEN(getCurrentTurn(), newCp));
	    insertPiece(dest, userPiece);
        status |= (userPiece << 24);
    }
	else
    {
	    insertPiece(dest, initPiece);
	}
    
	clearPiece(init);

	if(status & MOVE_CASTLING)
	{
	    insertPiece(BOARD_POSITION(dest.y, dest.x - moveDir), getPiece(BOARD_POSITION(init.y, castleBase)));
	    clearPiece(BOARD_POSITION(init.y, castleBase));
    } 
	
	if(status & MOVE_EN_PASSENT)
	{
	    clearPiece(BOARD_POSITION(init.y, dest.x));
    }


        /* Update the following figures for use for castling..
	 * Lets make things simple... dont even look for the pieces in the positions. 
	 * Just mark the pieces as moved if the init pos of a valid move matches them 
	 */

    if(memcmp(&init, &wKingPos, sizeof(BOARD_POSITION)) == 0)
	{
        gState.wKingMoved = TRUE;
        
        if(gState.wKingMoved == FALSE)
            castlingRightsChanged = TRUE;

	    if(status & MOVE_CASTLING) /* not useful... just for completeness sake */
	    {
	        if(moveDir > 0)
	        {
		        gState.wRCastleMoved = TRUE;
            }
            else
            {
		        gState.wLCastleMoved = TRUE;
            }
	    }
	}
    else if(memcmp(&init, &bKingPos, sizeof(BOARD_POSITION)) == 0)  
	{
        gState.bKingMoved = TRUE;

        if(gState.bKingMoved == FALSE)
            castlingRightsChanged = TRUE;

	    if(status & MOVE_CASTLING) /* just 4 completeness */
	    {
	        if(moveDir > 0)
		        gState.bRCastleMoved = TRUE;
            else
		        gState.wLCastleMoved = TRUE;
	    }

	}
    else if(memcmp(&init, &wLCastlePos, sizeof(BOARD_POSITION)) == 0 || 
                   memcmp(&dest, &wLCastlePos, sizeof(BOARD_POSITION)) == 0)
    {
        if(gState.wLCastleMoved == FALSE)
            castlingRightsChanged = TRUE;
         
        gState.wLCastleMoved = TRUE;
    }
    else if(memcmp(&init, &wRCastlePos, sizeof(BOARD_POSITION)) == 0 || 
                   memcmp(&dest, &wRCastlePos, sizeof(BOARD_POSITION)) == 0)  
    {
        if(gState.wRCastleMoved == FALSE)
            castlingRightsChanged = TRUE;
            
        gState.wRCastleMoved = TRUE;
    }
    else if(memcmp(&init, &bLCastlePos, sizeof(BOARD_POSITION)) == 0 || 
                   memcmp(&dest, &bLCastlePos, sizeof(BOARD_POSITION)) == 0)  
    {
        if(gState.bLCastleMoved == FALSE)
            castlingRightsChanged = TRUE;
            
        gState.bLCastleMoved = TRUE;
    }
    else if(memcmp(&init, &bRCastlePos, sizeof(BOARD_POSITION)) == 0 || 
                   memcmp(&dest, &bRCastlePos, sizeof(BOARD_POSITION)) == 0)  
    {
        if(gState.bRCastleMoved == FALSE)
            castlingRightsChanged = TRUE;
                               
        gState.bRCastleMoved = TRUE;
    }
            

    if(CHESSPIECE_TYPE(initPiece) != PAWN && !(status & MOVE_CAPTURE_PIECE))
    {
        gState.draw50HalfMoveCount++;
    }
    else
    {
        gState.draw50HalfMoveCount = 0;
    }            

    if(status & MOVE_CAPTURE_PIECE)
    {        
        if(status & MOVE_EN_PASSENT)
        {
	        temp.data = BOARD_POSITION(init.y, dest.x);
        }
	    else
	    {
	        temp.data = dest;        
        }
            
	    /* remove the captured piece from the chessmen list and add it to the captured pieces list */
        cList = (CHESSMENLIST *)chessmenList[CHESSPIECE_COLOR(oldPiece)].removeNode((DLL_NODE *)&temp); 
	    deleteChessmenListEntry(cList);

	    captList = newCapturedListEntry();
	    captList->piece = oldPiece;
	    capturedPiecesList[CHESSPIECE_COLOR(oldPiece)].insertAtTail((DLL_NODE *)captList);
    }
        
	temp.data = init;        
    cList = (CHESSMENLIST *)chessmenList[CHESSPIECE_COLOR(getPiece(dest))].findNode((DLL_NODE *)&temp);
        
	if(NULL != cList)
	{
        cList->data = dest;
    }

	if(status & MOVE_CASTLING)
	{
	    temp.data = BOARD_POSITION(init.y, castleBase);        
        cList = (CHESSMENLIST *)chessmenList[CHESSPIECE_COLOR(getPiece(dest))].findNode((DLL_NODE *)&temp);
        
	    if(NULL != cList)
	    {
            cList->data = BOARD_POSITION(dest.y, dest.x - moveDir);
        }
	}

    if(status & MOVE_CAPTURE_PIECE || CHESSPIECE_TYPE(initPiece) == PAWN || castlingRightsChanged == TRUE)
    {
        gState.seqNum = (gState.seqNum + 1) & 0xff;   
    }
    	
	mList = newMoveListEntry();
    mList->init = init;
    mList->dest = dest;
    mList->status = status;
    mList->seqNum = gState.seqNum;
    mList->SAN[0] = '\0';
    
    
    moveList.insertAtTail((DLL_NODE *)mList);
        
    return OK;
}

STATUS CChessGame :: unmakeMove(void)
{
    MOVELIST *lastMove = (MOVELIST *)moveList.removeTail();
    MOVELIST *temp;
    
    if(lastMove == NULL)
    {
        return ERROR;
    }

    initGame(getLastFENString(), FALSE, playerType[WHITE], playerType[BLACK]);
    
    if(lastMove->status & MOVE_CAPTURE_PIECE)
    {
        //remove last entry from captured pieces list
        deleteCapturedListEntry((CAPTUREDLIST *)capturedPiecesList[GET_OTHER_TURN(gState.turn)].removeTail());

        //add a new node for the captured piece
        chessmenList[GET_OTHER_TURN(gState.turn)].insertAtTail((DLL_NODE *)newChessmenListEntry());
    }
    
    updateChessmenList();    

    gState.gameProgress = GP_IN_PROGRESS;
    
    temp = (MOVELIST *)moveList.getTail();
    
    if(temp == NULL)
    {
        gState.seqNum = 0;
    }
    else
    {
        gState.seqNum = temp->seqNum;
    }
    
    deleteMoveListEntry(lastMove);
        
    return OK;
}

MOVE_OFFER CChessGame :: handleOfferDraw()
{
    //Opponent offered a draw..
    //Decide whether we are accepting that...
    //An offer can come only when a game is in progress
      
    if(gState.gameProgress == GP_IN_PROGRESS)
    {
        //Till this feature is implemented; always accept
        return OFFER_ACCEPTED;
    }
    else
    {
        return OFFER_REJECTED;
    }
}
