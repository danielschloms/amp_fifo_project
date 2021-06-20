#ifndef QUEUE_H
#define QUEUE_H
class Queue{
public:
    virtual bool enq(int index) = 0;
    virtual int deq(int * error_code) = 0;
    virtual void kill() = 0;
};
#endif //QUEUE_H