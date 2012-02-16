#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include "../../include/chess.h"
#include "../../libh/zobrist.h"

using namespace std;

static int readLineFromConsole(char *line, int size, FILE *fp)
{
        int c;
        int pos = 0;

        for (;;) {
                errno = 0;
                c = fgetc(fp);
                if (c == EOF) {
                        if (!errno) return -1;
                        printf("error: %s\n", strerror(errno));
                        errno = 0;
                        continue;
                }
                if (c == '\n') {
                        break;
                }
                if (pos < size-1) {
                        line[pos++] = c;
                }
        }
        line[pos] = 0;
        return pos;
}

CHESSPIECE_GENERAL getNewPieceFromUser()
{
    CHESSPIECE_GENERAL list[]={QUEEN, CASTLE, KNIGHT, BISHOP};
    int choice = 1, ret;

    do
    {
        printf("\nSelect your Piece : 1. Queen  2. Castle  3. Knight  "
		       "4. Bishop :: \nEnter ur choice (1-4) :: ");

        ret = scanf("%d\n", &choice);
	if(ret <= 0) break;

    } while(choice < 1 || choice > 4);

    return list[choice -1];
}

void CChessGame :: displayBoard()
{
    int i, j;
    CHESSPIECE cp;
    BOARD_POSITION bp;
    INT8 charp[2];

    cout<<"\n";
    for(j=0;j<=6*MAXX;j++)
     {
          if(j %  6 == 0)
              cout<<"|";
          else
             cout<<"-";
     }

    for(i=0; i<MAXY; i++)
    {
        cout<<"\n|";
        for(j=0; j<MAXX; j++)
        {
            bp = convertPosition2Absolute(BOARD_POSITION(MAXY-1-i, j));
            cp = getPiece(bp);
            strcpy(charp, (cp != INVALID_CHESSPIECE) ? toString(cp) : (((bp.x + bp.y) % 2 == 0) ? "." : " "));
            cout<<"  "<<charp<<"  |";
        }

        cout<<"\n";
        for(j=0;j<=6*MAXX;j++)
        {
            if(j %  6 == 0)
                cout<<"|";
             else
                cout<<"-";
        }
    }
    cout<<"\n";
}

void CChessGame :: printMoveList()
{
/*
    BOARD_POSITION *init, *dest;
    CHESSPIECE *cPiece;
    enum MOVE_STATUS *status;
*/    
    DLL_NODE *node;
    INT16 count = 0;
    INT16 numbering = 1;
//  INT8 moveXDir;
    char numberstring[6];

    printf("\n Move List:\n =========\n\n");    
    node = moveList.getHead();
    while(node)
    {
           
/*      init = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));
        dest = init + 1;
        status = (enum MOVE_STATUS *)(dest + 1);

	if(*status & MOVE_CASTLING)
	{
	    moveXDir = (dest->x > init->x) ? 1 : -1;
	    printf("\n%s%s%s %s", count % 2 ? "               " : "", "O - O", 
	           moveXDir > 0 ? "" : " - O",
		   *status & MOVE_CHECKMATE ? "++" : (*status & MOVE_CHECK ? "+" : ""));
	}
	else
            printf("\n%s%c%d %c %c%d %s",count % 2 ? "               " : "", init->x + 'a', init->y + 1, 
	           *status & MOVE_CAPTURE_PIECE ? 'x' : '-' , dest->x + 'a', dest->y + 1, 
	           *status & MOVE_CHECKMATE ? "++" : (*status & MOVE_CHECK ? "+" : ""));
*/
       if(count && count % 2 == 0)
           printf("\n");

       sprintf(numberstring, "%d.", numbering);
       printf("%s%s ", (count % 2 == 0) ? numberstring : "", ((MOVELIST *)node)->SAN);
           
       node = moveList.getNext(node);
        count++;

	if(count % 2 == 0) numbering++;
    }

    printf("\n");
#if 0
    printf("\n\n Chessmen List:\n =============\n");
    node = chessmenList[WHITE].getHead();
    while(node)
    {
        init = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));
        printf("\n%s", toString(getPiece(*init)));
        node = chessmenList[WHITE].getNext(node);
    }
    printf("\n");

    node = chessmenList[BLACK].getHead();
    while(node)
    {
        init = (BOARD_POSITION *)((INT8*)node + sizeof(DLL_NODE));
        printf("\n%s", toString(getPiece(*init)));
        node = chessmenList[BLACK].getNext(node);
    }
    printf("\n\n Captured Pieces List:\n =====================\n");
    node = capturedPiecesList[WHITE].getHead();
    while(node)
    {
        cPiece = (CHESSPIECE *)((INT8*)node + sizeof(DLL_NODE));
        printf("\n%s", toString(*cPiece));
        node = chessmenList[WHITE].getNext(node);
    }
    printf("\n");

    node = capturedPiecesList[BLACK].getHead();
    while(node)
    {
        cPiece = (CHESSPIECE *)((INT8*)node + sizeof(DLL_NODE));
        printf("\n%s", toString(*cPiece));
        node = chessmenList[BLACK].getNext(node);
    }
    printf("\n");
#endif
    printf("\n\n");
}


int main(int argc, char *argv[])
{
    CChessGame game(DEFAULT_FEN_STRING, PT_HUMAN, PT_COMPUTER);
    CHESSPIECE_GENERAL newPiece;
    MOVELIST userMove;
    int x1, y1, x2, y2;
    STATUS status = ERROR;
    int xboard_mode = 0;
    UINT32 xboard_protover=1;
    char command[128], line[128];
//    char numberString[50];
    int playMode = 1, moveCount = 0;
    char openingBookPath[256] = "", configPath[256] = "", configLine[100] = "";
    int i;
    ifstream configFile;
    bool found;
    
    strcpy(openingBookPath, argv[0]);

    for(i = strlen(openingBookPath)-1; i >= 0; i--)
    {
        if(openingBookPath[i] == '\\' || openingBookPath[i] == '/')
        {
            break;
        }
    }

    openingBookPath[i+1] = '\0';
    
    strncpy(configPath, openingBookPath, 200);
    strcat(configPath, "../../");
    strcat(configPath, "config.txt");
    
    configFile.open(configPath);
    
    if (configFile.is_open())
    {
        while (! configFile.eof() )
        {
            configFile.getline(configLine, 100);
            if(strncmp(configLine, "OpeningBook", 11) == 0)
            {
                found = true;
                break;
            }
        }
    
        configFile.close();
    }

    if(found == true)
    {
        char *start = configLine + strlen("OpeningBook");
        
        //trim leading spaces
        for(i = (int)strlen("OpeningBook"); i < (int)strlen(configLine); i++)
        {
            if(isalpha(configLine[i])) break;
            
            start = configLine + i + 1; //next char
        }
        
        if(i < (int)strlen(configLine))
        {
            strcat(openingBookPath, "../../openingbook/");
            strcat(openingBookPath, start);     
            game.setOpeningBook(openingBookPath);
        }
    }
    
    
    //game.setOpeningBook("D:\\Programs\\Chess\\openingbook\\performance.bin");
    //game.setOpeningBook("D:\\Programs\\Chess\\openingbook\\varied.bin");
    //game.setOpeningBook("D:\\Programs\\Chess\\openingbook\\Morphius201004.bin");
    //game.setOpeningBook("D:\\Programs\\Chess\\openingbook\\raj4.bin");
    //game.setOpeningBook("D:\\Programs\\Chess\\openingbook\\TDDBII.bin");
    
    game.displayBoard();

    while (1)
    {
        newPiece = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(INVALID_CHESSPIECE));
        
        if (!xboard_mode) 
        {
            fputs("\nMOVE# ", stdout);
        }
       
        if(game.getCurrentPlayerType() == PT_HUMAN || 
           game.getGameProgress() != GP_IN_PROGRESS)
        {
            fflush(stdout);

            if (readLineFromConsole(line, sizeof(line), stdin) < 0) 
            {
                //fputs("Read Error\n", stdout);
                fflush(stdout);
                break;
            }
        
            if (1 != sscanf(line, "%s", command)) continue;
       
            if(strncmp(command, "quit", 4) == 0 ||
               strncmp(command, "exit", 4) == 0 )
            {
                break;
            }
            else if(strncmp(command, "xboard", 6) == 0)
            {
                printf("Switching to xboard mode... \n");
                xboard_mode = 1;
                xboard_protover = 1;
                continue;
            }
            else if(strncmp(command, "protover", 7) == 0)
            {
                char *ptr = line;
                while(*ptr != ' ')ptr++;
                 
                while(*ptr == ' ')ptr++;

                if(!xboard_mode)
                {
                    continue;
                }
                
                sscanf(ptr, "%lu", &xboard_protover);
                
                if(xboard_protover == 2)
                {
                    //request custom position feature..
                    printf("feature setboard=1 sigint=0 sigterm=0\n");
                }
            }
            else if(strncmp(command, "accepted", 8) == 0 ||
                    strncmp(command, "rejected", 8) == 0)
            {
                continue; //ignore for now
            }
            else if(strncmp(command, "invert", 6) == 0)
            {
                game.invertBoard();
                game.displayBoard();
                continue;
            }
            else if(strncmp(command, "displayboard", 12) == 0)
            {
                game.displayBoard();
                continue;
            }
            else if(strncmp(command, "displayFEN", 10) == 0)
            {
                printf("%s\n", game.getLastFENString());
                continue;
            }
            else if(strncmp(command, "printmoves", 10) == 0)
            {
                game.printMoveList();
                continue;            
            }
            else if(strncmp(command, "new", 3) == 0)
            {
                 //implement this!!
                 game.cleanupLists();
                 game.setInitFEN(DEFAULT_FEN_STRING);
                 game.initGame(game.getInitFEN(), TRUE, PT_HUMAN, PT_COMPUTER);
                 
                 if(!xboard_mode)
                 {
                    game.displayBoard();
                    printf("%s\n", game.getLastFENString());
                 }                 
                 
                 continue;
            }
            else if(strncmp(command, "result", 6) == 0)
            {
                 char *ptr = line;
                 while(*ptr != ' ')ptr++;
                 
                 while(*ptr == ' ')ptr++;
                 

                 if(ptr[0] == '*')
                 {
                     //ignore
                     continue;
                 }
                 else if(ptr[0] == '1' && ptr[1]!= '/')
                 {
                     //White wins
                 }
                 else if(ptr[0] == '0')
                 {
                     //Black wins
                 }
                 else if(ptr[1] == '/')
                 {
                      //Draw
                 }

                 game.updateGameProgress(GP_ENDED);   
                 continue;
            }
            else if(strncmp(command, "draw", 4) == 0)
            {
                 //opponent offers draw - for now, for test, accept every offer
                 if(game.handleOfferDraw() == OFFER_ACCEPTED)
                 {
                     if(xboard_mode) 
                     {
                        printf("offer draw\n");
                     }
                     else
                     {
                        printf("Game ended.. Draw offer accepted!!\n");    
                     }
                 }
                 else
                 {
                     //ignore - do nothing
                 }
                 
                 continue;
            }
            else if(strncmp(command, "undo", 4) == 0)
            {
                game.unmakeMove();
                 
                if(!xboard_mode)
                {
                   game.displayBoard();
                   printf("%s\n", game.getLastFENString());
                }                 
                
                continue;
            }
            else if(strncmp(command, "remove", 6) == 0)
            {
                 continue;
            }
            else if(strncmp(command, "white", 5) == 0)
            {
                 game.updatePlayerTypes(PT_HUMAN, PT_COMPUTER);
                 game.setNextTurn(MT_WHITE);
                 //stop clocks
                 continue;
            }
            else if(strncmp(command, "black", 5) == 0)
            {
                 game.updatePlayerTypes(PT_COMPUTER, PT_HUMAN);
                 game.setNextTurn(MT_BLACK);
                 //stop clocks
                 continue;
            }
            else if(strncmp(command, "force", 5) == 0)
            {
                 game.updatePlayerTypes(PT_HUMAN, PT_HUMAN);
                 continue;
            }
            else if(strncmp(command, "go", 2) == 0)
            {
                 if(game.getCurrentTurn() == MT_WHITE)
                 {
                     game.updatePlayerTypes(PT_COMPUTER, PT_HUMAN);
                 }
                 else
                 {
                     game.updatePlayerTypes(PT_HUMAN, PT_COMPUTER);
                 }
                 
                 continue;
            }
            else if(strncmp(command, "playother", 9) == 0)
            {
                 continue;
            }
            else if(strncmp(command, "fen", 3) == 0 ||
                    strncmp(command, "setb", 4) == 0 )  //setb[oard]
            {
                 char *ptr = line;
                 while(*ptr != ' ')ptr++; //strip the "fen" at the beginning
                 
                 while(*ptr == ' ')ptr++; //take out the extra spaces

                 game.cleanupLists();
                 game.setInitFEN(ptr);
                 game.initGame(game.getInitFEN(), TRUE, 
                               game.getPlayerType(MT_WHITE), 
                               game.getPlayerType(MT_BLACK));
                 
                 if(!xboard_mode)
                 {
                    game.displayBoard();
                    printf("%s\n", game.getLastFENString());
                 }                 

                 continue;
            }            
            else if(strncmp(command, "time", 4) == 0 ||
                    strncmp(command, "otim", 4) == 0 ) 
            {
                // We don't support this yet
                continue;
            }
            else
            {
                //detect input format
                if(isalpha(command[0]) && isdigit(command[1]) && 
                   isalpha(command[2]) && isdigit(command[3]) && 
                   islower(command[0]) && islower(command[2]))
                    playMode = 0;
                else 
                    playMode = 1;
            } 

            userMove.status = MOVE_NONE; 
            newPiece = CHESSPIECE_GENERAL(CHESSPIECE_TYPE(INVALID_CHESSPIECE));
        
            if(playMode == 0)
            {
                status = game.decodeMoveString(command, &userMove);
            }
            else if(playMode == 1)
            {
                status = game.decodeSANString(command, &userMove);
            }
        }
        
        else 
        {
            //testing.. overwrite
            char *fen;
            CZobrist zob;
            BOOK_MOVE bookMove;
            BOOL rc;
            STATUS moveStatus;
            CHESSPIECE newPiece;
            //char numberString[30];
                           
            fen = game.getLastFENString();
            //printf("FEN# %s\n", fen);
            zob.updateBoardPosition(fen);
            zob.calculateZobristHash();
            //printf("Key = %s\n", 
              //     print64_hex(zob.getZobristHash(), numberString));

            rc = game.getBestMoveFromBook(zob.getZobristHash(), &bookMove);
            
            if(rc == TRUE)
            {
                //printf("Move found from BOOK.. %c%c%c%c", bookMove.init.x + 'a',
                       //bookMove.init.y + '1', bookMove.dest.x + 'a',
                       //bookMove.dest.y + '1');

                moveStatus = game.validateMove(bookMove.init, 
                                          bookMove.dest, MT_VALIDATE);
                                          
                if(moveStatus == MOVE_ILLEGAL)
                {
                    //printf("Illegal move from book.. using engine..\n");
                    status = game.getRandomMove(&userMove);
                }
                else
                {
                    newPiece = INVALID_CHESSPIECE;
                    
                    if(moveStatus & MOVE_PAWN_PROMOTED)
                    {
                        newPiece = CHESSPIECE(MAKE_CHESSMEN(game.getCurrentTurn(),
                                                            bookMove.newPiece));
                        moveStatus |= (newPiece << 24);
                    }
                    
                    userMove.init = bookMove.init;
                    userMove.dest = bookMove.dest;
                    userMove.status = moveStatus;
                    status = OK;
                }
            }
            else
            {
                 //printf("No move found.. key = %s\n", 
                   //    print64_hex(zob.getZobristHash(), numberString));
                       
//                status = game.getRandomMove(&userMove);
                status = game.getBestMove(&userMove);
            }
            

        }

        if(!xboard_mode)
        {
            if(game.getCurrentPlayerType() == PT_HUMAN)
            {            
                printf("%s\n", command);
            }
            else
            {
                game.updateSANString(&userMove, FALSE);
                printf("%s ", userMove.SAN);
            }
        }
        
        if(status == OK)
        {
            if(game.getGameProgress() != GP_IN_PROGRESS)
            {
                //printf("Game progress = %x\n", game.getGameProgress());
                continue;
            }
            
            if(userMove.status & MOVE_PAWN_PROMOTED)
                newPiece = CHESSPIECE_GENERAL(CHESSPIECE(userMove.status >>24));

            x1 = userMove.init.x;
            y1 = userMove.init.y;
            x2 = userMove.dest.x;
            y2 = userMove.dest.y;
            
            game.updateCANString(&userMove);

//            printf("Decoded move = %u %u  -  %u %u =  %u\n", x1,y1,x2,y2,newPiece);
        }
        else
        {
            if(!xboard_mode)
            {
                if(userMove.status & MOVE_AMBIGUOUS)
                    printf("Ambiguous move!!\n");
                else
                    printf("Cannot decode move string\n");
            }
            continue;
        }

        status = game.playMoveOnBoard(game.convertPosition2Absolute(BOARD_POSITION(y1,x1)),
                           game.convertPosition2Absolute(BOARD_POSITION(y2,x2)),
                           CHESSPIECE_GENERAL(newPiece), 
                           CLAIM_NONE);    
                           
        if(status & MOVE_SUCCESS) 
        {
            if(game.getPreviousPlayerType() == PT_COMPUTER)
            {
                if(xboard_mode)
                {
                    /* Send the move to xboard */
                    //testing drawing  - remove after testing
                    if(status & (MOVE_DRAW_50MOVE_ACTIVE | MOVE_DRAW_3MOVE_ACTIVE | MOVE_DRAW_INSUFFICIENT_PIECES_ACTIVE))
                    {
                        printf("offer draw\n");
                    }
                    
                    printf("move %s\n", userMove.CAN);
                }
            }

            if(status & MOVE_CHECKMATE)
            {
                //printf("\nCheckmate!!\n");
                if(game.getPreviousTurn() == MT_WHITE)
                {
                    if(xboard_mode)
                    {
                        printf("1-0 {White mates}\n");
                    }
                    else
                    {
                        printf("Checkmate!! White wins..\n");
                    }
                }
                else
                {
                    if(xboard_mode)
                    {
                        printf("0-1 {Black mates}\n");
                    }
                    else
                    {
                        printf("Checkmate!! Black wins..\n");
                    }
                }

                game.updateGameProgress(GP_ENDED);   
            }
            else if(status & MOVE_STALEMATE)
            {
                //printf("\nStalemate!!\n");
                if(xboard_mode)
                {
                    printf("1/2-1/2 {Stalemate}\n");
                }
                else
                {
                    printf("Stalemate!! Game drawn..\n");
                }

                game.updateGameProgress(GP_ENDED);   
            }
            else if(status & MOVE_CHECK)
            {
                 if(!xboard_mode)
                 {
                    printf("\nCheck!!\n");
                 }
            }
             

            if(!xboard_mode)
            {
                game.displayBoard();
                
                if(game.getPreviousPlayerType() == PT_COMPUTER)
                {
                    printf("%s\n", game.getLastFENString());
                }
            }

            //game.toggleTurn();
            moveCount++;
        }
        else
        {
            if(xboard_mode)
            {
                printf("\nIllegal move: ");
                printf("%s\n", userMove.CAN);
            }
            else
            {
                printf("Illegal move!!\n");
            }
            
            continue;
        }
    }
}
