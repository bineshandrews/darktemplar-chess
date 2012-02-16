#include "../include/chess.h"

extern const  BOARD_POSITION wKingPos;
extern const BOARD_POSITION bKingPos;

int genMoveListCompare(const void *ptr1, const void *ptr2)
{
    GENERATED_MOVELIST *m1, *m2;
    
    m1 = (GENERATED_MOVELIST *)ptr1;
    m2 = (GENERATED_MOVELIST *)ptr2;    
    
    if(m1->boardVal > m2->boardVal) return 1;
    else if(m1->boardVal > m2->boardVal) return -1;
    else return 0;
}

STATUS CChessGame :: validateMove(BOARD_POSITION init, BOARD_POSITION dest, VALIDATION_TYPE vType = MVT_GENERAL)
{
    CHESSPIECE cp_init, cp_dest;
    CHESSPIECE_GENERAL cp;
    MOVELIST *enpMove;
    STATUS status = MOVE_NONE;
    INT8 xDiff, yDiff, x, y;
    UINT8 xAbsDiff, yAbsDiff;
    UINT8 pawnBase, pawnDestination;
    BOOL correctPawnAdvanceDir;
    BOARD_POSITION kingBase;
	
	/* here no values can be less than zero owing to the range of the data types */
	
	if(init.x >= MAXX || dest.x >= MAXX || init.y >= MAXY || dest.y >= MAXY)
	    return MOVE_ILLEGAL;

    cp_init = getPiece(init);
    cp_dest = getPiece(dest);

    if(!(vType & MVT_ALLOW_OUT_OF_TURN_MOVES) && CHESSPIECE_COLOR(cp_init) != gState.turn)
        return MOVE_ILLEGAL;
    
    if((cp_dest != INVALID_CHESSPIECE) &&  (CHESSPIECE_COLOR(cp_init) == CHESSPIECE_COLOR(cp_dest)))
        return MOVE_ILLEGAL;
        
    cp = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(cp_init));
    xDiff = dest.x - init.x;
    yDiff = dest.y - init.y;
    xAbsDiff = abs(xDiff);
    yAbsDiff = abs(yDiff); 
    
    if(xAbsDiff == 0 && yAbsDiff == 0)
        return MOVE_ILLEGAL;
        
    if(QUEEN == cp)
    {
        if(xAbsDiff == yAbsDiff)
            cp = BISHOP;
        else if(xAbsDiff == 0 || yAbsDiff == 0)
            cp = CASTLE;
    }
    
    switch(cp)
    {
        case KING:
             kingBase = (CHESSPIECE_COLOR(cp_init) == WHITE) ? wKingPos : bKingPos;

	     if(yAbsDiff == 0 && xAbsDiff == 2 && memcmp(&init, &kingBase,sizeof(BOARD_POSITION)) == 0)
	     {
	        if(isCastlingLegal(init, dest) == TRUE)
		    status |= MOVE_CASTLING;
		else
		    return MOVE_ILLEGAL;
	     }
             
	     else if(xAbsDiff > 1 || yAbsDiff > 1) 
                 return MOVE_ILLEGAL;
        break;
        
        case CASTLE:
            if(xAbsDiff !=0 && yAbsDiff != 0)
                return MOVE_ILLEGAL;

            for(y = MIN(init.y, dest.y) + 1; y < MAX(init.y, dest.y) ; y++)
            {
                  if(INVALID_CHESSPIECE != getPiece(BOARD_POSITION(y, init.x)))
                      return MOVE_ILLEGAL;
	        } 

	        for(x = MIN(init.x, dest.x) + 1; x < MAX(init.x, dest.x); x++)
            {
                      if(INVALID_CHESSPIECE != getPiece(BOARD_POSITION(init.y, x)))
                          return MOVE_ILLEGAL;
            }
        break;
        
        case BISHOP:
            if(xAbsDiff != yAbsDiff)
                return MOVE_ILLEGAL;
                 
            for(x = 1; x < xAbsDiff; x++)
            {
                if(INVALID_CHESSPIECE != getPiece(BOARD_POSITION(init.y + x*(yDiff/yAbsDiff), init.x + x*(xDiff/xAbsDiff))))
                    return MOVE_ILLEGAL;
            }
        break;

        case KNIGHT:
            if(xAbsDiff == 0 || xAbsDiff > 2 || yAbsDiff == 0 || yAbsDiff > 2)
                return MOVE_ILLEGAL;
             
            if((xAbsDiff ==1 && yAbsDiff == 1) || (xAbsDiff == 2 && yAbsDiff == 2))
                return MOVE_ILLEGAL;
        break;
        
        case PAWN:
            correctPawnAdvanceDir = ((cp_init == W_PAWN && yDiff > 0) || (cp_init == B_PAWN && yDiff < 0))?TRUE : FALSE;
	        pawnDestination = cp_init ==  W_PAWN ? 7 : 0;
            pawnBase = cp_init == W_PAWN ? 1 : 6;

            if(yAbsDiff == 0 || correctPawnAdvanceDir != TRUE)
                return MOVE_ILLEGAL;
                 
            if(yAbsDiff > 2 || xAbsDiff > 1)
                return MOVE_ILLEGAL;
                
            if(init.y != pawnBase && yAbsDiff == 2)
                return MOVE_ILLEGAL;
                 
            if(yAbsDiff == 2 && xAbsDiff != 0)
                return MOVE_ILLEGAL;

	        if(xAbsDiff == 0 && getPiece(dest) != INVALID_CHESSPIECE)
	            return MOVE_ILLEGAL;

            if(yAbsDiff == 2 && getPiece(BOARD_POSITION(init.y + yDiff/yAbsDiff, init.x)) != INVALID_CHESSPIECE)
                return MOVE_ILLEGAL;

	        if(xAbsDiff == 0 && yAbsDiff == 2)
                status |= MOVE_EN_PASSENT_ACTIVE;

            if(xAbsDiff !=0 && CHESSPIECE_TYPE(cp_dest) == INVALID_CHESSPIECE)
	        {
	            //hisPawnBase = MAXY - 1 - pawnBase;
	            enpMove = (MOVELIST *)moveList.getTail();

		        if(enpMove == NULL) return MOVE_ILLEGAL;

                /*unpack this irrespective of enpassent active; but use only when enpassent is active */
		        UNPACKPOS(((enpMove->status & 0xFF000000) >> 24),x,y);

	            if(xAbsDiff ==1 && yAbsDiff == 1 &&
		           enpMove->status & MOVE_EN_PASSENT_ACTIVE &&
		           x == dest.x && y == dest.y)
		        {
                    //printf("EnPassent\n");
	                status |= MOVE_EN_PASSENT;	     
		            status |= MOVE_CAPTURE_PIECE;
		        }
		        else
                {
                    //printf("EnPassent Illegal 0x%lx 0x%lx %u %u\n",enpMove->status,MOVE_EN_PASSENT_ACTIVE, x, y);
                    return MOVE_ILLEGAL;
                }
		     
	        }

	        if(dest.y == pawnDestination)
	        {
	            status |= MOVE_PAWN_PROMOTED;
	        }
            
        break;
        
        default:
            return MOVE_ILLEGAL;
    }
    
    status |= MOVE_SUCCESS;
    if(cp_dest != INVALID_CHESSPIECE)
        status |= MOVE_CAPTURE_PIECE;

    if(!(vType & MVT_SKIP_EXPOSED_4_CHECK_CASE))
    {
        if(isKingExposed(init, dest, status))
        {
            //printf("king exposed\n");
            return MOVE_ILLEGAL;
        }
    }

    return status;
}

void CChessGame :: getValidMoveList(BOARD_POSITION init, 
                                    GENERATED_MOVELIST *vMList,
                                    UINT32 *generatedMoveCount)
{
    /* MAXX == MAXY ... so this function may use one of them in place of them
     * other for code size reduction
     */
	
	/* Pawn promotion will be considered with each possible piece promoted */
	
    CHESSPIECE_GENERAL cpg, newCp[] = {QUEEN, CASTLE, KNIGHT, BISHOP};
    CHESSPIECE cp, userPiece, destCp;
    INT8 pawnAdvanceDir, i, j;
    UINT8 select;
    BOARD_POSITION dest;
    STATUS status;
    GENERATED_MOVELIST *mList;

    cp = getPiece(init);
    cpg = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(cp));
    pawnAdvanceDir = CHESSPIECE_COLOR(cp) == WHITE ? 1 : -1;

    switch(cpg)
    {
        case PAWN:
            dest = BOARD_POSITION(init.y + pawnAdvanceDir, init.x);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
            {
                if(status & MOVE_PAWN_PROMOTED)
                {
                    /* Have different moves with different pieces promoted */
                    for(select = 0; select < 4; select++)
                    {
                        userPiece = CHESSPIECE(MAKE_CHESSMEN(getCurrentTurn(), newCp[select]));

                        mList = &vMList[*generatedMoveCount];
                        mList->init = init;
                        mList->dest = dest;
                        mList->status = status | (userPiece << 24);
                        (*generatedMoveCount)++;
                    }
                }
                else
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }

            dest = BOARD_POSITION(init.y + 2*pawnAdvanceDir, init.x);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
            {
                mList = &vMList[*generatedMoveCount];
                mList->init = init;
                mList->dest = dest;
                mList->status = status;
                (*generatedMoveCount)++; 
            }

            dest = BOARD_POSITION(init.y + pawnAdvanceDir, init.x + 1);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
            {
                if(status & MOVE_PAWN_PROMOTED)
                {
                    /* Have different moves with different pieces promoted */
                    for(select = 0; select < 4; select++)
                    {
                        userPiece = CHESSPIECE(MAKE_CHESSMEN(getCurrentTurn(), newCp[select]));

                        mList = &vMList[*generatedMoveCount];
                        mList->init = init;
                        mList->dest = dest;
                        mList->status = status | (userPiece << 24);
                        (*generatedMoveCount)++;
                    }
                }
                else
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }

            dest = BOARD_POSITION(init.y + pawnAdvanceDir, init.x - 1);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
            {
                if(status & MOVE_PAWN_PROMOTED)
                {
                    /* Have different moves with different pieces promoted */
                    for(select = 0; select < 4; select++)
                    {
                        userPiece = CHESSPIECE(MAKE_CHESSMEN(getCurrentTurn(), newCp[select]));

                        mList = &vMList[*generatedMoveCount];
                        mList->init = init;
                        mList->dest = dest;
                        mList->status = status | (userPiece << 24);
                        (*generatedMoveCount)++;
                    }
                }
                else
                {                       
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }

            break;
		
        case CASTLE:
        case QUEEN:
            for(i = init.x - 1; i >= 0; i--)
            {
                dest = BOARD_POSITION(init.y, i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;

                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
	        }

            for(i = init.x + 1; i < MAXX; i++)
            {
                dest = BOARD_POSITION(init.y, i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
	        }

            for(i = init.y - 1 ; i >= 0; i--)
            {
                dest = BOARD_POSITION(i, init.x);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
	        }

            for(i = init.y + 1 ; i < MAXY; i++)
            {
                dest = BOARD_POSITION(i, init.x);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
	        }

            if(cpg != QUEEN) 
                break;

        case BISHOP:	
            for(i = -1; i >= -MIN(init.x, init.y);  i--)
            {
                dest = BOARD_POSITION(init.y + i, init.x + i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }

            for(i = 1; i < MAXX - MAX(init.x, init.y); i++)
            {
                dest = BOARD_POSITION(init.y + i, init.x + i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }

            for(i = -1; i >= -MIN(MAXX - 1 - init.x, init.y); i--)
            {
                dest = BOARD_POSITION(init.y + i, init.x - i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }

            for(i = 1; i < MAXX - MAX(MAXX - 1 - init.x, init.y); i++)
            {
                dest = BOARD_POSITION(init.y + i, init.x - i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                {
                    mList = &vMList[*generatedMoveCount];
                    mList->init = init;
                    mList->dest = dest;
                    mList->status = status;
                    (*generatedMoveCount)++; 
                }
            }
            
	    break;

        case KNIGHT:
            for(i = -1; i <= 1; i += 2)
            {
                for(j = -2; j <= 2; j += 4)
                {
                    dest = BOARD_POSITION(init.y + i, init.x + j);
                    if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    {
                        mList = &vMList[*generatedMoveCount];
                        mList->init = init;
                        mList->dest = dest;
                        mList->status = status;
                        (*generatedMoveCount)++; 
                    }

                    dest = BOARD_POSITION(init.y + j, init.x + i);
                    if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    {
                        mList = &vMList[*generatedMoveCount];
                        mList->init = init;
                        mList->dest = dest;
                        mList->status = status;
                        (*generatedMoveCount)++; 
                    }
                }
            }

            break;

        case KING:
            for(i = -1; i <= 1; i++)
            {
                for(j = -2; j <= 2; j++)
                {
                    if( i == 0 && j == 0)
                        continue;

		            if((j == 2 || j == -2) && i != 0)
		                continue;

                    dest = BOARD_POSITION(init.y + i, init.x + j);
                    if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    {
                        mList = &vMList[*generatedMoveCount];
                        mList->init = init;
                        mList->dest = dest;
                        mList->status = status;
                        (*generatedMoveCount)++; 
                    }
                }
            }				

            break;
    }
}

UINT8 CChessGame :: getValidMoveCount(BOARD_POSITION init)
{
    /* MAXX == MAXY ... so this function may use one of them in place of them
     * other for code size reduction
     */

    /* Count Pawn promotion as 1 move only. */
    	
    CHESSPIECE_GENERAL cpg;
    CHESSPIECE cp, destCp;
    INT8 pawnAdvanceDir, i, j;
    BOARD_POSITION dest;
    STATUS status;
    UINT8 validMoveCnt = 0;
	
    cp = getPiece(init);

    cpg = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(cp));
    pawnAdvanceDir = CHESSPIECE_COLOR(cp) == WHITE ? 1 : -1;
	
    switch(cpg)
    {	
        case PAWN:
            dest = BOARD_POSITION(init.y + pawnAdvanceDir, init.x);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                validMoveCnt++;
                
            dest = BOARD_POSITION(init.y + 2*pawnAdvanceDir, init.x);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                validMoveCnt++;
                
            dest = BOARD_POSITION(init.y + pawnAdvanceDir, init.x + 1);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                validMoveCnt++;
                
            dest = BOARD_POSITION(init.y + pawnAdvanceDir, init.x - 1);
            if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                validMoveCnt++;	
  
            break;
		
        case CASTLE:
        case QUEEN:
            for(i = init.x - 1; i >= 0; i--)
            {
                dest = BOARD_POSITION(init.y, i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    validMoveCnt++;
            }

            for(i = init.x + 1; i < MAXX; i++)
            {
                dest = BOARD_POSITION(init.y, i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    validMoveCnt++;
	        }

            for(i = init.y - 1; i >= 0; i--)
            {
                dest = BOARD_POSITION(i, init.x);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    validMoveCnt++;
	        }

            for(i = init.y + 1; i < MAXY; i++)
            {
                dest = BOARD_POSITION(i, init.x);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    validMoveCnt++;
	        }

            if(cpg != QUEEN) 
                break;

        case BISHOP:	
            for(i = -1; i >= -MIN(init.x, init.y);i--)
            {
                dest = BOARD_POSITION(init.y + i, init.x + i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
		            validMoveCnt++;
            }
			
            for(i = 1; i < MAXX - MAX(init.x, init.y); i++)
            {
                dest = BOARD_POSITION(init.y + i, init.x + i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
		            validMoveCnt++;
            }

            for(i = -1; i >= -MIN(MAXX - 1 - init.x, init.y);i--)
            {
                dest = BOARD_POSITION(init.y + i, init.x - i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    validMoveCnt++;
            }
			
            for(i = 1; i < MAXX - MAX(MAXX - 1 - init.x, init.y); i++)
            {
                dest = BOARD_POSITION(init.y + i, init.x - i);

                destCp = getPiece(dest);
                if(destCp != INVALID_CHESSPIECE &&
                   CHESSPIECE_COLOR(destCp) == gState.turn)
		            break;
                
                if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                    validMoveCnt++;
            }

    	    break;

        case KNIGHT:
            for(i = -1; i <= 1; i += 2)
            {
                for(j = -2; j <= 2; j += 4)
                {
                    dest = BOARD_POSITION(init.y + i, init.x + j);
                    if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
                        validMoveCnt++;

                    dest = BOARD_POSITION(init.y + j, init.x + i);
                    if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
		        validMoveCnt++;
                }
            }

            break;
		
        case KING:
            for(i = -1; i <= 1; i++)
            {
                for(j = -2; j <= 2; j++)
                {
                    if( i == 0 && j == 0)
                        continue;
		    
		    if((j == 2 || j == -2) && i != 0)
		        continue;

                    dest = BOARD_POSITION(init.y + i, init.x + j);
                    if((status = validateMove(init, dest, MVT_GENERAL)) & MOVE_SUCCESS)
		        validMoveCnt++;
                }
            }				
            
	    break;
    }

    return validMoveCnt;
}


void CChessGame :: getValidMoveList(GENERATED_MOVELIST *vMList,
                                    UINT32 *generatedMoveCount)
{
    CHESSMENLIST *node;
    UINT8 turn = getCurrentTurn();

    node = (CHESSMENLIST *)chessmenList[turn].getHead();

    while(node)
    {
        getValidMoveList(node->data, vMList, generatedMoveCount);

        node = (CHESSMENLIST *)chessmenList[turn].getNext((DLL_NODE*)node);
    }
}

UINT8 CChessGame :: getValidMoveCount()
{
    CHESSMENLIST *node;
    INT8 validMoveCnt = 0;
    UINT8 turn = getCurrentTurn();

    node = (CHESSMENLIST *)chessmenList[turn].getHead();

    while(node)
    {
        validMoveCnt += getValidMoveCount(node->data);
        
	    node = (CHESSMENLIST *)chessmenList[turn].getNext((DLL_NODE*)node);
    }

    return validMoveCnt;
}

STATUS CChessGame :: getRandomMove(MOVELIST *randomMove)
{
    UINT32 numPossibleMoves = 0, randomIndex = 0;
    
    getValidMoveList(&generatedMoves[0][0], &numPossibleMoves);
    
    if(numPossibleMoves == 0)
    {
        return ERROR;
    }
    
    randomIndex = getRandomNumber(0, numPossibleMoves-1);
    
    randomMove->init = generatedMoves[0][randomIndex].init;
    randomMove->dest = generatedMoves[0][randomIndex].dest;
    randomMove->status = generatedMoves[0][randomIndex].status;
    
    return OK;    
}


STATUS CChessGame :: getBestMove(MOVELIST *bestMove)
{
    GENERATED_MOVELIST tempMove;
    
    if(recFindBestMove(0, MAX_PLAY_DEPTH, &tempMove, -32768, 32767) == OK)
    {
        bestMove->init = tempMove.init;
        bestMove->dest = tempMove.dest;
        bestMove->status = tempMove.status;
//        printf("Eval points = %ld\n", tempMove.boardVal);
        return OK;
    }
    else
    {
        return ERROR;
    }
    
}

STATUS CChessGame :: recFindBestMove(UINT32 recDepth, UINT32 maxDepth,
                                     GENERATED_MOVELIST *bestMove,
                                     INT32 alpha, INT32 beta)
{
    UINT32 numPossibleMoves = 0, i = 0;
    GENERATED_MOVELIST *tempMove, alphaMove, bMove;
    INT32 bestBoardVal = -32769;
    CHECK_STATUS cStatus;
    
    recDepth++;
    
    getValidMoveList(&generatedMoves[recDepth-1][0], &numPossibleMoves);
    
    if(numPossibleMoves == 0)
    {
        return ERROR;
    }

    if(recDepth > maxDepth)
    {
        //static evaluation
        //toggleTurn();
        bestMove->boardVal = evaluateStaticBoardPosition(getCurrentTurn());
        //toggleTurn();
  
        return OK;
    }

    for(i = 0; i < numPossibleMoves; i++)
    {
        tempMove = &(generatedMoves[recDepth-1][i]);
        tempMove->boardVal = evaluateStaticBoardPosition(getCurrentTurn());
    }
    
    qsort(&(generatedMoves[recDepth-1][0]), numPossibleMoves, 
          sizeof(GENERATED_MOVELIST), &genMoveListCompare);

    for(i = 0; i < numPossibleMoves; i++)
    {
        tempMove = &(generatedMoves[recDepth-1][i]);
        tempMove->boardVal = 0;
        
        makeGeneratedMove(*tempMove);
        //printf("Made move..\n");

        toggleTurn();
        
        if(recFindBestMove(recDepth, maxDepth, &bMove, -beta, -alpha) == ERROR)
        {
            //END-GAME
            toggleTurn();
     	    cStatus = scan4Check(tempMove->init, tempMove->dest, CHECK);         
            
            if(cStatus == CHECK)
            {
                tempMove->boardVal = evaluateStaticBoardPosition(getCurrentTurn());                       
                tempMove->boardVal += 20000;
                tempMove->boardVal += ((MAX_PLAY_DEPTH+MAX_QUIES_SEARCH_DEPTH)-recDepth)*1000;
                *bestMove = *tempMove;
                unmakeMove();
                break;
            }
            else
            {
                //stalemate - change this later
                tempMove->boardVal = evaluateStaticBoardPosition(getCurrentTurn());
            }
        }
        else
        {
            tempMove->boardVal = -bMove.boardVal; 
        }

        //  printf("Unmaking move..\n");
        unmakeMove();
        //toggleTurn();
    
        if(i == 0)
        {
           bestBoardVal = tempMove->boardVal;
           *bestMove = *tempMove;            
           alpha = bestBoardVal;                
           alphaMove = *tempMove;
        }
        else
        {
        
            if(bestBoardVal < tempMove->boardVal)
            {
                bestBoardVal = tempMove->boardVal;
                *bestMove = *tempMove;            
            }
              
            if(bestBoardVal > alpha)
            {
               alpha = bestBoardVal;
               alphaMove = *tempMove;                             
            }
        
            if(alpha > beta)
            {
                *bestMove = alphaMove;
                break;
            }
        }        
    }
    
    //printf("Best move val = %ld\n", bestBoardVal); 
    //printf("Returned from recDepth.. %ld\n", recDepth);   
    return OK;                       
}
