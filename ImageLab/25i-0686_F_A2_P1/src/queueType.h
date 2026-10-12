#ifndef QUEUETYPE_H
#define QUEUETYPE_H


template <class Type>
class queueType {
private:

	int front;
	int rear;
	Type* arr;
	int count;
	int max;

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
void queueType<T>::copyQueue(const queueType<T>& oQueue) {

	front = oQueue.front;
	rear = oQueue.rear;
	count = oQueue.count;
	max = oQueue.max;

	arr = new T[max];


	if (oQueue.front < oQueue.rear) {
		for (int i = front; i <= rear; i++)
			arr[i] = oQueue.arr[i];
	}



}

template <typename T>
queueType<T>::queueType(int queueSize) {
	if (queueSize <= 0) queueSize = 100;

	max = queueSize;
	count = 0;
	front = -1;
	rear = -1;

	arr = new T[max];


}

template <typename T>
queueType<T>::queueType(const queueType<T>& oQueue) {

	copyQueue(oQueue);

}

template <typename T>
const queueType<T>& queueType<T>::operator=(const queueType<T>& otherQueue) {

	if (this != otherQueue) {
		copyQueue(otehrQueue);
	}

	return *this;
}


template <typename T>
queueType<T>::~queueType() {
	
	initializeQueue();

}

template <typename T>
int queueType<T>::size() const {
	return count;
}

template <typename T>
int queueType<T>::capacity() const {
	return max;
}

template <typename T>
T queueType<T>::front() const {
	if (count == 0) return T();

	return arr[front];
}

template <typename T>
T queueType<T>::back() const {
	if (count == 0) return T();

	return arr[rear];
}

template <typename T>
bool queueType<T>::isEmptyQueue() const {
	return count == 0;
}

template <typename T>
bool queueType<T>::isFullQueue() const {
	return count + 1 == max;
}

template <typename T>
void queueType<T>::addQueue(const T& item) {

	count++;
	if (count >= capacity) {
		cout << endl << "The queue is full" << endl;
		return;
	}

	rear = (rear + 1) % capacity;
	arr[rear] = item;
	
	return;

}

template <typename T>
void queueType<T>::initialzeQueue() {

	delete[] arr;
	arr = nullptr;

	front = -1;
	rear = -1;
	count = 0;

}


template <typename T>
void queueType<T>::deleteQueue() {
	if (count <= 0) return;

	count--;

	front = (front + 1) % max;

	return;

}



#endif