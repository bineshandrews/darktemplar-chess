
#ifndef __DLLIST_H__
#define __DLLIST_H__

#include "../include/datatypes.h"

typedef struct dllNode
{
        struct dllNode *prev;
        struct dllNode *next;
        
        dllNode(struct dllNode *p = (struct dllNode *)NULL, struct dllNode *n = (struct dllNode*) NULL)
        {
            prev = p;
            next = n;
        }
} DLL_NODE;

typedef struct dllControlBlock
{
        DLL_NODE *first;
        DLL_NODE *last;
        //UINT32 nodeSize;
        UINT32 nodeCount;
        
        dllControlBlock(DLL_NODE *f = (DLL_NODE *)NULL, DLL_NODE *l = (DLL_NODE *)NULL)
        {
    	    nodeCount = 0;
            first = f;
            last  = l;
        }
} DLL_CB;

typedef STATUS (*COMPAREFN)(void *, void *);

class CDLList
{
    private:
      DLL_CB controlBlock;
      COMPAREFN cmpFn;
      
    public:
      CDLList(COMPAREFN);
      CDLList();
      void setCompare(COMPAREFN);
      STATUS insertAtHead(DLL_NODE *);
      STATUS insertAtTail(DLL_NODE *);
      STATUS insertAfter(DLL_NODE *, DLL_NODE *);
      STATUS insertBefore(DLL_NODE *, DLL_NODE *);
      DLL_NODE * removeTail();
      DLL_NODE * removeHead();
      DLL_NODE * removeNode(DLL_NODE *);
      DLL_NODE * findNode(DLL_NODE *);
      DLL_NODE * getHead();
      DLL_NODE * getTail(); 
      DLL_NODE * getNext(DLL_NODE *);
      DLL_NODE * getPrev(DLL_NODE *);

      UINT32 getListSize();
};

#endif
