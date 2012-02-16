#include "../libh/dllist.h"
#include <stddef.h>

CDLList :: CDLList(COMPAREFN fn )
{
//    controlBlock.nodeSize = nodeSize;
    setCompare(fn);
}

CDLList :: CDLList()
{
}

void CDLList :: setCompare(COMPAREFN fn)
{
    cmpFn = fn;
}

STATUS CDLList :: insertAtHead(DLL_NODE *dllNode)
{
    if(NULL == dllNode)
        return ERROR; 
        
    dllNode->next = controlBlock.first;
    dllNode->prev = (DLL_NODE*)NULL;
    
    if(NULL == controlBlock.first)
        controlBlock.last = dllNode; 
    else
        (controlBlock.first)->prev = dllNode;
        
    controlBlock.first = dllNode;
    controlBlock.nodeCount++;
    
    return OK;
}

STATUS CDLList :: insertAtTail(DLL_NODE *dllNode)
{
    if(NULL == dllNode)
        return ERROR;
         
    dllNode->next = (DLL_NODE*)NULL;
    dllNode->prev = controlBlock.last;
    
    if(NULL == controlBlock.last)
        controlBlock.first = dllNode;
    else
        (controlBlock.last)->next = dllNode;
        
    controlBlock.last = dllNode;
    controlBlock.nodeCount++;
    
    return OK;
}

STATUS CDLList :: insertAfter(DLL_NODE *baseNode, DLL_NODE *dllNode)
{
    DLL_NODE *node;
    
    if(NULL == baseNode || NULL == dllNode || NULL == cmpFn)
        return ERROR;
    
    node = findNode(baseNode);
    
    if(NULL != node)
    {
        if(NULL == node->next)
            controlBlock.last = dllNode;
        else 
            (node->next)->prev = dllNode;
                   
        dllNode->prev = node;
        dllNode->next = node->next;
        node->next = dllNode;
        controlBlock.nodeCount++;
        
        return OK;
    }
    
    return ERROR;
}

STATUS CDLList :: insertBefore(DLL_NODE *baseNode, DLL_NODE *dllNode)
{
    DLL_NODE *node;
    
    if(NULL == baseNode || NULL == dllNode || NULL == cmpFn)
        return ERROR;
    
    node = findNode(baseNode);
    
    if(NULL != node)
    {
        if(NULL == node->prev)
            controlBlock.first = dllNode;
        else
            (node->prev)->next = dllNode;
        
        dllNode->next = node;
        dllNode->prev = node->prev;
        node->prev = dllNode;
        controlBlock.nodeCount++;
        
        return OK;
    }
    
    return ERROR;
}

DLL_NODE * CDLList :: removeHead()
{
    DLL_NODE *node;
    
    if(NULL == controlBlock.first)
        return (DLL_NODE*)NULL;
        
    node = controlBlock.first;
 
     controlBlock.first = (controlBlock.first)->next;
     
     if(NULL == controlBlock.first)
         controlBlock.last = (DLL_NODE*)NULL;
     else
         (controlBlock.first)->prev = (DLL_NODE*)NULL;
     
     controlBlock.nodeCount--;
     
     return node;        
}

DLL_NODE * CDLList :: removeTail()
{
    DLL_NODE *node;
    
    if(NULL == controlBlock.last)
        return (DLL_NODE*)NULL;
    
    node = controlBlock.last;
    
    controlBlock.last = (controlBlock.last)->prev;
    
    if(NULL == controlBlock.last)
        controlBlock.first = NULL;
    else
        (controlBlock.last)->next = (DLL_NODE*)NULL;

    controlBlock.nodeCount--;
    
    return node;
}

DLL_NODE * CDLList :: removeNode(DLL_NODE *dllNode)
{
    DLL_NODE *node;

    if(NULL == dllNode || NULL == cmpFn)
        return (DLL_NODE*)ERROR;
            
    node = findNode(dllNode);
    
    if(NULL != node)
    {
        if(controlBlock.first == node)
            controlBlock.first = node->next;
        if(controlBlock.last == node)
            controlBlock.last = node->prev;
            
        if(NULL != node->prev)
            (node->prev)->next = node->next;
        if(NULL != node->next)
            (node->next)->prev = node->prev;
            
        controlBlock.nodeCount--;
        
        return node;
    }
    
    return (DLL_NODE*)NULL;
}

DLL_NODE * CDLList :: findNode(DLL_NODE *dllNode)
{
    DLL_NODE *node = controlBlock.first, *searchNode = (DLL_NODE*)NULL;
    
    while(NULL != node && NULL != cmpFn)
    {
        if(OK == cmpFn(((INT8*)dllNode + sizeof(DLL_NODE)), ((INT8*)node + sizeof(DLL_NODE))))
        {
            searchNode = node;
            break;
        }
        node = node->next;
    }
    
    return searchNode;
}

DLL_NODE * CDLList :: getHead()
{
    return controlBlock.first;
}

DLL_NODE * CDLList :: getTail()
{
    return controlBlock.last;
}

DLL_NODE * CDLList :: getNext(DLL_NODE *node)
{
    if(NULL != node)
        return node->next;
    else
        return (DLL_NODE *)NULL; 
}

DLL_NODE * CDLList :: getPrev(DLL_NODE *node)
{
    if(NULL != node)
        return node->prev;
    else
        return (DLL_NODE *)NULL;
}

UINT32 CDLList :: getListSize()
{
   return controlBlock.nodeCount; 
}
