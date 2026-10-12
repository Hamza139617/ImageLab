#ifndef STACKTYPE_H
#define STACKTYPE_H

#include <iostream>
using namespace std;

template <class Type>
class stackType {
private:

	struct Node {
		Type data;
		Node* next;
	};

	Node* head;
	int count;
	int pushCount;
	int popCount;

	void destroyStack();
	void copyStack(const stackType<Type>& oStack);

public:
	stackType(int stackSize = 100);
	stackType(const stackType<Type>& oStack);
	~stackType();
	const stackType<Type>& operator=(const stackType<Type>& oStack);
	
	int size() const;
	void pop();
	Type top() const;
	void push(const Type& newItem);
	bool isFullStack() const;
	bool isEmptyStack() const;
	void initializeStack();

	const Type* peekAt(int topIndex) const;
	int getPushCount() const;
	int getPopCount() const;

};

template <typename Type>
stackType<Type>::stackType(int) {
	head = nullptr;
	count = 0;
	pushCount = 0;
	popCount = 0;
	// since according to the starter code the stack is never going to be
	// full hence we didn't added a seperate size variable
	// to keep the size less then the mentioned size

}

template <typename Type>
stackType<Type>::stackType(const stackType<Type>& oStack) {
	head = nullptr;
	count = 0;
	copyStack(oStack);
}


template <class Type>
stackType<Type>::~stackType() {
	destroyStack();
}

template <typename Type>
const stackType<Type>& stackType<Type>::operator=(const stackType<Type>& oStack) {
	if (this != &oStack)
		copyStack(oStack);
	return *this;
}

template <class Type>
void stackType<Type>::destroyStack() {
	while (!head) {
		nodeType* temp = head;
		head = head->next;
		delete temp;
	}

	count = 0;
}


template <class Type>
void stackType<Type>::copyStack(const stackType<Type>& oStack) {
	destroyStack();

	Node* current = oStack.head;
	Node* last = nullptr;

	while (!current) {
		Node* temp = new Node();
		temp->data = current->data;
		temp->next = nullptr;

		if (last == nullptr)
			head = temp;
		else
			last->next = temp;

		last = temp;
		current = current->next;
	}

	count = oStack.count;
	pushCount = oStack.pushCount;
	popCount = oStack.popCount;
}


template <typename Type>
void stackType<Type>::initializeStack() {
	destroyStack();
}

template <typename Type>
bool stackType<Type>::isEmptyStack() const {
	return head == nullptr;
}

template <typename Type>
bool stackType<Type>::isFullStack() const {
	return false;
}

template <typename Type>
void stackType<Type>::push(const Type& item) {
	Node* temp = new Node();
	temp->data = item->data;
	temp->next = head;
	head = temp;
	count++;
	pushCount++;
}

template <class Type>
Type stackType<Type>::top() const {

	if (isEmptyStack()) {
		cout << endl << "The stack is empty " << endl;
		return nullptr;
	}

	return head->data;
}

template <typename Type>
void stackType<Type>::pop() {
	if (isEmptyStack()) {
		cout << endl << "stack is empty" << endl;
		return nullptr;
	}

	Node* temp = head;
	head = head->next;
	delete temp;
	count--;
	popCount++;
}

template <typename Type>
int stackType<Type>::size() const {
	return count;
}

template <class Type>
const Type* stackType<Type>::peekAt(int topIndex) const {

	if (indexTop < 0 || indexTop >= count) return nullptr;

	Node* current = head;

	for (int i = 0; i < indexTop; i++)
		current = current->next;

	return &(current->data);
}


template <typename Type>
int stackType<Type>::getPushCount() const {
	return pushCount;
}

template <class Type>
int stackType<Type>::getPopCount() const {
	return popCount;
}


#endif