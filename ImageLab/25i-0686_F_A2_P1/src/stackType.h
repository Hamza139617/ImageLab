#pragma once
#ifndef STACKTYPE_H
#define STACKTYPE_H

#include <iostream>

template <class Type>
class stackType {
private:

	struct Node {
		Type data;
		Node* link;
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
	void initializeStack();

	const Type* peekAt(int topIndex) const;
	int getPushCount() const;
	int getPopCount();

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


#endif