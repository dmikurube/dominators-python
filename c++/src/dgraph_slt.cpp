#include "dgraph.h"

/*--------------------------------------------------------
 | Simple Lenguauer-Tarjan algorithm (SLT)
 | - ancestor and parent share an array
 | - recursive compress
 | - bucket processed at the beginning of each iteration
 | - vertex v not inserted in bucket if semi[v]==parent[v]
 *--------------------------------------------------------*/

static int _readDFS(const char* parents_filename,
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

void DominatorGraph::slt (int r, int *idom) {
	int bsize = n+1;
	int *buffer    = new int [6*bsize];
	int *pre2label = &buffer[0];
	int *parent    = &buffer[bsize];
	int *semi      = &buffer[2*bsize];
	int *label     = &buffer[3*bsize];
	int *dom       = &buffer[4*bsize];
	int *ubucket   = &buffer[5*bsize];

	int *label2pre = idom;          //indexed by label

	resetcounters();

	int i;
	for (i=n; i>=0; i--) {
		label[i] = semi[i] = i;
		ubucket[i] = 0;
	}

	//pre-dfs
	int N;
        N = preDFSp (r, label2pre, pre2label, parent);
        N = _readDFS("data.dimacs.parents", "data.dimacs.preorder", parent, pre2label, label2pre);

	// process the vertices in reverse preorder 
	for (i=N; i>1; i--) {
		/*--------------------- 
		 | process i-th bucket
		 *--------------------*/
		for (int v=ubucket[i]; v; v=ubucket[v]) {
			rcompress (v, parent, semi, label, i);
			int u = label[v];
			incc();
			dom[v] = (semi[u]<semi[v]) ? u : i;
		}
		//no need to empty the bucket

		/*---------------------------------------------
		 | check incoming arcs, update semi-dominators
		 *--------------------------------------------*/
		int *p, *stop;
		getInBounds (pre2label[i], p, stop);
		for (; p<stop; p++) {
			int v = label2pre[*p];
			incc();
			if (v) {
				int u; 
				incc();
				if (v<=i) {u=v;} //v is an ancestor of i
				else {
					rcompress (v, parent, semi, label, i);
					u = label[v];
				}
				incc();
				if (semi[u]<semi[i]) semi[i] = semi[u];
			}
		}

		/*---------------------------
		 | process candidate semidom
		 *--------------------------*/
		int s = semi[i];
		incc();
		if (s!=parent[i]) { //if semidominator n not parent: add i to s's bucket
			ubucket[i] = ubucket[s]; 
			ubucket[s] = i;
		} else {
			dom[i] = s; //semidominator is parent: s is a candidate dominator
		}
	}

	/*------------------
	 | process bucket 1
	 *-----------------*/
	for (int v=ubucket[1]; v; v=ubucket[v]) dom[v]=1;

	/*---------------
	 | recover idoms 
	 *--------------*/
	dom[1] = 1;
	idom[r] = r;
	for (i=2; i<=N; i++) {
		incc();
		if (dom[i]!=semi[i]) dom[i]=dom[dom[i]]; //make relative absolute
		idom[pre2label[i]] = pre2label[dom[i]];
   	}

	delete [] buffer; //cleanup stuff
}
