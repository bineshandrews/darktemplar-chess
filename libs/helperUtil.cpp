#include "../include/datatypes.h"

void ReverseString(char *str)
{
     UINT16 i, len = strlen(str);
     char temp;
     
     for(i = 0; i < len/2; i++)
     {
         temp = str[len-1-i];
         str[len-1-i] = str[i];
         str[i] = temp;
     }
}

char *print64(UINT64 num, char *str)
{
     //computationally expensive..
     UINT8 i = 0;
     strcpy(str, "test");
     if(num == 0)
         str[i++] = '0';
         
     while(num)
     {
         str[i++] = '0' + num%10;
         num/=10;
     }
     str[i] = '\0';
     
     ReverseString(str);
     
     return str;
}

char *print64_hex(UINT64 num, char *str)
{
     //computationally expensive
     UINT8 i = 0;
     char charset[]= "0123456789abcdef";
     
     if(num == 0)
         str[i++] = '0';
         
     while(num)
     {
         str[15-i++] = charset[num & 0x0f];
         num = num>>4;
     }
     
     while(i < 16)
     {
         str[15-i++] = '0';
     }
     
     str[16] = '\0';

     //ReverseString(str);
     
     return str;
}

UINT32 getRandomNumber(UINT32 start, UINT32 end)
{
    UINT32 min, max, range, randVal;
    
    min = start < end ? start : end;
    max = start >= end ? start : end;
    range = max - min + 1;
    
    if(range == 0) 
    {
        randVal = 0;
    }   
    else
    {
        //Seed the RNG
        srand ( time(NULL) );
        randVal = rand() % range;
    }
    
    return (min + randVal);
}
