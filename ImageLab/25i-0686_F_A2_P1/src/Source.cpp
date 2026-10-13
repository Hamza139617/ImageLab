#include "queueType.h"
#include "stackType.h"

int main() {

	stackType<int> stack(20);
	queueType<int> queue(20);

	queue.addQueue(1);
	queue.addQueue(2);
	queue.addQueue(3);
	
	queue.deleteQueue();

	cout << queue.front();


	return 0;
}