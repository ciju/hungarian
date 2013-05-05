// Minimum weight perfect matching algorithm,
// from "Algorithm Design" - Kleinberg, Tardos 
#include <string>
#include <vector>
#include <iterator>
#include <map>
#include <queue>
#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "priqueue.h"
#include "indexedList.h"

using namespace std;

const int INF = INT_MAX;
typedef unsigned int Count;
typedef unsigned int Name;
typedef int Cost;

class Edge;

class SP {
private:
  friend class Edge;
  friend class Node;
  friend class NodeP;
  friend class Digraph;
  friend class BiDigraph;
  Cost price;
  Cost dist;
  bool done;
  Count key;
  Edge *prev;
public:
  void reset() { dist=INF; done=false; prev=NULL; }
};

typedef indexedList<Edge *>::key ILKey; // indexed edge list
typedef indexedList<Edge *>::iterator ILIterator;

class Node {
private:
  indexedList<Edge *> edges;
  Name name;
public:
  SP sp;
  Node(Name s) : name(s) { }

  Name get_name() { return name; }

  void add_in_edge(Edge *e);
  void add_out_edge(Edge *e);
  void rem_edge(ILKey k) { edges.remove(k); }
  void reverse_dir(ILKey k) { edges.change_list(k); }
  
  ILIterator out_begin() { return edges.begin_first(); }
  ILIterator out_end() { return edges.end_first(); }
  ILIterator in_begin() { return edges.begin_second(); }
  ILIterator in_end() { return edges.end_second(); }
};

class NodeP {
private:
  Node *n;
public:
  NodeP() {}
  NodeP(Node *nd) :n(nd) { }
  NodeP(Name name) { n = new Node(name);}
  bool operator>= (NodeP p) {return n->sp.dist >= p->sp.dist;}
  bool operator<= (NodeP p) {return n->sp.dist <= p->sp.dist;}
  bool operator> (NodeP p) {return n->sp.dist > p->sp.dist;}
  bool operator< (NodeP p) {return n->sp.dist < p->sp.dist;}
  Node* operator-> () const {return n;}
};

class Edge {
private:
  Cost cost;
  bool back_edge;
  Node *head;
  Node *tail;
  ILKey hkey;
  ILKey tkey;
public:
  Edge(Node *x, Node *y, Cost c) 
    : cost(c), back_edge(false), head(y), tail(x) { }

  Cost get_cost() { return cost; }
  Cost get_red_cost() { return tail->sp.price + cost - head->sp.price; }
  bool is_back_edge() { return back_edge; }
  Node* get_head() { return head; }
  Node* get_tail() { return tail; }

  // key for head node of edge
  ILKey get_hkey() { return hkey; }
  void set_hkey(ILKey k) { hkey = k; }
  // key for tail node of edge
  ILKey get_tkey() { return tkey; }
  void set_tkey(ILKey k) { tkey = k; }

  void remove() {
    head->rem_edge(hkey);
    tail->rem_edge(tkey);
  }

  void reverse_dir() {
    head->reverse_dir(hkey);
    tail->reverse_dir(tkey);
    cost = -cost;
    back_edge = !back_edge;
    Node *tmp = head;
    head = tail;
    tail = tmp;
  }

  // used in shortest path calculation
  // NOTE: uses reduced cost of edge, to caculate shortest dist.
  void relax_red() {
    Cost newc = tail->sp.dist + get_red_cost();
    Cost prec = head->sp.dist;
    if (newc <= prec) {
      head->sp.dist = newc;
      head->sp.prev = this;
    }
  }
};
void Node::add_in_edge(Edge *e)  {e->set_hkey(edges.insert(e, 0));}
void Node::add_out_edge(Edge *e) {e->set_tkey(edges.insert(e, 1));}

class Digraph {
private:
  vector<NodeP> nodes;
public:
  Node *add_node(Name name) {
    Node *n = new Node(name);
    nodes.push_back((Node *)n);
    return n;
  }

  void add_edge(Node *x, Node *y, Cost c) {
    Edge *e = new Edge(x, y, c);
    x->add_out_edge(e);
    y->add_in_edge(e);
  }

  static void init_node(NodeP n) { n->sp.reset(); }
  typedef priqueue<NodeP, Cost> NodePriQ;
  void dijkstra_spath(NodeP s) {
    Count i;
    NodePriQ *q;
    vector<NodeP>::iterator nit;

    q = new NodePriQ(nodes.size());

    // priqueue handles start from 1
    for (i=1, nit=nodes.begin(); nit<nodes.end(); nit++, i++) {
      init_node(*nit);
      (*nit)->sp.key = i;       // priorityQ handle
    }

    s->sp.dist = 0;
    q->set_priqueue(nodes.begin(), nodes.end());

    ILIterator it;
    while (!q->empty()) {
      NodeP n = q->popmin();
      n->sp.done=true;
      
      for (it=n->out_begin(); it!=n->out_end(); ++it) {
	if ( ! (*it)->get_head()->sp.done ) {
	  (*it)->relax_red();   // relax edges based on reduced cost of edges.
	  q->update_dec_key( (*it)->get_head()->sp.key );
	}
      }
    }
  }

};

enum NodeType {SourceNode, TargetNode, XNode, YNode};
class BiDigraph : public Digraph {
private:
  Node *s;
  Node *t;
  vector<Node *> x;
  vector<Node *> y;
public:
  Node* add_node(Name name, NodeType nt) {
    Node *n = Digraph::add_node(name);
    switch (nt) {
    case XNode: x.push_back(n); break;
    case YNode: y.push_back(n); break;
    case SourceNode: s = n; break;
    case TargetNode: t = n; break;
    }
    return n;
  }

  void initiate_node_prices() {
    s->sp.price = t->sp.price = 0;
    vector<Node *>::iterator it;

    for (it=x.begin(); it!=x.end(); ++it)
      (*it)->sp.price = 0;

    for (it=y.begin(); it!=y.end(); ++it) {
      Node *n = *it;
      Edge *e = *min_element(n->in_begin(), n->in_end());
      n->sp.price = e->get_cost();
    }
  }

  static void reduce_price(Node *n) {
    n->sp.price = n->sp.dist + n->sp.price;
  }
  void reduce_node_prices() {
    for_each(x.begin(), x.end(), reduce_price);
    for_each(y.begin(), y.end(), reduce_price);
  }
    
  void augment_spath() {
    Cost minc=INF;
    Edge *e = *t->in_begin();

    // remove edge to t in spath, from graph. 
    ILIterator it;
    for (it=t->in_begin(); it!=t->in_end(); ++it) {
      Node *n = (*it)->get_tail();
      // c is also calculated by reduced_node_prices, so if
      // reduced_node_prices is called earlier, c should be
      // initialized with n->sp.price
      Cost c = n->sp.dist + n->sp.price;
      if (c < minc) {
	minc = c;
	e = *it;
      }
    }
    e->remove();                //reverse should also work.

    // switch the rest of the path, except the edge from s
    Node *tmp;
    e = e->get_tail()->sp.prev;
    for ( ; (tmp=e->get_tail())!=s; e=tmp->sp.prev) {
      e->reverse_dir();	// use tmp, as tail doesnt remain same after rev
    }

    // remove edge out of s in the path, from graph, although
    // reverse should also work
    e->remove();
  }

  void min_weight_assignment() {
    initiate_node_prices();
    
    for (int i=0; i<(int)x.size(); i++) {
      dijkstra_spath(s);
      augment_spath();
      reduce_node_prices();
    }

    print_mapping();
  }

  void print_mapping() {
    Cost tcost=0;
    vector<Node *>::iterator it;
    vector<Name> v;
    
    for (it=y.begin(); it!=y.end(); ++it) 
      tcost += -((*(*it)->out_begin())->get_cost());
    cout << tcost << endl;

    for (it=y.begin(); it!=y.end(); ++it) {
      printf("(%d,%d) ", (*(*it)->out_begin())->get_head()->get_name()
             , (*it)->get_name());
    }
    cout << endl;
  }

};

BiDigraph* prepare_graph(char *name) {
  FILE *f = fopen(name, "r");
  Cost cost;
  Name x, y;
  BiDigraph *g = new BiDigraph();
  map<Name, Node*> lstx, lsty;

  // <node> <node> <edgecost>, where node is represented by int
  while (fscanf(f, "%d %d %d\n", &x, &y, &cost) != EOF) {
    Node *xnode, *ynode;
    if ( lstx.find(x) == lstx.end() )
      lstx[x] = xnode = g->add_node(x, XNode);
    if ( lsty.find(y) == lsty.end() )
      lsty[y] = ynode = g->add_node(y, YNode);
    xnode = lstx[x];
    ynode = lsty[y];

    g->add_edge(xnode, ynode, cost);
  }

  Node *s = g->add_node(0, SourceNode);
  Node *t = g->add_node(0, TargetNode);
  map<Name, Node*>::iterator it;
  for (it=lstx.begin(); it!=lstx.end(); it++)
    g->add_edge(s, it->second, 0);
  for (it=lsty.begin(); it!=lsty.end(); it++)
    g->add_edge(it->second, t, 0);

  fclose(f);

  return g;
}

int main(int argc, char *argv[]) {
  if (argc!=2) {
    cout << "input file missing\n";
    exit(0);
  }
  
  BiDigraph *g;
  
  g = prepare_graph(argv[1]);
  g->min_weight_assignment();

  return 0;
}
