#include "dgraph.h"

/*---------------------------------------------------------------
 | SEMI-NCA (snca): two-phase algorithm:
 | 1. computes semidominators as in Simple Lengauer-Tarjan (SLT)
 | 2. builds the dominator tree incrementally
 |
 | Notes:
 | - parent and ancestor share an array
 | - recursive compress
 *--------------------------------------------------------------*/

/*
inline void rcompress (int v, int *parent, int *label, int c) {
  incc();
  int p;
  if ((p=parent[v])>c) {
    rcompress (p, parent, label, c); //does not change parent[v]
    incc();
    if (label[p]<label[v]) label[v] = label[p];
    parent[v] = parent[p];
  }
}
*/

int readDFS(const char* parents_filename,
            const char* preorder_filename,
            int* parent,
            int* pre2label,
            int* label2pre) {
  FILE *input = fopen (parents_filename, "r");
  if (!input) {
    fprintf (stderr, "Error opening file \"%s\".\n", parents_filename);
    exit(-1);
  }

  int n, src;
  if (fscanf(input,"parents %d %d\n", &n, &src) != 2) {
    fprintf (stderr, "Error reading graph size (%s).\n", parents_filename);
    exit (-1);
  }
  parent[src] = 0;

  while (1) {
    int node, p;
    if (fscanf(input, "%d %d\n", &node, &p)!=2)
      break; //arc from a to b
    parent[node] = p;
  }
  fclose (input);

  input = fopen (preorder_filename, "r");
  if (!input) {
    fprintf (stderr, "Error opening file \"%s\".\n", preorder_filename);
    exit(-1);
  }

  int n2;
  if (fscanf(input,"preorder %d %d\n", &n2, &src) != 2) {
    fprintf (stderr, "Error reading graph size (%s).\n", preorder_filename);
    exit (-1);
  }
  if (n != n2) {
    fprintf (stderr, "#nodes differ.\n");
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

void DominatorGraph::snca (int r, int *idom) {
        int bsize = n+1;
        int *buffer    = new int [5*bsize];
        int *dom       = &buffer[0*bsize]; //not shared
        int *pre2label = &buffer[1*bsize];
        int *parent    = &buffer[2*bsize]; //shared with ancestor
        int *label     = &buffer[3*bsize];
        int *semi      = &buffer[4*bsize];

        int *label2pre = idom;          //indexed by label

        resetcounters();

        //initialize semi and label
        int i;
        for (i=n; i>=0; i--) label[i] = semi[i] = i;

        int N;
        N = preDFSp(r, label2pre, pre2label, parent);
        for (i=0; i<10; i++)
          printf("%d: %d\n", i, pre2label[i]);
        printf("%d, %d\n", N, parent[r]);
        N = readDFS("data.dimacs.parents", "data.dimacs.preorder", parent, pre2label, label2pre);
        printf("%d, %d, %d, %d\n", N, parent[r], pre2label[0], label2pre[0]);

        /*----------------
         | semidominators
         *---------------*/
        for (i=N; i>1; i--) {
                int *p, *stop;
                dom[i] = parent[i]; //can't put dom and parent together

                //process each incoming arc
                getInBounds (pre2label[i], p, stop);
                for (; p<stop; p++) {
                        int v = label2pre[*p];
                        if (v) {
                                int u;
                                incc();
                                if (v<=i) {u=v;} //v is an ancestor of i
                                else {
                                        rcompress (v, parent, label, i);
                                        u = label[v];
                                }
                                incc();
                                if (semi[u]<semi[i]) semi[i] = semi[u];
                        }
                }
                label[i] = semi[i];
        }
        printf("%d: %d\n", 1, pre2label[1]);
        printf("root: %d\n", r);

        /*-----------------------------------------------------------
         | compute dominators using idom[w]=NCA(I,parent[w],sdom[w])
         *----------------------------------------------------------*/
        dom[1] = 1;
        idom[r] = r;
        for (i=2; i<=N; i++) {
                int j = dom[i];
                while (j>semi[i]) {j=dom[j]; incc();}
                incc();
                dom[i] = j;
                idom[pre2label[i]] = pre2label[dom[i]];
        }

        //cleanup stuff
        delete [] buffer;
}
