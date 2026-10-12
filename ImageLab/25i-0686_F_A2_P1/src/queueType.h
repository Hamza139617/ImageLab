#ifndef QUEUETYPE_H
#define QUEUETYPE_H


template <class Type>
class queueType {
private:

	int front;
	int rear;
	Type* arr;
	int count;
	int capacity;

	// helper function section

	void copyQueue(const queueType<Type>& oQueue);

public:

	void initialzeQueue();
	bool isEmptyQueue() const;
	bool isFullQueue() const;
	Type front() const;
	Type back() const;
	void addQueue(const Type& queueElement);
	void deleteQueue();
	int size() const;
	int capacity() const;
	queueType(int queueSize = 100);
	queueType(const queueType<Type>& otherQueue);
	~queueType();
	const queueType<Type>& operator=(const queueType<Type>& otherQueue);


};

template <typename T>
queueType<T>::queueType(int queueSize) {
	if (queueSize <= 0) queueSize = 100;

	capacity = queueSize;
	count = 0;
	front = -1;
	rear = -1;

	arr = new T[capacity];


}

template <typename T>
queueType<T>::queueType(const queueType<T>& oQueue) {

}


#endif