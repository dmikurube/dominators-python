#include "dgraph.h"

/*--------------------------------------
 | iterative dominators algorithm
 | - dominators initalized with zero
 *--------------------------------------*/

static int readPostDFS(const char* postorder_filename,
                       int* post2label,
                       int* label2post) {
  int src;
  FILE* input = fopen (postorder_filename, "r");
  if (!input) {
    fprintf (stderr, "Error opening file \"%s\".\n", postorder_filename);
    exit(-1);
  }

  int n;
  if (fscanf(input,"postorder %d %d\n", &n, &src) != 2) {
    fprintf (stderr, "Error reading graph size (%s).\n", postorder_filename);
    exit (-1);
  }

  while (1) {
    int post, ord;
    if (fscanf(input, "%d %d\n", &post, &ord)!=2)
      break; //arc from a to b
    post2label[post] = ord;
    label2post[ord] = post;
  }
  fclose (input);

  return n;
}

static int readPreDFS(const char* preorder_filename,
                       int* pre2label,
                       int* label2pre) {
  int src;
  FILE* input = fopen (preorder_filename, "r");
  if (!input) {
    fprintf (stderr, "Error opening file \"%s\".\n", preorder_filename);
    exit(-1);
  }

  int n;
  if (fscanf(input,"preorder %d %d\n", &n, &src) != 2) {
    fprintf (stderr, "Error reading graph size (%s).\n", preorder_filename);
    exit (-1);
  }

  while (1) {
    int pre, ord;
    if (fscanf(input, "%d %d\n", &pre, &ord)!=2)
      break; //arc from a to b
    pre2label[pre] = ord;
    label2pre[ord] = pre;
  }
  fclose (input);

  return n;
}

void DominatorGraph::idfs (int r, int *idom) {
  int v, i, new_idom, N;
  int bsize = n+1;
  int *buffer = new int [2*bsize];
  int *post2label = &buffer[0]; //post-dfs ids to original label
  int *dom = &buffer[bsize];    //dominators (indexed by post-ids)

  resetcounters();

  int *label2post = idom; //idom will not be used until later
  // N = postDFS (r, label2post, post2label); //get post-ids
  N = readPostDFS("data.dimacs.postorder", post2label, label2post);
  bool changed;

  for (v=n; v>=0; v--) dom[v] = 0;
  dom[N] = N;

  /*-----------
   | main loop
   *----------*/
  do {
    inci(); //increment number of iterations (operation count)
    changed = false;

    for (i=N-1; i>0; i--) { //reverse post-order
      new_idom = 0; //using dom[i] is not faster

      /*----------------------------------------------------
       | for each incoming arc (v,w), compute nca between v
       | and the current candidate dominator of w
       *---------------------------------------------------*/
      int *p, *stop;
      getInBounds (post2label[i], p, stop);
      for (; p<stop; p++) {
        int v = label2post[*p]; //v is the source of the arc
        incc();
        if (dom[v]) {           //find nca between current dom and v
          new_idom = (new_idom ? intersect(v,new_idom,dom) : v);
          incc();
        }
      }

      /*-------------------------------------------------------
       | if new dominator found, update dom and mark as changed
       *------------------------------------------------------*/
      incc();
      if (new_idom > dom[i]) {
        dom[i] = new_idom;
        changed = true;
      }
    }
  } while (changed);

  /*-----------------------------------------------------------
   | restore idoms: unreachable nodes are already zero because
   | array is shared with label2post
   *----------------------------------------------------------*/
  idom[r] = r;
  for (i=N-1; i>0; i--) idom[post2label[i]] = post2label[dom[i]];

  delete [] buffer;
}


/*-------------------------------------------------
 | iterative dominators algorithm
 | - dominators initialized with parent in bfs
 | - vertices visited in direct pre-order
 *------------------------------------------------*/

void DominatorGraph::ibfs (int r, int *idom) {
  int bsize = n+1;
  int *buffer = new int [2*bsize];
  int *pre2label = &buffer[0];
  int *dom       = &buffer[bsize];
  int *label2pre = idom;          //indexed by label
  resetcounters();

  //find pre-ids, initialize dom with parents in BFS tree
  int N;
  N = preBFSp (r, label2pre, pre2label, dom);

  bool changed = true;

  while (changed) {
    inci(); //increment iteration counter
    changed = false;

    // process vertices in preorder
    for (int i=2; i<=N; i++) {
      int new_idom = dom[i];

      /*----------------------------------------------------
       | for each incoming arc (v,w), compute nca between v
       | and the current candidate dominator of v
       *---------------------------------------------------*/
      int *p, *stop;
      getInBounds (pre2label[i], p, stop);
      for (; p<stop; p++) {
        int v = label2pre[*p];
        incc();
        if (v) new_idom = preIntersect (v, new_idom, dom);
      }

      /*-----------------------------------------------------------
       | if we new dominator found, update dom and mark as changed
       *----------------------------------------------------------*/
      incc();
      if (new_idom!=dom[i]) {
        dom[i] = new_idom;
        changed = true;
      }
    }
  }

  //get dominators
  for (int i=N; i>0; i--) idom[pre2label[i]] = pre2label[dom[i]];
  delete [] buffer;
}
