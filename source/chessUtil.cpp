#include "../include/chess.h"
#include "../libh/zobrist.h"

STATUS compareChessmen(void *data1, void *data2)
{
    BOARD_POSITION *bp1, *bp2;

    bp1 = (BOARD_POSITION *)data1;
    bp2 = (BOARD_POSITION *)data2;

    if(bp1->x == bp2->x && bp1->y == bp2->y) 
        return OK;

    return ERROR; 
}

void CChessGame :: initGeneratedMoves()
{
    UINT32 i;
    
    generatedMoves = new GENERATED_MOVELIST *[MAX_REC_DEPTH];
    
    for(i = 0; i < MAX_REC_DEPTH; i++)
    {
        generatedMoves[i] = new GENERATED_MOVELIST[MAX_MOVES_PER_POSITION];
    }
}

void CChessGame :: deInitGeneratedMoves()
{
    UINT32 i;
    
    for(i = 0; i < MAX_REC_DEPTH; i++)
    {
        delete generatedMoves[i];
    }
    
    delete generatedMoves;
}

void CChessGame :: initFreeLists()
{
     MOVELIST *mList;
     CAPTUREDLIST *cList;
     CHESSMENLIST *cmList;
     UINT32 i;
     
     for(i = 0; i < MAX_FREE_MOVELIST_ENTRIES; i++)
     {
         mList = new MOVELIST;
         freeMoveList.insertAtTail((DLL_NODE *)(mList));
     }
     
     for(i = 0; i < 32; i++)
     {
         //16 * 2 sides (excluding kings..)
         cList = new CAPTUREDLIST;
         freeCapturedPiecesList.insertAtTail((DLL_NODE *)(cList));
     }  
     
     for(i = 0; i < 32; i++)
     {
         //16 * 2 sides (excluding kings..)
         cmList = new CHESSMENLIST;
         freeChessmenList.insertAtTail((DLL_NODE *)(cmList));
     }      
}

void CChessGame :: deInitFreeLists()
{
    MOVELIST *mList;
    CAPTUREDLIST *cList;
    CHESSMENLIST *cmList;

    while((mList = (MOVELIST *)freeMoveList.removeHead()) != NULL)
    {
        delete mList;
    }

    while((cList = (CAPTUREDLIST *)freeCapturedPiecesList.removeHead()) != NULL)
    {
        delete cList;
    }
    
    while((cmList = (CHESSMENLIST *)freeChessmenList.removeHead()) != NULL)
    {
        delete cmList;
    }    
}

void CChessGame :: setInitFEN(const char *fenString)
{
    strncpy(initFEN, fenString, FEN_STRING_LENGTH);    
}

char * CChessGame :: getInitFEN()
{
    return initFEN;
}

void CChessGame :: initGame(const char *fenString, BOOL createChessmenList,
                            PLAYER_TYPE white, PLAYER_TYPE black)
{
    INT16 i, j, k, l;
    char FEN[FEN_STRING_LENGTH];
    char *fenToken;

    strncpy(FEN, fenString, FEN_STRING_LENGTH);
    FEN[FEN_STRING_LENGTH-1] = '\0';

    //cleanupLists();

    fenToken = strtok(FEN, " /");
    for(i = MAXY-1; i >= 0; i--)
    {
        for(j = 0, k = 0; j < MAXX; j++, k++)
        {
            if(fenToken[k] >= '1' && fenToken[k] <= '8')
            {
                for(l = 0; l < fenToken[k] - '0'; l++)
                {
                    clearPiece(BOARD_POSITION(i, j + l));
                }
                
                j += (fenToken[k] - '1');
                continue;
            }
            
            if(isalpha(fenToken[k]))
            {
                insertPiece(BOARD_POSITION(i,j), strToPiece(fenToken[k]));
            }
        }
        
        fenToken = strtok(NULL, " /");
    }

    gState.gameProgress = GP_IN_PROGRESS;

    if(createChessmenList == TRUE)
    {
        generateChessmenList();
    }
    
    playerType[MT_WHITE] = white;
    playerType[MT_BLACK] = black;    

    if(fenToken == NULL)
    {
        return;
    }    
    
    if(fenToken[0] == 'w')
        gState.turn = MT_WHITE;
    else
        gState.turn = MT_BLACK;

    // castling and other info forbid castling by default           
    gState.wKingMoved = TRUE;
    gState.bKingMoved = TRUE;
    gState.wRCastleMoved = TRUE;
    gState.wLCastleMoved = TRUE;
    gState.bRCastleMoved = TRUE;
    gState.bLCastleMoved = TRUE;
    
    fenToken = strtok(NULL, " ");
    if(fenToken == NULL)
    {
        return;
    }
    
    
    if(fenToken[0] == '-')
    {
        //all set
    }
    else
    {
        for(i = 0; i < (INT8)strlen(fenToken); i++)
        {
            if(fenToken[i] == 'K')
            {
                if(game.board[0][4] == W_KING && game.board[0][7] == W_CASTLE)
                {
                    gState.wKingMoved = FALSE;
                    gState.wRCastleMoved = FALSE;
                }
            }
            else if(fenToken[i] == 'Q')
            {
                if(game.board[0][4] == W_KING && game.board[0][0] == W_CASTLE)
                {
                    gState.wKingMoved = FALSE;
                    gState.wLCastleMoved = FALSE;            
                }
            }
            else if(fenToken[i] == 'k')
            {
                if(game.board[7][4] == B_KING && game.board[7][7] == B_CASTLE)
                {
                    gState.bKingMoved = FALSE;
                    gState.bRCastleMoved = FALSE;
                }
            }
            else if(fenToken[i] == 'q')
            {
                if(game.board[7][4] == B_KING && game.board[7][0] == B_CASTLE)
                {
                    gState.bKingMoved = FALSE;
                    gState.bLCastleMoved = FALSE;            
                }
            }
        }
    }

    //enp square
    fenToken = strtok(NULL, " ");
    if(fenToken == NULL)
    {
        return;
    }
    else
    {
        //if movelist is empty add the pawn move in the movelist so that enpassent capture can take place
      	while(*fenToken == ' ') fenToken++;
      	
      	if(moveList.getTail() == NULL && fenToken[0] != '-')
        {
            INT8 beginning = (fenToken[1] - '1') > 3 ? 6 : 1;
            INT8 end       = beginning == 6 ? 4 : 3;
            INT8 packPos;
            MOVELIST *mList;
            CZobrist zob;
            
            mList = newMoveListEntry();
            mList->init = BOARD_POSITION(beginning, fenToken[0]-'a');
            mList->dest = BOARD_POSITION(end, fenToken[0]-'a'); 
            strncpy(mList->FEN, fenString, FEN_STRING_LENGTH);
            mList->FEN[FEN_STRING_LENGTH-1] = '\0';
            mList->seqNum = 0;
            mList->status = MOVE_SUCCESS | MOVE_EN_PASSENT_ACTIVE;
            
            packPos = PACKPOS((fenToken[0]-'a'), (fenToken[1]-'1'));
	        mList->status |= (packPos << 24);
            
            zob.updateBoardPosition(fenString);
            zob.calculateZobristHash();
            mList->zobristHash = zob.getZobristHash();
            
            moveList.insertAtTail((DLL_NODE *)mList);
        }

    }
    //half move count
    fenToken = strtok(NULL, " ");
    if(fenToken == NULL)
    {
        return;
    }  
    
    gState.draw50HalfMoveCount = atoi(fenToken);
    
    return;  
}

void CChessGame :: cleanupLists()
{
    DLL_NODE *node;
    CHESSMENLIST *cList;
    MOVELIST *mList;
    CAPTUREDLIST *cpList;

    while(NULL != (node = chessmenList[WHITE].removeTail()))
    {
        cList = (CHESSMENLIST *)node;
        deleteChessmenListEntry(cList);
    }

    while(NULL != (node = chessmenList[BLACK].removeTail()))
    {
        cList = (CHESSMENLIST *)node;
        deleteChessmenListEntry(cList);
    }

    while(NULL != (node = moveList.removeTail()))
    {
        mList = (MOVELIST *)node;
        deleteMoveListEntry(mList);
    }

    while(NULL != (node = capturedPiecesList[WHITE].removeTail()))
    {
        cpList = (CAPTUREDLIST *)node;
        deleteCapturedListEntry(cpList);
    }

    while(NULL != (node = capturedPiecesList[BLACK].removeTail()))
    {
        cpList = (CAPTUREDLIST *)node;
        deleteCapturedListEntry(cpList);
    }

    return;
}

BOARD_POSITION CChessGame :: invertPosition(BOARD_POSITION bp)
{
    BOARD_POSITION final;

    final.x = MAXX - 1 - bp.x;
    final.y = MAXY - 1 - bp.y;

    return final;
}

BOARD_POSITION CChessGame :: convertPosition2Absolute(BOARD_POSITION bp)
{
    BOARD_POSITION final;

    final = (BS_NORMAL == getBoardStatus()) ? bp : invertPosition(bp);

    return final;
}

CHESSPIECE CChessGame :: strToPiece(char charPc)
{
    UINT32 color = isupper(charPc) ? WHITE : BLACK;
    
    switch(charPc)
    {
        case 'P':
        case 'p':
            return CHESSPIECE(MAKE_CHESSMEN(color, PAWN));
            
        case 'R':
        case 'r':
            return CHESSPIECE(MAKE_CHESSMEN(color, CASTLE));

        case 'N':
        case 'n':
            return CHESSPIECE(MAKE_CHESSMEN(color, KNIGHT));

        case 'B':
        case 'b':
            return CHESSPIECE(MAKE_CHESSMEN(color, BISHOP));

        case 'K':
        case 'k':
            return CHESSPIECE(MAKE_CHESSMEN(color, KING));

        case 'q':
        case 'Q':
            return CHESSPIECE(MAKE_CHESSMEN(color, QUEEN));

        default:
            return INVALID_CHESSPIECE;        
    }
}

char* CChessGame :: toString(CHESSPIECE piece)
{
      const char *ptr;

      switch(piece)
      {
          case W_PAWN:
               ptr = "P";
               break;
          case B_PAWN:
               ptr = "p";
               break;
          case W_CASTLE:
               ptr = "R";
               break;
          case B_CASTLE:
               ptr = "r";
               break;
          case W_KNIGHT:
               ptr = "N";
               break;
          case B_KNIGHT:
               ptr = "n";
               break;
          case W_BISHOP:
               ptr = "B";
               break;
          case B_BISHOP:
               ptr = "b";
               break;
          case W_QUEEN:
               ptr = "Q";
               break;
          case B_QUEEN:
               ptr = "q";
               break;
          case W_KING:
               ptr = "K";
               break;
          case B_KING:
               ptr = "k";
               break;
          default:
               ptr = " ";
      }

    return (char *)ptr;
}

void CChessGame :: updateFENString(char *fenString)
{
    INT16 i, j, counter = 0, strIndex = 0;
    CHESSPIECE piece;
    char strBuffer[5];
    MOVELIST *enpMove;
    INT8 x,y;
    
    if(fenString == NULL)
    {
        return;
    }
    
    for(i = MAXY-1; i >= 0; i--)
    {
        counter = 0;
        for(j = 0; j < MAXX; j++)
        {
            piece = getPiece(BOARD_POSITION(i,j));
            
            if(piece != INVALID_CHESSPIECE)
            {
                if(counter != 0)
                {
                    fenString[strIndex++] = '0' + counter;
                    counter = 0;
                }
                fenString[strIndex++] = toString(piece)[0];
            }
            else
            {
                counter++;
            }
        }
        
        if(counter != 0)
        {
            fenString[strIndex++] = '0' + counter;
            counter = 0;
        }
        
        fenString[strIndex++] = '/';
    }
    
    fenString[strIndex-1] = ' ';
    
    toggleTurn();
    if(gState.turn == MT_WHITE)
    {
        fenString[strIndex++] = 'w';
    }
    else
    {
        fenString[strIndex++] = 'b';
    }
    toggleTurn();
    
    fenString[strIndex++] = ' ';
    
    counter = 0;
    
    if(!gState.wKingMoved && !gState.wRCastleMoved)
    {
        fenString[strIndex++] = 'K';
        counter++;
    }
    
    if(!gState.wKingMoved && !gState.wLCastleMoved)
    {
        fenString[strIndex++] = 'Q';
        counter++;
    }

    if(!gState.bKingMoved && !gState.bRCastleMoved)
    {
        fenString[strIndex++] = 'k';
        counter++;
    }

    if(!gState.bKingMoved && !gState.bLCastleMoved)
    {
        fenString[strIndex++] = 'q';
        counter++;
    }
    
    if(counter == 0)
    {
        fenString[strIndex++] = '-';
    }
    
    fenString[strIndex++] = ' ';
    enpMove = (MOVELIST *)moveList.getTail();
    if(enpMove == NULL)
    {
        fenString[strIndex++] = '-';
    }
    else
    {
        UNPACKPOS(((enpMove->status & 0xFF000000) >> 24),x,y);
        if(enpMove->status & MOVE_EN_PASSENT_ACTIVE)
        {
            fenString[strIndex++] = 'a' + x;
            fenString[strIndex++] = '1' + y;
        }
        else
        {
            fenString[strIndex++] = '-';
        }
    }
    
    fenString[strIndex++] = ' ';
    fenString[strIndex] = '\0';
    //temp values for 50 move rule half moves
    sprintf(strBuffer, "%lu", gState.draw50HalfMoveCount);
    strcat(fenString, strBuffer);
    strcat(fenString, " ");
    strBuffer[0] = '\0';    
    
    //Movecount
    sprintf(strBuffer, "%lu", 1 + this->moveList.getListSize()/2);
    strcat(fenString, strBuffer);
    
    return;
}

char * CChessGame :: getLastFENString()
{
    MOVELIST *lastMove;
    
    lastMove = (MOVELIST *)moveList.getTail();
    
    if(lastMove != NULL)
    {
        return lastMove->FEN;
    }
    else
    {
        return initFEN;
    }
    
    return (char *)"";
}

STATUS CChessGame :: decodeMoveString(char *moveString, MOVELIST *mList)
{
    CHESSPIECE_GENERAL newPiece;
    char c;
    
    if(moveString[0] < 'a' || moveString[0] > 'h' ||
       moveString[1] < '1' || moveString[1] > '8' ||
       moveString[2] < 'a' || moveString[2] > 'h' ||
       moveString[3] < '1' || moveString[3] > '8' )
    {
        return ERROR;
    }    
    
    mList->init.x=moveString[0] - 'a';
    mList->init.y=moveString[1] - '1';
    mList->dest.x=moveString[2] - 'a';
    mList->dest.y=moveString[3] - '1';

    c = toupper(moveString[4]);

    if(moveString[4] == '\0')
    {
        //Do nothing :-)
    }
    else if(moveString[4] == '=' || c == 'Q' || c == 'R' || c == 'N' || c == 'B')
    {
        if(c == 'Q' || moveString[5] == 'Q' || moveString[5] == 'q')
            newPiece = QUEEN;
        else if(c == 'R' || moveString[5] == 'R' || moveString[5] == 'r')
            newPiece = CASTLE;
        else if(c == 'N' || moveString[5] == 'N' || moveString[5] == 'n')
            newPiece = KNIGHT;
        else if(c == 'B' || moveString[5] == 'B' || moveString[5] == 'b')
            newPiece = BISHOP;
        else
        {        
            newPiece = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(INVALID_CHESSPIECE));
            return ERROR;
        }
        
        mList->status |= (newPiece << 24);
        mList->status |= MOVE_PAWN_PROMOTED;
    }
    
    return OK;
}

STATUS CChessGame :: decodeSANString(char *sanString, MOVELIST *mList)
{
    CHESSPIECE_GENERAL type, newPiece;
    CHESSPIECE piece;
    CHESSMENLIST *node;
    CDLList *cmenList = &chessmenList[gState.turn];
    UINT8 turn = gState.turn;
    UINT8 format = 0, i = 0, promoPos = 0, candidateCount = 0;
    
    enum AmbiguityResolv {BOTH_UNKNOWN = 0, RANK_KNOWN =1, FILE_KNOWN =2, BOTH_KNOWN = 3} ambig;
    
    mList->status = 0;
    ambig = BOTH_UNKNOWN;
    
    if(strncmp(sanString, "O-O-O",5) == 0)
    {
        type = KING;
        mList->status |= MOVE_CASTLING;
        
        if(turn == WHITE)
        {
            mList->init.x = 4;
            mList->init.y = 0;
            mList->dest.x = 2;
            mList->dest.y = 0;
            return OK;
        }
        else
        {
            mList->init.x = 4;
            mList->init.y = 7;
            mList->dest.x = 2;
            mList->dest.y = 7;
            return OK;
        }        
    }
    else if(strncmp(sanString, "O-O", 3) == 0)
    {
        type = KING;
        mList->status |= MOVE_CASTLING;
        
        if(turn == WHITE)
        {
            mList->init.x = 4;
            mList->init.y = 0;
            mList->dest.x = 6;
            mList->dest.y = 0;
            return OK;
        }
        else
        {
            mList->init.x = 4;
            mList->init.y = 7;
            mList->dest.x = 6;
            mList->dest.y = 7;
            return OK;
        }
    }
    else if(sanString[0] >= 'a' && sanString[0] <= 'h')
    {
        /* This is a pawn */
        type = PAWN;
        
        if(sanString[1] != 'x')
        {
            ambig = BOTH_UNKNOWN;
            mList->dest.x = sanString[0] - 'a';
            mList->dest.y = sanString[1] - '1';
            
            if(sanString[2] != '\0' && sanString[2] != '=' && sanString[2] != '+' && 
               sanString[2] != '#'  && sanString[2] != '?' && sanString[2] != '.' &&
               sanString[2] != '!')
            {
                //special check to detect wrong format of input
                return ERROR;
            }
            promoPos = 2;
        }
        else
        {
            ambig = RANK_KNOWN;
            mList->init.x = sanString[0] - 'a';
            mList->dest.x = sanString[2] - 'a';
            mList->dest.y = sanString[3] - '1';
            mList->status |= MOVE_CAPTURE_PIECE;
            
            if(strncmp(sanString+4, "(ep)", 4) == 0)
            {
                mList->status |= MOVE_EN_PASSENT;
            }
            else
            {            
                promoPos = 4;
            }
        }    
            
        if(sanString[promoPos] == '=')
        {
            if(sanString[promoPos+1] == 'R')
                newPiece = CASTLE;
            else if(sanString[promoPos+1] == 'N')
                newPiece = KNIGHT;
            else if(sanString[promoPos+1] == 'B')
                newPiece = BISHOP;
            else if(sanString[promoPos+1]  == 'Q')
                 newPiece = QUEEN;
            else 
                return ERROR;
                
            mList->status |= MOVE_PAWN_PROMOTED;
            mList->status |= (newPiece << 24);
        }
        
    }
    else
    {
        for(i = 1; i < 10 && sanString[i] != '\0'; i++)
        {
            if((sanString[i] >= 'a' && sanString[i] <= 'h') ||
               (sanString[i] >= '1' && sanString[i] <= '8') ||
               (sanString[i] == 'x'))
            {
                if(sanString[i] == 'x')
                {
                    mList->status |= MOVE_CAPTURE_PIECE;
                }
                else
                {
                   format++;
                }
            }
            else
            {
                break;
            }
        }
        
        if(format == 2 || format == 3 || format ==4)
        {
            if(mList->status & MOVE_CAPTURE_PIECE)
            {
                mList->dest.x = sanString[format] - 'a';
                mList->dest.y = sanString[format+1] - '1';
            }
            else
            {
                mList->dest.x = sanString[format - 1] - 'a';
                mList->dest.y = sanString[format] - '1';
            }
            
            if(format == 3)
            {
                if(sanString[1] >= 'a' && sanString[1] <= 'h')
                {
                    mList->init.x = sanString[1] - 'a';
                    ambig = RANK_KNOWN;
                }
                else if(sanString[1] >= '1' && sanString[1] <= '8')
                {
                    mList->init.y = sanString[1] - '1';
                    ambig = FILE_KNOWN;
                }
                else
                    return ERROR;
            }
            else if( format == 4)
            {
                mList->init.x = sanString[1] - 'a';
                mList->init.y = sanString[2] - '1';
                ambig = BOTH_KNOWN;
            }            
        }
        else
            return ERROR;
        
        if(sanString[0] == 'R')
            type = CASTLE;
        else if(sanString[0] == 'N')
            type = KNIGHT;
        else if(sanString[0] == 'B')
            type = BISHOP;
        else if(sanString[0]  == 'Q')
            type = QUEEN;
        else if(sanString[0] == 'K')
            type = KING;
        else 
            return ERROR;
    }
    
    piece = CHESSPIECE(MAKE_CHESSMEN(turn, type));
    node = (CHESSMENLIST *)cmenList->getHead();
    
    while(node)
    {
        if(getPiece(node->data) == piece)
        {
            if(ambig == BOTH_KNOWN)
            {
                if(node->data.x == mList->init.x && node->data.y == mList->init.y)
                {
                    if(validateMove(mList->init, mList->dest, MVT_GENERAL) & MOVE_SUCCESS)
                    {
                        candidateCount++;
                    }
                }
                 
            }
            
            else if(ambig == RANK_KNOWN)
            {
                if(node->data.x == mList->init.x)
                {
                    if(validateMove(node->data, mList->dest, MVT_GENERAL) & MOVE_SUCCESS)
                    {
                        mList->init.y = node->data.y;
                        candidateCount++;
                    }
                }
            }
            
            else if(ambig == FILE_KNOWN)
            {
                if(node->data.y == mList->init.y)
                {
                    if(validateMove(node->data, mList->dest, MVT_GENERAL) & MOVE_SUCCESS)
                    {
                        mList->init.x = node->data.x;
                        candidateCount++;
                    }
                }
            }
            
            else if(ambig == BOTH_UNKNOWN)
            {
                if(validateMove(node->data, mList->dest, MVT_GENERAL) & MOVE_SUCCESS)
                {
                    mList->init.x = node->data.x;
                    mList->init.y = node->data.y;                
                    candidateCount++;
                }
            }
        }
        
        node = (CHESSMENLIST *)cmenList->getNext((DLL_NODE*)node);
    }
    
    if(candidateCount == 0)
    {
        return ERROR;
    }
    else if (candidateCount > 1)
    {
        mList->status |= MOVE_AMBIGUOUS;
        return ERROR;
    }
    
    return OK;
}


void CChessGame :: updateSANString(MOVELIST *mList, BOOL moveBack)
{
    BOARD_POSITION init = mList->init;
    BOARD_POSITION dest = mList->dest;
    BOARD_POSITION checkPos = init; 
    CDLList *cmenList = &chessmenList[gState.turn];
    STATUS status = mList->status;
    char *sanString = mList->SAN;
    
    CHESSPIECE piece, dupPiece;
    CHESSPIECE_GENERAL type;
    CHESSMENLIST *node;
    enum AMBIGUITY_TYPE {AT_NONE, AT_PRESENT, AT_FILE, AT_RANK, AT_BOTH} ambiguityType;

    char dstString[3] = "";
    char disAmbig[3] = "";
    char operation[2] = "";
    char newPiece[3] = "=";
    char flags[3] = "";
    char promoPiece = ' ';
	
    if(status & MOVE_CASTLING)
    {
        if(dest.x > init.x)
        {
            strcpy(sanString, "O-O");
        }
    	else
	    {
	        strcpy(sanString, "O-O-O");
	    }
    }

    else
    {
        if(moveBack)
        {
            /* Temporarily playback - to be put back; moveList needn't be changed */
            piece = getPiece(dest);
            clearPiece(dest);
            insertPiece(init, piece);
            checkPos = dest;
        }

        piece = getPiece(init);
        type = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(piece));
        
        if(type == PAWN || status & MOVE_PAWN_PROMOTED)
        {
	        sanString[0] =(status & MOVE_CAPTURE_PIECE) ? 'a' + init.x : '\0';
        }
	    else
	        sanString[0] =  toupper(toString(piece)[0]);

	    sanString[1] = '\0';
	
       dstString[0] = 'a' + dest.x;
       dstString[1] = '1' + dest.y;
       dstString[2] = '\0';

	   operation[0] = (status & MOVE_CAPTURE_PIECE) ? 'x' : '\0';
	   operation[1] = '\0';

       node = (CHESSMENLIST *)cmenList->getHead();
       ambiguityType = AT_NONE;

       while(node)
       {
           /* No ambiguity possible for KING/PAWN */
           if(type == PAWN || type == KING)
               break;
            
           dupPiece = getPiece(node->data);

           if(dupPiece == piece)
           {
               if(node->data.x != checkPos.x || node->data.y != checkPos.y) 
               {
                   /* Only proceed if this is not the piece we are moving */
                   if(validateMove(node->data, dest, MVT_GENERAL) & MOVE_SUCCESS)
                   {
                       /* Ambiguous move!! */
                       if(ambiguityType == AT_NONE)
                          ambiguityType = AT_PRESENT;
                       
                       if(node->data.x == init.x)
                       {
                           if(ambiguityType == AT_PRESENT)
                               ambiguityType = AT_RANK;
                           else if(ambiguityType == AT_FILE)
                               ambiguityType = AT_BOTH;
                       }
                    
                       if(node->data.y == init.y)
                       {
                           if(ambiguityType == AT_PRESENT)
                               ambiguityType = AT_FILE;
                           else if(ambiguityType == AT_RANK)
                               ambiguityType = AT_BOTH;                    
                       }
                   } 
               }
           }
           node = (CHESSMENLIST *)cmenList->getNext((DLL_NODE*)node);
       }
    
      if(moveBack)
      {
          /* Add back the modified data */
          clearPiece(init);
          insertPiece(dest, piece);
     }
   
    if(ambiguityType == AT_PRESENT)
        ambiguityType = AT_FILE;
        
    if(ambiguityType == AT_BOTH || ambiguityType == AT_FILE)
    {
        disAmbig[0] = 'a' + init.x;
        disAmbig[1] = '\0';
    }

    if(ambiguityType == AT_BOTH)
    {
        disAmbig[1] = '1' + init.y;
        disAmbig[2] = '\0';
    }

    if(ambiguityType == AT_RANK)
    {
        disAmbig[0] = '1' + init.y;
        disAmbig[1] = '\0';
    }    
    strcat(sanString, disAmbig);
	strcat(sanString, operation);
	strcat(sanString, dstString);
    
    if(status & MOVE_EN_PASSENT)
    {
        strcat(sanString, "(ep)");
    }
   
    if(status & MOVE_PAWN_PROMOTED)
    {
        promoPiece = toString(CHESSPIECE(status >> 24))[0];
        
        if(promoPiece != ' ')
            newPiece[1] = toupper(promoPiece);
        else 
            newPiece[1] = '\0';
            
        newPiece[2] = '\0';
        
        strcat(sanString, newPiece);
    }
    }

    flags[0] = (status & MOVE_CHECKMATE ) ? '#' : (status & MOVE_CHECK) ?  '+' : '\0';
    flags[1] = '\0';
    flags[2] = '\0';

    strcat(sanString, flags);

    return;
}

void CChessGame :: updateCANString(MOVELIST *mList)
{
    BOARD_POSITION init = mList->init;
    BOARD_POSITION dest = mList->dest;
    UINT8 index = 0;
    
    mList->CAN[index++] = 'a' + init.x;
    mList->CAN[index++] = '1' + init.y;
    mList->CAN[index++] = 'a' + dest.x;
    mList->CAN[index++] = '1' + dest.y;
    mList->CAN[index] = '\0';
    
    if(mList->status & MOVE_PAWN_PROMOTED)
    {
        //mList->CAN[index++] = '=';
        mList->CAN[index++] = tolower(toString(CHESSPIECE(mList->status >> 24))[0]);
        mList->CAN[index++] = '\0';
    }
    
    return; 
}

void CChessGame :: generateChessmenList()
{
    CHESSMENLIST *cList; 
    UINT8 i, j;

    for(i = 0; i < MAXY; i++)
    {
        for(j = 0; j < MAXX; j++)
        {
            if(game.board[i][j] == INVALID_CHESSPIECE) 
                continue;

            cList = newChessmenListEntry();

            cList->data.x = j;
            cList->data.y = i;

            chessmenList[CHESSPIECE_COLOR(game.board[i][j])].insertAtTail((DLL_NODE *)cList);
        }
    }

    return;
}

void CChessGame :: updateChessmenList()
{
    //recreate the chessmenlist. But the linked list is already available
    CHESSMENLIST *cList[2]; 
    UINT8 i, j, side;

    cList[WHITE] = (CHESSMENLIST *)chessmenList[WHITE].getHead();
    cList[BLACK] = (CHESSMENLIST *)chessmenList[BLACK].getHead();
    
    for(i = 0; i < MAXY; i++)
    {
        for(j = 0; j < MAXX; j++)
        {
            if(game.board[i][j] == INVALID_CHESSPIECE) 
                continue;

            side = CHESSPIECE_COLOR(game.board[i][j]);
            
            cList[side]->data.x = j;
            cList[side]->data.y = i;
            
            cList[side] = (CHESSMENLIST *)chessmenList[side].getNext((DLL_NODE *)cList[side]);
        }
    }
    
    return;
}

void CChessGame :: duplicateMoveList(CDLList mList)
{
    DLL_NODE *node;
    MOVELIST *mvData;
    BOARD_POSITION *pInit, *pDest;

    node = mList.getHead();
	
    while(node)
    {
        pInit = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));
        pDest = pInit + 1;

        mvData = new MOVELIST;

        mvData->init = *pInit;
        mvData->dest = *pDest;

        moveList.insertAtTail((DLL_NODE *)mvData);

        node = mList.getNext(node);
   }
}

BOARD_STATUS CChessGame :: getBoardStatus()
{
    return gState.boardStatus;
}

void CChessGame :: invertBoard()
{
     gState.boardStatus = gState.boardStatus == BS_NORMAL ? BS_INVERTED : BS_NORMAL;
}

MOVE_TURN CChessGame :: getCurrentTurn()
{
    return gState.turn;
}

MOVE_TURN CChessGame :: getPreviousTurn()
{
    return GET_OTHER_TURN(gState.turn);
}

void CChessGame :: setNextTurn(MOVE_TURN turn)
{
     gState.turn = turn;
}


PLAYER_TYPE CChessGame :: getCurrentPlayerType()
{
    return playerType[gState.turn];
}

PLAYER_TYPE CChessGame :: getPreviousPlayerType()
{
    return playerType[GET_OTHER_TURN(gState.turn)];
}

PLAYER_TYPE CChessGame :: getPlayerType(MOVE_TURN turn)
{
    return playerType[turn];
}

void CChessGame :: updatePlayerTypes(PLAYER_TYPE white, PLAYER_TYPE black)
{
    /* Used for implementing the white, black, force, go commands of xboard */
    playerType[MT_WHITE] = white;
    playerType[MT_BLACK] = black;  
}

GAME_PROGRESS CChessGame :: getGameProgress()
{
    return gState.gameProgress;
}

void CChessGame :: updateGameProgress(GAME_PROGRESS progress)
{
    gState.gameProgress = progress;
}

BOARD_POSITION CChessGame :: getKingPos(GAME_SIDE side)
{
   BOARD_POSITION kPos, *data;
   DLL_NODE *node;
   CHESSPIECE king = CHESSPIECE(MAKE_CHESSMEN(side, KING));

   node = chessmenList[side].getHead();
   while(node)
   {
       data = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));

       if(getPiece(BOARD_POSITION(data->y, data->x)) == king)
           return *data;

       node = chessmenList[side].getNext(node);
   }
   
   //should not happen
   return BOARD_POSITION(0, 0);
}

BOOL CChessGame :: insufficientMatingMaterial(char *fenString)
{
    char *temp = fenString;
    UINT8 rank = 7, file = 0;
    UINT8 bNCount = 0, wNCount = 0, bbBCount = 0, bwBCount = 0, wbBCount = 0, wwBCount = 0;
    
    while(*temp != '\0' && *temp != ' ')
    {
        if(*temp == '/')
        {
            rank--;
            file = 0;
            temp++;
            continue;
        }
        
        else if(*temp >= '0' && *temp <= '8')
        {
            file += (*temp - '0');
        }
        
        else if(*temp == 'K' || *temp == 'k')
        {
            temp++;
            continue;
        }
 
        else if(*temp == 'Q' || *temp == 'q' || *temp == 'R' || *temp == 'r' ||
           *temp == 'p' || *temp == 'P')
        {
            return FALSE;
        }
        
        else if(*temp == 'n')
        {
            bNCount++;
        }
        
        else if(*temp == 'N')
        {
            wNCount++;
        }
        
        else if(*temp == 'b')
        {
            if((rank + file) % 2 == 0)
            {
                bbBCount++;
            } 
            else
            {
                bwBCount++;
            }
        }

        else if(*temp == 'B')
        {
            if((rank + file) % 2 == 0)
            {
                wbBCount++;
            } 
            else
            {
                wwBCount++;
            }
        }
            
        if((bNCount + bbBCount + bwBCount) > 1)
        {
            return FALSE;
        }
        
        if((wNCount + wbBCount + wwBCount) > 1)
        {
            return FALSE;
        }
        //KNKB is not insufficient
        if((bNCount + wbBCount + wwBCount) > 1)
        {
            return FALSE;
        }
        
        if((wNCount + bbBCount + bwBCount) > 1)
        {
            return FALSE;
        }
        
        //KNKN is not insufficient
        if((wNCount + bNCount) > 1)
        {
            return FALSE;
        }
        
        if((wbBCount + bwBCount) > 1)
        {
            //bishops on opposite color
            return FALSE;
        }
        
        if((wwBCount + bbBCount) > 1)
        {
            //bishops on opposite color
            return FALSE;
        }
        
        temp++;
    }
    
    return TRUE;
}
