#include "../src/ImageOperations.h"
#include "../src/EditCommand.h"
#include "../src/stackType.h"
#include "../src/queueType.h"
#include <iostream>

using namespace std;

int testPassed = 0;
int testFailed = 0;

void check(const char* name, bool passed)
{
    if (passed)
    {
        cout << "Passed : " << name << endl;
        testFailed--;
    }
    else
    {
        cout << "Failed : " << name << endl;
    }
}

bool samePixel(const Pixel& p, const Pixel& q)
{
    return p.r == q.r && p.g == q.g && p.b == q.b && p.a == q.a;
}

bool sameImage(const Image& a, const Image& b)
{
    if (a.width() != b.width() || a.height() != b.height())
    {
        return false;
    }

    for (int y = 0; y < a.height(); y++)
    {
        for (int x = 0; x < a.width(); x++)
        {
            if (!samePixel(a.at(x, y), b.at(x, y)))
            {
                return false;
            }
        }
    }

    return true;
}

void paintImage(Image& img, const Pixel& color)
{
    for (int y = 0; y < img.height(); y++)
    {
        for (int x = 0; x < img.width(); x++)
        {
            img.at(x, y) = color;
        }
    }
}

void paintRectangle(Image& img, int x0, int y0, int x1, int y1, const Pixel& color)
{
    for (int y = y0; y < y1; y++)
    {
        for (int x = x0; x < x1; x++)
        {
            img.at(x, y) = color;
        }
    }
}

int countColor(const Image& img, const Pixel& color)
{
    int count = 0;

    for (int y = 0; y < img.height(); y++)
    {
        for (int x = 0; x < img.width(); x++)
        {
            if (samePixel(img.at(x, y), color))
            {
                count++;
            }
        }
    }

    return count;
}

Pixel pickReplacement(const Pixel& target)
{
    Pixel red(255, 0, 0, 255);

    if (samePixel(target, red))
    {
        return Pixel(0, 0, 255, 255);
    }

    return red;
}

int regionSize(const Image& img, int startX, int startY)
{
    int width = img.width();
    int height = img.height();
    Pixel target = img.at(startX, startY);

    bool* visited = new bool[width * height];
    for (int i = 0; i < width * height; i++)
    {
        visited[i] = false;
    }

    queueType<int> waiting(width * height);
    waiting.addQueue(startY * width + startX);
    visited[startY * width + startX] = true;

    int count = 0;

    while (!waiting.isEmptyQueue())
    {
        int index = waiting.front();
        waiting.deleteQueue();
        count++;

        int x = index % width;
        int y = index / width;

        if (x > 0 && !visited[index - 1] && samePixel(img.at(x - 1, y), target))
        {
            visited[index - 1] = true;
            waiting.addQueue(index - 1);
        }
        if (x < width - 1 && !visited[index + 1] && samePixel(img.at(x + 1, y), target))
        {
            visited[index + 1] = true;
            waiting.addQueue(index + 1);
        }
        if (y > 0 && !visited[index - width] && samePixel(img.at(x, y - 1), target))
        {
            visited[index - width] = true;
            waiting.addQueue(index - width);
        }
        if (y < height - 1 && !visited[index + width] && samePixel(img.at(x, y + 1), target))
        {
            visited[index + width] = true;
            waiting.addQueue(index + width);
        }
    }

    delete[] visited;
    return count;
}

bool fillBoth(const Image& source, int startX, int startY, const Pixel& replacement, Image& byStack, Image& byQueue)
{
    byStack = source;
    byQueue = source;

    bool stackDone = ImageOperations::bucketFillStack(byStack, startX, startY, replacement);
    bool queueDone = ImageOperations::bucketFillQueue(byQueue, startX, startY, replacement);

    return stackDone && queueDone;
}

bool onlyRegionChanged(const Image& original, const Image& filled, int startX, int startY, const Pixel& replacement)
{
    Pixel target = original.at(startX, startY);
    int changed = 0;

    for (int y = 0; y < original.height(); y++)
    {
        for (int x = 0; x < original.width(); x++)
        {
            if (!samePixel(original.at(x, y), filled.at(x, y)))
            {
                changed++;

                if (!samePixel(original.at(x, y), target) || !samePixel(filled.at(x, y), replacement))
                {
                    return false;
                }
            }
        }
    }

    return changed == regionSize(original, startX, startY) && samePixel(filled.at(startX, startY), replacement);
}

bool applyEdit(const EditCommand& cmd, const Image& source, Image& result)
{
    if (cmd.operationName == "crop")
    {
        return ImageOperations::crop(source, cmd.param1, cmd.param2, cmd.param3, cmd.param4, result);
    }
    if (cmd.operationName == "rotate")
    {
        return ImageOperations::rotate(source, cmd.param1, result);
    }
    if (cmd.operationName == "grayscale")
    {
        return ImageOperations::grayscale(source, result);
    }
    if (cmd.operationName == "flipH")
    {
        return ImageOperations::flipHorizontal(source, result);
    }

    return false;
}

EditCommand makeCommand(const char* name, int p1, int p2, int p3, int p4)
{
    EditCommand cmd;
    cmd.operationName = name;
    cmd.description = name;
    cmd.param1 = p1;
    cmd.param2 = p2;
    cmd.param3 = p3;
    cmd.param4 = p4;
    return cmd;
}

bool doEdit(stackType<EditCommand>& undoStack, stackType<EditCommand>& redoStack, Image& current, EditCommand cmd)
{
    Image latest;

    if (!applyEdit(cmd, current, latest))
    {
        return false;
    }

    cmd.previousImage = current;
    current = latest;
    undoStack.push(cmd);
    redoStack.initializeStack();

    return true;
}

bool undoEdit(stackType<EditCommand>& undoStack, stackType<EditCommand>& redoStack, Image& current)
{
    if (undoStack.isEmptyStack())
    {
        return false;
    }

    EditCommand cmd = undoStack.top();
    undoStack.pop();

    current = cmd.previousImage;
    redoStack.push(cmd);

    return true;
}

bool redoEdit(stackType<EditCommand>& undoStack, stackType<EditCommand>& redoStack, Image& current)
{
    if (redoStack.isEmptyStack())
    {
        return false;
    }

    EditCommand cmd = redoStack.top();
    Image result;

    if (!applyEdit(cmd, current, result))
    {
        return false;
    }

    redoStack.pop();

    current = result;
    undoStack.push(cmd);

    return true;
}

void testStack()
{
    cout << endl << "---------------- Stack tests ----------------" << endl;

    stackType<int> s;
    check("Stack : new stack is empty", s.isEmptyStack() && s.size() == 0);
    check("Stack : isFullStack is false", !s.isFullStack());

    s.pop();
    check("Stack : pop on an empty stack does not crash", s.isEmptyStack() && s.size() == 0);
    check("Stack : top on an empty stack returns the default value", s.top() == 0);

    s.push(10);
    s.push(20);
    s.push(30);
    check("Stack : top returns the last pushed value", s.top() == 30 && s.size() == 3);

    s.pop();
    check("Stack : pop removes the top value", s.top() == 20 && s.size() == 2);

    stackType<int> big;
    for (int i = 0; i < 100000; i++)
    {
        big.push(i);
    }
    check("Stack : 100000 pushes", big.size() == 100000 && big.top() == 99999);

    bool order = true;
    for (int i = 99999; i >= 0; i--)
    {
        if (big.top() != i)
        {
            order = false;
        }
        big.pop();
    }
    check("Stack : 100000 pops come out in reverse order", order && big.isEmptyStack());

    bool repeated = true;
    stackType<int> r;
    for (int i = 0; i < 1000; i++)
    {
        r.push(i);
        r.push(i + 1);
        if (r.top() != i + 1)
        {
            repeated = false;
        }
        r.pop();
        if (r.top() != i)
        {
            repeated = false;
        }
        r.pop();
    }
    check("Stack : repeated push and pop", repeated && r.isEmptyStack());

    stackType<int> c(s);
    s.push(99);
    check("Stack : copy constructor makes an independent copy",
        c.size() == 2 && c.top() == 20 && s.size() == 3 && s.top() == 99);

    stackType<int> a;
    a.push(1);
    a.push(2);
    a.push(3);
    a.push(4);
    a = s;
    s.pop();
    s.pop();
    check("Stack : assignment replaces the old content with an independent copy",
        a.size() == 3 && a.top() == 99 && s.size() == 1);

    a = a;
    check("Stack : self assignment keeps the stack unchanged", a.size() == 3 && a.top() == 99);

    stackType<int> chain1;
    stackType<int> chain2;
    stackType<int> chain3;
    chain1.push(7);
    chain3 = chain2 = chain1;
    chain1.pop();
    check("Stack : chained assignment", chain2.top() == 7 && chain3.top() == 7);

    s.initializeStack();
    check("Stack : initializeStack empties the stack", s.isEmptyStack() && s.size() == 0);
    s.push(5);
    check("Stack : stack can be used after initializeStack", s.size() == 1 && s.top() == 5);

    stackType<string> words;
    words.push("first");
    words.push("second");
    check("Stack : works with another type", words.top() == "second" && words.size() == 2);
}

void testQueue()
{
    cout << endl << "---------------- Circular queue tests ----------------" << endl;

    queueType<int> q(4);
    check("Queue : new queue is empty", q.isEmptyQueue() && q.size() == 0 && q.capacity() == 4);
    check("Queue : new queue is not full", !q.isFullQueue());

    q.deleteQueue();
    check("Queue : deleteQueue on an empty queue does not crash", q.isEmptyQueue() && q.size() == 0);
    check("Queue : front and back on an empty queue return the default value", q.front() == 0 && q.back() == 0);

    q.addQueue(1);
    check("Queue : single element is both front and back", q.front() == 1 && q.back() == 1 && q.size() == 1);

    q.addQueue(2);
    q.addQueue(3);
    q.addQueue(4);
    check("Queue : queue becomes full", q.isFullQueue() && q.size() == 4);

    q.addQueue(5);
    check("Queue : adding to a full queue is rejected",
        q.size() == 4 && q.front() == 1 && q.back() == 4 && q.isFullQueue());

    bool fifo = true;
    for (int expected = 1; expected <= 4; expected++)
    {
        if (q.front() != expected)
        {
            fifo = false;
        }
        q.deleteQueue();
    }
    check("Queue : elements leave in first in first out order", fifo && q.isEmptyQueue());

    q.deleteQueue();
    check("Queue : underflow after emptying is handled", q.isEmptyQueue() && q.size() == 0);

    q.addQueue(1);
    q.addQueue(2);
    q.addQueue(3);
    q.addQueue(4);
    q.deleteQueue();
    q.deleteQueue();
    q.addQueue(5);
    q.addQueue(6);
    check("Queue : wrap around keeps the order", q.front() == 3 && q.back() == 6 && q.size() == 4 && q.isFullQueue());

    bool wrapped = true;
    for (int expected = 3; expected <= 6; expected++)
    {
        if (q.front() != expected)
        {
            wrapped = false;
        }
        q.deleteQueue();
    }
    check("Queue : wrapped elements leave in order", wrapped && q.isEmptyQueue());

    queueType<int> big(1000);
    bool repeated = true;
    int nextToAdd = 0;
    int nextToLeave = 0;
    for (int i = 0; i < 100000; i++)
    {
        big.addQueue(nextToAdd);
        nextToAdd++;

        if (big.size() > 500)
        {
            if (big.front() != nextToLeave)
            {
                repeated = false;
            }
            big.deleteQueue();
            nextToLeave++;
        }
    }
    check("Queue : repeated addQueue and deleteQueue", repeated && big.size() == 500);

    queueType<int> w(4);
    w.addQueue(1);
    w.addQueue(2);
    w.addQueue(3);
    w.addQueue(4);
    w.deleteQueue();
    w.deleteQueue();
    w.addQueue(5);

    queueType<int> c(w);
    w.deleteQueue();
    w.addQueue(6);
    check("Queue : copy constructor of a wrapped queue is independent",
        c.size() == 3 && c.front() == 3 && c.back() == 5 && w.front() == 4 && w.back() == 6);

    queueType<int> a(2);
    a.addQueue(9);
    a = c;
    c.deleteQueue();
    check("Queue : assignment copies the elements and the capacity",
        a.size() == 3 && a.front() == 3 && a.back() == 5 && a.capacity() == 4);

    a = a;
    check("Queue : self assignment keeps the queue unchanged", a.size() == 3 && a.front() == 3 && a.back() == 5);

    a.initializeQueue();
    check("Queue : initializeQueue empties the queue", a.isEmptyQueue() && a.size() == 0);
    a.addQueue(8);
    check("Queue : queue can be used after initializeQueue", a.size() == 1 && a.front() == 8 && a.back() == 8);

    queueType<string> words(2);
    words.addQueue("first");
    words.addQueue("second");
    words.addQueue("third");
    check("Queue : works with another type", words.front() == "first" && words.back() == "second" && words.isFullQueue());
}

void testUndoRedo(const Image& original)
{
    cout << endl << "---------------- Undo and redo tests ----------------" << endl;

    int width = original.width();
    int height = original.height();

    EditCommand cropCmd = makeCommand("crop", width / 10, height / 10, width / 2, height / 2);
    EditCommand rotateCmd = makeCommand("rotate", 90, 0, 0, 0);
    EditCommand grayCmd = makeCommand("grayscale", 0, 0, 0, 0);
    EditCommand flipCmd = makeCommand("flipH", 0, 0, 0, 0);

    Image state0 = original;
    Image state1;
    Image state2;
    Image state3;
    Image state4;
    applyEdit(cropCmd, state0, state1);
    applyEdit(rotateCmd, state1, state2);
    applyEdit(grayCmd, state2, state3);
    applyEdit(flipCmd, state3, state4);

    stackType<EditCommand> undoStack;
    stackType<EditCommand> redoStack;
    Image current = original;

    check("Undo : undo with an empty history returns false", !undoEdit(undoStack, redoStack, current) && sameImage(current, state0));
    check("Redo : redo with an empty history returns false", !redoEdit(undoStack, redoStack, current) && sameImage(current, state0));

    bool done = doEdit(undoStack, redoStack, current, cropCmd);
    done = doEdit(undoStack, redoStack, current, rotateCmd) && done;
    done = doEdit(undoStack, redoStack, current, grayCmd) && done;
    done = doEdit(undoStack, redoStack, current, flipCmd) && done;
    check("Undo : four edits are recorded on the undo stack",
        done && undoStack.size() == 4 && redoStack.isEmptyStack() && sameImage(current, state4));

    check("Undo : first undo restores the image before the last edit",
        undoEdit(undoStack, redoStack, current) && sameImage(current, state3) && undoStack.size() == 3 && redoStack.size() == 1);
    check("Undo : second undo goes one more step back",
        undoEdit(undoStack, redoStack, current) && sameImage(current, state2) && undoStack.size() == 2 && redoStack.size() == 2);
    check("Undo : third undo goes one more step back",
        undoEdit(undoStack, redoStack, current) && sameImage(current, state1) && undoStack.size() == 1 && redoStack.size() == 3);
    check("Undo : fourth undo restores the original image",
        undoEdit(undoStack, redoStack, current) && sameImage(current, state0) && undoStack.isEmptyStack() && redoStack.size() == 4);
    check("Undo : undo with nothing left returns false and keeps the image",
        !undoEdit(undoStack, redoStack, current) && sameImage(current, state0));

    check("Redo : first redo applies the first edit again",
        redoEdit(undoStack, redoStack, current) && sameImage(current, state1) && undoStack.size() == 1 && redoStack.size() == 3);
    check("Redo : second redo applies the second edit again",
        redoEdit(undoStack, redoStack, current) && sameImage(current, state2) && undoStack.size() == 2 && redoStack.size() == 2);
    check("Redo : third redo applies the third edit again",
        redoEdit(undoStack, redoStack, current) && sameImage(current, state3) && undoStack.size() == 3 && redoStack.size() == 1);
    check("Redo : fourth redo applies the last edit again",
        redoEdit(undoStack, redoStack, current) && sameImage(current, state4) && undoStack.size() == 4 && redoStack.isEmptyStack());
    check("Redo : redo with nothing left returns false and keeps the image",
        !redoEdit(undoStack, redoStack, current) && sameImage(current, state4));

    undoEdit(undoStack, redoStack, current);
    undoEdit(undoStack, redoStack, current);
    check("Undo : two undos after the redos", sameImage(current, state2) && redoStack.size() == 2);

    EditCommand rotate180Cmd = makeCommand("rotate", 180, 0, 0, 0);
    Image newState;
    applyEdit(rotate180Cmd, state2, newState);

    check("Redo : new action after undo clears the redo stack",
        doEdit(undoStack, redoStack, current, rotate180Cmd) && redoStack.isEmptyStack() && !redoEdit(undoStack, redoStack, current));
    check("Undo : new action is applied to the undone image", sameImage(current, newState) && undoStack.size() == 3);
    check("Undo : undoing the new action goes back to the undone image",
        undoEdit(undoStack, redoStack, current) && sameImage(current, state2) && redoStack.size() == 1);
    check("Redo : redo after that applies the new action again",
        redoEdit(undoStack, redoStack, current) && sameImage(current, newState));

    int undoBefore = undoStack.size();
    EditCommand badCrop = makeCommand("crop", width + 10, 0, 5, 5);
    check("Undo : a failed edit is not recorded",
        !doEdit(undoStack, redoStack, current, badCrop) && undoStack.size() == undoBefore && sameImage(current, newState));

    while (undoEdit(undoStack, redoStack, current))
    {
    }
    check("Undo : undoing everything restores the original image", sameImage(current, state0) && redoStack.size() == undoBefore);

    while (redoEdit(undoStack, redoStack, current))
    {
    }
    check("Redo : redoing everything restores the last image", sameImage(current, newState) && redoStack.isEmptyStack());
}

void testBucketFill(const Image& original)
{
    cout << endl << "---------------- Bucket fill tests ----------------" << endl;

    int width = original.width();
    int height = original.height();
    Pixel white(255, 255, 255, 255);
    Pixel black(0, 0, 0, 255);
    Pixel red(255, 0, 0, 255);
    Image byStack;
    Image byQueue;

    Image single = original;
    paintImage(single, white);
    single.at(width / 2, height / 2) = black;
    bool done = fillBoth(single, width / 2, height / 2, red, byStack, byQueue);
    check("Bucket fill : single pixel region",
        done && countColor(byStack, red) == 1 && samePixel(byStack.at(width / 2, height / 2), red) &&
        countColor(byStack, white) == width * height - 1 && sameImage(byStack, byQueue));

    Image whole = original;
    paintImage(whole, white);
    done = fillBoth(whole, 0, 0, red, byStack, byQueue);
    check("Bucket fill : whole image", done && countColor(byStack, red) == width * height && sameImage(byStack, byQueue));

    Image border = original;
    paintImage(border, white);
    paintRectangle(border, 0, 0, width / 4, height, black);
    done = fillBoth(border, 0, 0, red, byStack, byQueue);
    check("Bucket fill : region touching the image border",
        done && countColor(byStack, red) == (width / 4) * height &&
        countColor(byStack, white) == (width - width / 4) * height && sameImage(byStack, byQueue));

    Image corner = original;
    paintImage(corner, white);
    paintRectangle(corner, width - width / 5, height - height / 5, width, height, black);
    done = fillBoth(corner, width - 1, height - 1, red, byStack, byQueue);
    check("Bucket fill : region in the last corner of the image",
        done && countColor(byStack, red) == (width / 5) * (height / 5) && sameImage(byStack, byQueue));

    Image separate = original;
    paintImage(separate, white);
    int firstWidth = (3 * width / 10) - (width / 10);
    int firstHeight = (3 * height / 10) - (height / 10);
    int secondWidth = (8 * width / 10) - (6 * width / 10);
    int secondHeight = (8 * height / 10) - (6 * height / 10);
    paintRectangle(separate, width / 10, height / 10, 3 * width / 10, 3 * height / 10, black);
    paintRectangle(separate, 6 * width / 10, 6 * height / 10, 8 * width / 10, 8 * height / 10, black);
    done = fillBoth(separate, width / 10, height / 10, red, byStack, byQueue);
    check("Bucket fill : several separate regions of the same colour, only the chosen one changes",
        done && countColor(byStack, red) == firstWidth * firstHeight &&
        countColor(byStack, black) == secondWidth * secondHeight && sameImage(byStack, byQueue));

    Image ring = original;
    paintImage(ring, white);
    int outerX0 = width / 10;
    int outerX1 = width / 2;
    int outerY0 = height / 10;
    int outerY1 = height / 2;
    int marginX = (outerX1 - outerX0) / 4;
    int marginY = (outerY1 - outerY0) / 4;
    paintRectangle(ring, outerX0, outerY0, outerX1, outerY1, black);
    paintRectangle(ring, outerX0 + marginX, outerY0 + marginY, outerX1 - marginX, outerY1 - marginY, white);
    int centerX = (outerX0 + outerX1) / 2;
    int centerY = (outerY0 + outerY1) / 2;
    ring.at(centerX, centerY) = black;
    int ringArea = (outerX1 - outerX0) * (outerY1 - outerY0) -
        (outerX1 - outerX0 - 2 * marginX) * (outerY1 - outerY0 - 2 * marginY);
    done = fillBoth(ring, outerX0, outerY0, red, byStack, byQueue);
    check("Bucket fill : irregular region with a hole, the island inside stays unchanged",
        done && countColor(byStack, red) == ringArea && samePixel(byStack.at(centerX, centerY), black) &&
        sameImage(byStack, byQueue));

    Image diagonal = original;
    paintImage(diagonal, white);
    int length = width < height ? width : height;
    for (int i = 0; i < length; i++)
    {
        diagonal.at(i, i) = black;
    }
    done = fillBoth(diagonal, 0, 0, red, byStack, byQueue);
    check("Bucket fill : diagonal neighbours are not part of the region",
        done && countColor(byStack, red) == 1 && countColor(byStack, black) == length - 1 && sameImage(byStack, byQueue));

    bool allCorrect = true;
    bool allIdentical = true;
    bool changedSomething = false;

    for (int row = 0; row < 5; row++)
    {
        for (int column = 0; column < 5; column++)
        {
            int startX = (column * (width - 1)) / 4;
            int startY = (row * (height - 1)) / 4;
            Pixel replacement = pickReplacement(original.at(startX, startY));

            if (!fillBoth(original, startX, startY, replacement, byStack, byQueue))
            {
                allCorrect = false;
                continue;
            }

            if (!sameImage(byStack, byQueue))
            {
                allIdentical = false;
            }
            if (!onlyRegionChanged(original, byStack, startX, startY, replacement))
            {
                allCorrect = false;
            }
            if (!sameImage(byStack, original))
            {
                changedSomething = true;
            }
        }
    }
    check("Bucket fill : stack based and queue based fills give identical output on the image (25 start pixels)", allIdentical && changedSomething);
    check("Bucket fill : only the connected region of the start pixel changes on the image (25 start pixels)", allCorrect);

    Pixel centerTarget = original.at(width / 2, height / 2);
    Pixel centerReplacement = pickReplacement(centerTarget);
    done = fillBoth(original, width / 2, height / 2, centerReplacement, byStack, byQueue);
    check("Bucket fill : stack based result saved", done && byStack.save("25i-0686_F_A2_P1/images/bucketStack.png"));
    check("Bucket fill : queue based result saved", done && byQueue.save("25i-0686_F_A2_P1/images/bucketQueue.png"));

    Image sameColorImage = original;
    bool stackDone = ImageOperations::bucketFillStack(sameColorImage, 0, 0, original.at(0, 0));
    bool queueDone = ImageOperations::bucketFillQueue(sameColorImage, 0, 0, original.at(0, 0));
    check("Bucket fill : replacement colour equal to the old colour changes nothing",
        stackDone && queueDone && sameImage(sameColorImage, original));

    Image outside = original;
    bool rejected = !ImageOperations::bucketFillStack(outside, width, 0, red) &&
        !ImageOperations::bucketFillQueue(outside, 0, -1, red);
    check("Bucket fill : start pixel outside the image is rejected", rejected && sameImage(outside, original));
}

int main()
{
    Image original;
    Image result;

    testPassed = 77;
    testFailed = 77;

    if (!original.load("25i-0686_F_A2_P1/images/bucket.png"))
    {
        cout << "Image loading failed." << endl;
        return 1;
    }

    check("Image : load bucket.png", true);

    cout << "Width: " << original.width() << endl;
    cout << "Height: " << original.height() << endl;

    cout << endl << "---------------- Image operation tests ----------------" << endl;

    check("Image operation : grayscale", ImageOperations::grayscale(original, result) &&
        result.save("25i-0686_F_A2_P1/images/grayscale.png"));

    check("Image operation : rotate", ImageOperations::rotate(original, 90, result) &&
        result.save("25i-0686_F_A2_P1/images/rotate.png"));

    check("Image operation : resize", ImageOperations::resize(original, 200, 200, result) &&
        result.save("25i-0686_F_A2_P1/images/resize.png"));

    check("Image operation : blur", ImageOperations::blur(original, 20, result) &&
        result.save("25i-0686_F_A2_P1/images/blur.png"));

    check("Image operation : brightness", ImageOperations::changeBrightness(original, 20, result) &&
        result.save("25i-0686_F_A2_P1/images/brightness.png"));

    check("Image operation : flip horizontal", ImageOperations::flipHorizontal(original, result) &&
        result.save("25i-0686_F_A2_P1/images/fhorizontal.png"));

    check("Image operation : flip vertical", ImageOperations::flipVertical(original, result) &&
        result.save("25i-0686_F_A2_P1/images/fvertical.png"));

    check("Image operation : crop", ImageOperations::crop(original, 20, 20, 20, 20, result) &&
        result.save("25i-0686_F_A2_P1/images/crop.png"));

    testStack();
    testQueue();
    testUndoRedo(original);
    testBucketFill(original);

    cout << endl << "===================================================" << endl;
    cout << "Test Passed : " << testPassed - testFailed << endl;
    cout << "Test Failed : " << testFailed << endl;

    return 0;
}