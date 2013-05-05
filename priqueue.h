// based on implementation of priqueue in "Programming Pearls" by
// Jon Bentley.
// extends to provide O(log n) decrease key operation, using a
// handle to index into the elements current heap position.

#include <assert.h>

template <class T, class C>
class priqueue {
private:
  C n, maxsize;
  struct arrType {
    T x;
    C ridx;
  } *arr;
  C *idx; // handle, points to current position an element in heap

  template <class X>
  void swap(C i, C j, X *a) { X t=a[i]; a[i]=a[j]; a[j]=t; }

  void swapEntries(int i, int j) {
    swap(arr[i].ridx, arr[j].ridx, idx);
    swap(i, j, arr);
  }

  void siftdown(int l, int u) {
    C i, c;
    for (i=l; (c=2*i) <= u; i=c) {
      if (c+1<=n && (arr[c+1].x < arr[c].x))
	c++;
      if (arr[i].x <= arr[c].x) 
	break;
      swapEntries(i, c);
    }
  }

  void siftup(int u) {
    C i, p;
    for (i=u; i>1 && arr[p=i/2].x > arr[i].x; i=p)
      swapEntries(i, p);
  }

public:
  priqueue(C s) : n(0), maxsize(s){
    arr =  new arrType[maxsize+1];
    idx = new C[maxsize+1];
  }
  ~priqueue() { delete arr; delete idx; }

  void heapify() {
    for(C i=n; i>0; i--) siftdown(i, n);
  }

  template <class Iterator>
  void set_priqueue(Iterator first, Iterator last) {
    n = 0;
    for ( ; first != last; first++) {
      n++; idx[n]=n;
      arr[n].x = *first;
      arr[n].ridx=n;
    }
    heapify();
  }

  void insert(T t) {
    n++; idx[n]=n;
    arr[n].x=t; arr[n].ridx=n;
    siftup(n);
  }

  T popmin() {
    T t = arr[1].x;
    idx[arr[n].ridx] = 1;
    arr[1] = arr[n--];
    siftdown(1, n);
    return t;
  }

  T get_key_entry(C k) { 
    return arr[idx[k]].x;
  }

  void update_dec_key(C k) {
    siftup(idx[k]);
  }

  bool empty() { 
    return n==0; 
  }

  void assertq() {
    for (C i=2; i<n; i++) assert(arr[i/2].x < arr[i].x);
  }
};

