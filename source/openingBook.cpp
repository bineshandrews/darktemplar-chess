#include "../include/chess.h"

#define MAX_BOOK_ENTRIES 100

typedef struct 
{
    UINT64 key;	
    UINT16 move;
    UINT16 weight;
    UINT32 learn;
} BOOK_ENTRY;


BOOK_ENTRY entry_none = {
    0, 0, 0, 0
};


BOOL readNumberFromFile(FILE *f, int length, UINT64 *number)
{
    UINT8 i;
    int c;
    
    *number = 0;
    
    for(i = 0; i < length; i++)
    {
        c=fgetc(f);
    
        if(c==EOF)
        {
            return FALSE;
        }
        
        (*number)=((*number)<<8) + c;
    }
    
    return TRUE;
}

BOOL readEntryFromFile(FILE *f, BOOK_ENTRY *entry)
{
    BOOL rc;
    UINT64 r;
    
    rc = readNumberFromFile(f, 8, &r);
    if(rc != TRUE) 
        return FALSE;
    
    entry->key=r;

    rc = readNumberFromFile(f, 2, &r);
    if(rc != TRUE ) 
        return FALSE;
        
    entry->move=r;
    
    rc= readNumberFromFile(f, 2, &r);
    if(rc != TRUE) 
        return FALSE;
        
    entry->weight=r;
    
    rc= readNumberFromFile(f, 4, &r);
    if(rc != TRUE) 
        return FALSE;
        
    entry->learn=r;
    
    return TRUE;
}

int findKeyInFile(FILE *f, UINT64 key, BOOK_ENTRY *entry)
{
    int first, last, middle;
    BOOK_ENTRY first_entry = entry_none, last_entry, middle_entry;
    
    first=-1;
    
    if(fseek(f, -16, SEEK_END))
    {
        *entry=entry_none;
        entry->key=key+1; //hack
        return -1;
    }
    
    last=ftell(f)/16;
    readEntryFromFile(f, &last_entry);
    
    while(1)
    {
        if((last-first) == 1)
        {
            *entry = last_entry;
            return last;
        }
        
        middle = (first+last)/2;
        fseek(f, 16*middle, SEEK_SET);
        readEntryFromFile(f, &middle_entry);
        
        if(key <= middle_entry.key)
        {
            last = middle;
            last_entry = middle_entry;
        }
        else
        {
            first = middle;
            first_entry = middle_entry;
        }
    }
    
}

void CChessGame :: setOpeningBook(char *fileName)
{
    strncpy(openingBookFile, fileName, 100);
}

BOOL CChessGame :: getBestMoveFromBook(UINT64 key, BOOK_MOVE *move)
{
    FILE *f;
    BOOK_ENTRY allEntries[MAX_BOOK_ENTRIES];
    UINT16 moves[MAX_BOOK_ENTRIES];
    BOOK_ENTRY entry;
    UINT16 selectedMove;
    int offset, totalWeight, maxWeight = 0;
    UINT16 count = 0, moveCount = 0;
    UINT32 randomIndex, i;
    BOOL rc;
    double averageWeight;
    CHESSPIECE_GENERAL newPieces[] = {
                       CHESSPIECE_GENERAL(CHESSPIECE_TYPE(INVALID_CHESSPIECE)), 
                       KNIGHT, BISHOP, CASTLE, QUEEN
                       };
    
    f = fopen(openingBookFile, "rb");
    
    if(!f)
    {
        printf("Failed to open book.. %s\n", openingBookFile);
        return FALSE;
    }
    
    offset = findKeyInFile(f, key, &entry);
    if(entry.key!=key)
    {
        //printf("No entry found in book..\n");
        fclose(f);
        return FALSE;
    }
    
    allEntries[0]=entry;
    count=1;
    fseek(f, 16*(offset+1), SEEK_SET);
    
    while(1)
    {
        rc=readEntryFromFile(f, &entry);
        if(rc != TRUE)
        {
            break;
        }
        
        if(entry.key != key)
        {
            break;
        }
        
        if(count == MAX_BOOK_ENTRIES)
        {
            //printf("Too many moves in this position (max=%d)\n", MAX_BOOK_ENTRIES);
            break; 
        }
        
        allEntries[count++]=entry;
    }
    
    fclose(f);
    
    for(i = 0; i < count; i++)
    {
        totalWeight+= allEntries[i].weight;
        
        if(allEntries[i].weight > maxWeight)
        {
            maxWeight = allEntries[i].weight;
        }
    }
    
    //averageWeight = (double)totalWeight/count;
    
    averageWeight = (maxWeight) / 2;
    
    for(i = 0; i < count; i++)
    {
        if((double)allEntries[i].weight > averageWeight)
        {
            moves[moveCount++] = allEntries[i].move;
        }
    }
    
    //Select the first move if weights are all equal
    if(moveCount == 0)
        moves[moveCount++] = allEntries[0].move;
    
    //Select from moves with weight >= average   
    randomIndex = getRandomNumber(0, moveCount-1);
    selectedMove = moves[randomIndex];
    
    move->dest.x = selectedMove & 0x7;
    move->dest.y = (selectedMove >> 3) & 0x7;
    move->init.x = (selectedMove >> 6) & 0x7;
    move->init.y = (selectedMove >> 9) & 0x7;
    
    move->newPiece = newPieces[(selectedMove >> 12) & 0xf];
    
    //printf("move.. %d, count = %d\n", allEntries[randomIndex].move, count);
    
    return TRUE;
}
