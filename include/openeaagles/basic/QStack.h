//------------------------------------------------------------------------------
// Template QStack<T>
//      and Eaagles::Basic::Object::QStack<T>
//
// Description: Simple stack of items of type T
//
// Notes:
//    1) Use the constructor's 'ssize' parameter to set the max size of the stack.
//    2) Use push() to add items and pop() to remove items.
//    3) push(), pop() and clear() are internally protected by a semaphore
//
// Examples:
//    QStack<int>* q1 = new QStack<int>(100); // stack size 100 items
//    s1->push(1);          // pushes 1 on to the queue
//    s1->push(2);          // pushes 2 on to the queue
//    int i = s1->pop();    // i is equal to 2
//    int j = s1->pop();    // j is equal to 1
//------------------------------------------------------------------------------
template <class T> class QStack {
public:
   QStack(const unsigned int ssize) : SIZE(ssize), sp(ssize), semaphore(0)  { stack = new T[SIZE]; }
   QStack(const QStack<T> &s1) : SIZE(s1.SIZE), sp(s1.SIZE), semaphore(0)   { stack = new T[SIZE]; }
   ~QStack()                                                                { delete[] stack; }
   
   unsigned int entries() const   { return (SIZE - sp); }
   bool isEmpty() const           { return (sp == SIZE); /* Empty when stack pointer equals stack size */ }
   bool isNotEmpty() const        { return !isEmpty(); }
   bool isFull() const            { return (entries() >= SIZE); }
   bool isNotFull() const         { return !isFull(); }

   // Pushes an item on to the stack
   bool push(T item) {
      lcLock( semaphore );
      bool ok = false;
      if (sp > 0) {
         stack[--sp] = item;
         ok = true;
      }
      lcUnlock( semaphore );
      return ok;
   }

   // Pops an item from the top of the stack
   T pop() {
      lcLock( semaphore );
      T ii = 0;
      if (sp < SIZE) ii = stack[sp++];
      lcUnlock( semaphore );
      return ii;
   }

   // Clears the stack
   void clear() {
      lcLock( semaphore );
      sp = SIZE;
      lcUnlock( semaphore );
   }

private:
   QStack<T>& operator=(QStack<T>&) { return *this; }
   T* stack;                // The Stack
   const unsigned int SIZE; // Max size of the stack
   unsigned int sp;         // Stack pointer
   mutable long semaphore;  // ref(), unref() semaphore
};
