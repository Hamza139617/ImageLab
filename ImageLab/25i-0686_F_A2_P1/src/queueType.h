#ifndef QUEUETYPE_H
#define QUEUETYPE_H

#include <iostream>
using namespace std;

template <class Type>
class queueType {
private:
    int frontIndex;
    int rearIndex;
    Type* arr;
    int count;
    int maxQueueSize;

    int addCount;
    int deleteCount;
    int maxRecordedSize;

    void copyQueue(const queueType<Type>& otherQueue);

public:
    queueType(int queueSize = 100);
    queueType(const queueType<Type>& otherQueue);
    ~queueType();
    const queueType<Type>& operator=(const queueType<Type>& otherQueue);

    void initializeQueue();
    bool isEmptyQueue() const;
    bool isFullQueue() const;
    Type front() const;
    Type back() const;
    void addQueue(const Type& queueElement);
    void deleteQueue();
    int size() const;
    int capacity() const;

    const Type* peekAt(int indexFromFront) const;
    int getAddCount() const;
    int getDeleteCount() const;
    int getMaxRecordedSize() const;
};

template <class Type>
queueType<Type>::queueType(int queueSize)
{
    if (queueSize <= 0)
        queueSize = 100;

    maxQueueSize = queueSize;
    arr = new Type[maxQueueSize];
    frontIndex = 0;
    rearIndex = maxQueueSize - 1;
    count = 0;
    addCount = 0;
    deleteCount = 0;
    maxRecordedSize = 0;
}

template <class Type>
queueType<Type>::queueType(const queueType<Type>& otherQueue)
{
    arr = nullptr;
    copyQueue(otherQueue);
}

template <class Type>
queueType<Type>::~queueType()
{
    delete[] arr;
}

template <class Type>
const queueType<Type>& queueType<Type>::operator=(const queueType<Type>& otherQueue)
{
    if (this != &otherQueue)
        copyQueue(otherQueue);
    return *this;
}

template <class Type>
void queueType<Type>::copyQueue(const queueType<Type>& otherQueue)
{
    delete[] arr;

    maxQueueSize = otherQueue.maxQueueSize;
    count = otherQueue.count;
    addCount = otherQueue.addCount;
    deleteCount = otherQueue.deleteCount;
    maxRecordedSize = otherQueue.maxRecordedSize;

    arr = new Type[maxQueueSize];

    for (int i = 0; i < count; i++)
        arr[i] = otherQueue.arr[(otherQueue.frontIndex + i) % maxQueueSize];

    frontIndex = 0;
    rearIndex = (count + maxQueueSize - 1) % maxQueueSize;
}

template <class Type>
void queueType<Type>::initializeQueue()
{
    frontIndex = 0;
    rearIndex = maxQueueSize - 1;
    count = 0;
}

template <class Type>
bool queueType<Type>::isEmptyQueue() const
{
    return count == 0;
}

template <class Type>
bool queueType<Type>::isFullQueue() const
{
    return count == maxQueueSize;
}

template <class Type>
Type queueType<Type>::front() const
{
    if (isEmptyQueue())
    {
        cout << "Cannot front: queue is empty." << endl;
        return Type();
    }
    return arr[frontIndex];
}

template <class Type>
Type queueType<Type>::back() const
{
    if (isEmptyQueue())
    {
        cout << "Cannot back: queue is empty." << endl;
        return Type();
    }
    return arr[rearIndex];
}

template <class Type>
void queueType<Type>::addQueue(const Type& queueElement)
{
    if (isFullQueue())
    {
        cout << "Cannot add: queue is full." << endl;
        return;
    }

    rearIndex = (rearIndex + 1) % maxQueueSize;
    arr[rearIndex] = queueElement;
    count++;
    addCount++;

    if (count > maxRecordedSize)
        maxRecordedSize = count;
}

template <class Type>
void queueType<Type>::deleteQueue()
{
    if (isEmptyQueue())
    {
        cout << "Cannot delete: queue is empty." << endl;
        return;
    }

    frontIndex = (frontIndex + 1) % maxQueueSize;
    count--;
    deleteCount++;
}

template <class Type>
int queueType<Type>::size() const
{
    return count;
}

template <class Type>
int queueType<Type>::capacity() const
{
    return maxQueueSize;
}

template <class Type>
const Type* queueType<Type>::peekAt(int indexFromFront) const
{
    if (indexFromFront < 0 || indexFromFront >= count)
        return nullptr;

    return &arr[(frontIndex + indexFromFront) % maxQueueSize];
}

template <class Type>
int queueType<Type>::getAddCount() const
{
    return addCount;
}

template <class Type>
int queueType<Type>::getDeleteCount() const
{
    return deleteCount;
}

template <class Type>
int queueType<Type>::getMaxRecordedSize() const
{
    return maxRecordedSize;
}

#endif