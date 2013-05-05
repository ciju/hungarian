// a datastructure to handle the path augmentations in bi-partite
// graphs.

// essentially, two lists. One could insert element into any
// list. Could remove element by providing its handle(ex: pointer
// to ll<T>, no list informatin is needed). Could switch an
// element from one list to the other using CHANGE_LIST.

#include <vector>
#include <iostream>

using namespace std;

template <class T>
class ll {
public:
  T val;
  ll *head;
  ll *next, *prev;
  ll() : head(NULL), next(NULL), prev(NULL) {}
  ll(T v) : val(v), next(NULL), prev(NULL) {}

  void insert(ll *n) {
    n->next = next;
    n->prev = this;
    next = n;
    n->head = head;
    if (n->next != NULL) 
      n->next->prev = n;
  }
  void detach() {
    prev->next = next;
    if (next != NULL)
      next->prev = prev;
  }
};

template <class T>
class idxLst_iterator : public iterator<forward_iterator_tag, T> {
  friend class ll<T>;
  
  ll<T> *lp;
  
public:
  idxLst_iterator() {}
  idxLst_iterator(ll<T> *l) : lp(l) {}

  T& operator* () const { 
    return lp->val; 
  }
  idxLst_iterator& operator++() {
    lp = lp->next;
    return *this;
  }
  bool operator== (const idxLst_iterator<T>& right) const {
    return this->lp == right.lp;
  }
  bool operator!= (const idxLst_iterator<T>& right) const {
    return this->lp != right.lp;
  }
};

template <class T>
class indexedList {
private:
  ll<T> first;
  ll<T> second;
public:
  typedef idxLst_iterator<T> iterator;
  typedef ll<T> *key;

  indexedList() { first.head=&first; second.head=&second; }

  ll<T>* insert(T val, bool outedge=true) {
    ll<T> *n = new ll<T>(val);
    if (outedge) first.insert(n);
    else second.insert(n);
    return n;
  }

  void change_list(ll<T> *l) {
    l->detach();
    if (l->head == &first)
      second.insert(l);
    else
      first.insert(l);
  }

  void remove(ll<T> *l) {
    l->detach();
  }

  iterator begin(ll<T> &lst) { return idxLst_iterator<T>(lst.next); }
  iterator end() { return idxLst_iterator<T>(NULL); }
  iterator begin_first() { return begin(first); }
  iterator begin_second() { return begin(second); }
  iterator end_first() { return end(); }
  iterator end_second() { return end(); }
};

