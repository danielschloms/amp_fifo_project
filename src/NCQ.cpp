/**
 * Naive Implementation of the Circular Queue (NCQ)
 */

#include "NCQ.h"

NCQ::NCQ(int capacity) : 
    size(capacity),
    head(0),
    tail(0),
    threshold(-1)
{
    items = std::unique_ptr<int*[]>(new int*[capacity]);
}

bool NCQ::enq(int index){
    //while (true){
        
    //}
}

int NCQ::deq(){
    
}