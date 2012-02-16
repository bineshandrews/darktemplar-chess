//moveEvaluations
#include "../include/chess.h"

//Piece square tables
typedef INT32 (*PTRCHBRD)[MAXY][MAXX];

//pawn
INT32 pawnPieceSquare_b[MAXY][MAXX] = 
{
{ 0,  0,  0,  0,  0,  0,  0,  0},
{50, 50, 50, 50, 50, 50, 50, 50},
{10, 10, 20, 30, 30, 20, 10, 10},
{ 5,  5, 10, 25, 25, 10,  5,  5},
{ 0,  0,  0, 20, 20,  0,  0,  0},
{ 5, -5,-10,  0,  0,-10, -5,  5},
{ 5, 10, 10,-20,-20, 10, 10,  5},
{ 0,  0,  0,  0,  0,  0,  0,  0}
};

INT32 pawnPieceSquare_w[MAXY][MAXX] = 
{
{ 0,  0,  0,  0,  0,  0,  0,  0},
{ 5, 10, 10,-20,-20, 10, 10,  5},
{ 5, -5,-10,  0,  0,-10, -5,  5},
{ 0,  0,  0, 20, 20,  0,  0,  0},
{ 5,  5, 10, 25, 25, 10,  5,  5},
{10, 10, 20, 30, 30, 20, 10, 10},
{50, 50, 50, 50, 50, 50, 50, 50}, 
{ 0,  0,  0,  0,  0,  0,  0,  0}
};

 
INT32 knightPieceSquare_b[MAXY][MAXX] = 
{
{-50,-40,-30,-30,-30,-30,-40,-50},
{-40,-20,  0,  0,  0,  0,-20,-40},
{-30,  0, 10, 15, 15, 10,  0,-30},
{-30,  5, 15, 20, 20, 15,  5,-30},
{-30,  0, 15, 20, 20, 15,  0,-30},
{-30,  5, 10, 15, 15, 10,  5,-30},
{-40,-20,  0,  5,  5,  0,-20,-40},
{-50,-40,-30,-30,-30,-30,-40,-50}
};

INT32 knightPieceSquare_w[MAXY][MAXX] = 
{
{-50,-40,-30,-30,-30,-30,-40,-50},
{-40,-20,  0,  5,  5,  0,-20,-40},
{-30,  5, 10, 15, 15, 10,  5,-30},
{-30,  0, 15, 20, 20, 15,  0,-30},
{-30,  5, 15, 20, 20, 15,  5,-30},
{-30,  0, 10, 15, 15, 10,  0,-30},
{-40,-20,  0,  0,  0,  0,-20,-40},
{-50,-40,-30,-30,-30,-30,-40,-50}
};

INT32 bishopPieceSquare_b[MAXY][MAXX] = 
{
{-20,-10,-10,-10,-10,-10,-10,-20},
{-10,  0,  0,  0,  0,  0,  0,-10},
{-10,  0,  5, 10, 10,  5,  0,-10},
{-10,  5,  5, 10, 10,  5,  5,-10},
{-10,  0, 10, 10, 10, 10,  0,-10},
{-10, 10, 10, 10, 10, 10, 10,-10},
{-10,  5,  0,  0,  0,  0,  5,-10},
{-20,-10,-10,-10,-10,-10,-10,-20}
};

INT32 bishopPieceSquare_w[MAXY][MAXX] = 
{
{-20,-10,-10,-10,-10,-10,-10,-20},
{-10,  5,  0,  0,  0,  0,  5,-10},
{-10, 10, 10, 10, 10, 10, 10,-10},
{-10,  0, 10, 10, 10, 10,  0,-10},
{-10,  5,  5, 10, 10,  5,  5,-10},
{-10,  0,  5, 10, 10,  5,  0,-10},
{-10,  0,  0,  0,  0,  0,  0,-10},
{-20,-10,-10,-10,-10,-10,-10,-20}
};

INT32 castlePieceSquare_b[MAXY][MAXX] =
{
{  0,  0,  0,  0,  0,  0,  0,  0},
{  5, 10, 10, 10, 10, 10, 10,  5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{  0,  0,  0,  5,  5,  0,  0,  0}      
};

INT32 castlePieceSquare_w[MAXY][MAXX] =
{
{  0,  0,  0,  5,  5,  0,  0,  0},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{ -5,  0,  0,  0,  0,  0,  0, -5},
{  5, 10, 10, 10, 10, 10, 10,  5},             
{  0,  0,  0,  0,  0,  0,  0,  0}
};


INT32 queenPieceSquare_b[MAXY][MAXX] = 
{
{-20,-10,-10, -5, -5,-10,-10,-20},
{-10,  0,  0,  0,  0,  0,  0,-10},
{-10,  0,  5,  5,  5,  5,  0,-10},
{ -5,  0,  5,  5,  5,  5,  0, -5},
{  0,  0,  5,  5,  5,  5,  0, -5},
{-10,  5,  5,  5,  5,  5,  0,-10},
{-10,  0,  5,  0,  0,  0,  0,-10},
{-20,-10,-10, -5, -5,-10,-10,-20}
}; 

INT32 queenPieceSquare_w[MAXY][MAXX] = 
{
{-20,-10,-10, -5, -5,-10,-10,-20},
{-10,  0,  5,  0,  0,  0,  0,-10},
{-10,  5,  5,  5,  5,  5,  0,-10},
{  0,  0,  5,  5,  5,  5,  0, -5},
{ -5,  0,  5,  5,  5,  5,  0, -5},
{-10,  0,  5,  5,  5,  5,  0,-10},
{-10,  0,  0,  0,  0,  0,  0,-10},
{-20,-10,-10, -5, -5,-10,-10,-20}
}; 


//TODO: Have a different one for king in endgames

INT32 kingPieceSquare_b[MAXY][MAXX] = 
{
{-30,-40,-40,-50,-50,-40,-40,-30},
{-30,-40,-40,-50,-50,-40,-40,-30},
{-30,-40,-40,-50,-50,-40,-40,-30},
{-30,-40,-40,-50,-50,-40,-40,-30},
{-20,-30,-30,-40,-40,-30,-30,-20},
{-10,-20,-20,-20,-20,-20,-20,-10},
{ 20, 20,  0,  0,  0,  0, 20, 20},
{ 20, 30, 10,  0,  0, 10, 30, 20}
};

INT32 kingPieceSquare_w[MAXY][MAXX] = 
{
{ 20, 30, 10,  0,  0, 10, 30, 20},
{ 20, 20,  0,  0,  0,  0, 20, 20},
{-10,-20,-20,-20,-20,-20,-20,-10},
{-20,-30,-30,-40,-40,-30,-30,-20},
{-30,-40,-40,-50,-50,-40,-40,-30},
{-30,-40,-40,-50,-50,-40,-40,-30},
{-30,-40,-40,-50,-50,-40,-40,-30},  
{-30,-40,-40,-50,-50,-40,-40,-30}
};

INT32 invalidPieceSquare[MAXY][MAXX] = 
{
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0},
{0,  0,  0,  0,  0,  0,  0,  0}
}; 

INT32 pieceValues[2][256];
PTRCHBRD pieceValueSquares[256];
// #defines for special move situations comes here..

void CChessGame :: initPieceValues(void)
{
    //pieceValues[0] - white's point of view
    //pieceValues[1] = black's point of view

    pieceValues[0][W_PAWN] = pieceValues[1][B_PAWN] = 100;
    pieceValues[1][W_PAWN] = pieceValues[0][B_PAWN] = -100;
    
    pieceValues[0][W_KNIGHT] = pieceValues[1][B_KNIGHT] = 320;
    pieceValues[1][W_KNIGHT] = pieceValues[0][B_KNIGHT] = -320;

    pieceValues[0][W_BISHOP] = pieceValues[1][B_BISHOP] = 330;
    pieceValues[1][W_BISHOP] = pieceValues[0][B_BISHOP] = -330;

    pieceValues[0][W_CASTLE] = pieceValues[1][B_CASTLE] = 520;
    pieceValues[1][W_CASTLE] = pieceValues[0][B_CASTLE] = -520;

    pieceValues[0][W_QUEEN] = pieceValues[1][B_QUEEN] = 980;
    pieceValues[1][W_QUEEN] = pieceValues[0][B_QUEEN] = -980;

    pieceValues[0][W_KING] = pieceValues[1][B_KING] = 20000;
    pieceValues[1][W_KING] = pieceValues[0][B_KING] = -20000;
    
    pieceValues[0][INVALID_CHESSPIECE] = pieceValues[1][INVALID_CHESSPIECE] = 0;
    
    pieceValueSquares[INVALID_CHESSPIECE] = &invalidPieceSquare;
    pieceValueSquares[W_PAWN] = &pawnPieceSquare_w;
    pieceValueSquares[B_PAWN] = &pawnPieceSquare_b;
    pieceValueSquares[W_KNIGHT] = &knightPieceSquare_w;    
    pieceValueSquares[B_KNIGHT] = &knightPieceSquare_b;    
    pieceValueSquares[W_BISHOP] = &bishopPieceSquare_w;            
    pieceValueSquares[B_BISHOP] = &bishopPieceSquare_b; 
    pieceValueSquares[W_CASTLE] = &castlePieceSquare_w;            
    pieceValueSquares[B_CASTLE] = &castlePieceSquare_b; 
    pieceValueSquares[W_QUEEN] = &queenPieceSquare_w;            
    pieceValueSquares[B_QUEEN] = &queenPieceSquare_b; 
    pieceValueSquares[W_KING] = &kingPieceSquare_w;            
    pieceValueSquares[B_KING] = &kingPieceSquare_b; 
}

INT32 CChessGame :: evaluateStaticBoardPosition(UINT8 side)
{
    //crude - just check for piece values
    INT32 totalVal = 0;
    UINT8 i, j;
    
    for(i = 0; i < MAXY; i++)
    {
        for(j = 0; j < MAXX; j++)
        {
            if(game.board[i][j] == INVALID_CHESSPIECE)
                continue;
                
            totalVal += pieceValues[side][game.board[i][j]]; 
            
            if(CHESSPIECE_COLOR(game.board[i][j]) == side)
            {
                totalVal += (*pieceValueSquares[game.board[i][j]])[i][j];
            }
            else
            {
                totalVal -= (*pieceValueSquares[game.board[i][j]])[i][j];                
            }                
        }
    }
    
    totalVal += evaluatePawnStructure(side);
    
    return totalVal;
}

INT32 CChessGame :: evaluatePawnStructure(UINT8 side)
{
    //incentive for pawns supported by other pawns
    //penality for doubled/tripled pawns
    
    UINT8 bCount, wCount, i, j;
    INT8 score = 0, sign = 0;
    INT32 pawnPenality[] = {0, -20, -30, -60, -100}; // incremental penality for doubled
    INT32 pawnSupportIncentive = 10;
    
    sign = -2*side + 1; // white =1, black = -1
    
    for(i = 0; i < MAXX; i++)
    {
        bCount = 0;
        wCount = 0;
        
        for(j = 0; j < MAXY; j++)
        {
            if(game.board[j][i] == W_PAWN)
            {
                wCount++;
                
                if(j < 6)
                {
                    //penalise pawns in backward squares only
                    score += (pawnPenality[wCount]);
                }
                
                if( j <= 1) continue;
                
                if(i > 0 && game.board[j-1][i-1] == W_PAWN)
                {
                    score += pawnSupportIncentive;
                }
                if(i < 7 && game.board[j-1][i+1] == W_PAWN)
                {
                    score += pawnSupportIncentive;
                }
            }
            if(game.board[j][i] == B_PAWN)
            {
                bCount++;
                
                if(j > 1)
                {
                    //penalise pawns in backward squares only.
                    score -= (pawnPenality[bCount]);
                }

                if( j >= 6) continue;
                
                if(i > 0 && game.board[j+1][i-1] == B_PAWN)
                {
                    score -= pawnSupportIncentive;
                }
                if(i < 7 && game.board[j+1][i+1] == B_PAWN)
                {
                    score -= pawnSupportIncentive;
                }
            } 
        } 
    }
    
    return (sign * score);
}
