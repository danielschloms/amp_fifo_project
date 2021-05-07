#ifndef FIFO_LF_H
#define FIFO_LF_H

class FIFO_LF{
private:

public:
    FIFO_LF();      // Constructor
    bool enq();     // Enqueue operation
    int deq();      // Dequeue operation
};

#endif //FIFO_LF_H