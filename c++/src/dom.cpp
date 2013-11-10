/* Time various implementations of the following
   dominators algorithms:
   - Iterative algorithm
   - Lengauer-Tarjan 
   - SDOM-NCA */

#include "dgraph.h"
#include "rfw_timer.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int MINTIME = 1;

/*----------------------------------------------------------------
 | define methods and method names: make sure they are consistent
 *---------------------------------------------------------------*/
typedef enum {
	BFS, 
	DFS, 
	SDOM, 
	IBFS, IDFS, 
	LT,
	SLT,
	SNCA,
	METHODS
} Method;

const char *mnames[METHODS]= {
	"bfs", 
	"dfs", 
	"sdom", 
	"ibfs", "idfs", 
	"lt",
	"slt",
	"snca", 
};



/*------------------------------------
 | function for simple error messages 
 *-----------------------------------*/

void fatal (const char *msg) {
	fprintf (stderr, "ERROR: %s.\n", msg);
	exit(-1);
}

void printBasics (FILE *file) {
	fprintf (file, "version 04112401\n");
#ifdef COUNTOPS
	fprintf (file, "counting 1\n");
#else
	fprintf (file, "counting 0\n");
#endif
}


void printUsage(const char *command) {
	fprintf(stderr, "Usage: %s <input file> <method> [-reverse] [-simplify] [mintime]\n", command);
	fprintf(stderr, "Methods: ");
	for (int i=0; i<METHODS; i++) {
		fprintf (stderr, " %s", mnames[i]);
	}
	fprintf (stderr, "\n\n");
	exit(-1);
}

/*-----------------------
 | run an algorithm once
 *----------------------*/

inline void run (Method method, DominatorGraph *g, int r, int *idom) {
	switch (method) {
		//main methods
		case IBFS: g->ibfs (r, idom); break;
		case IDFS: g->idfs (r, idom); break;
		case SLT:  g->slt  (r, idom); break;
		case LT:   g->lt   (r, idom); break;
		case SNCA: g->snca (r, idom); break;

		//auxiliary functions
		case DFS:  g->run_dfs(r); break;
		case BFS:  g->run_bfs(r); break;
		case SDOM: g->semi_dominators(r); break;
	
		default: break;
	}
}


/*-----------------------------------------------
 | compare two arrays, return true iff identical
 *----------------------------------------------*/

inline bool compare (int n, int *a, int *b, bool verbose) {
	bool valid = true;
	for (int i=1; i<=n; i++) {
		if (a[i]!=b[i]) {
			valid = false;
			if (verbose) fprintf (stderr, "%d(%d,%d) ", i, a[i], b[i]);
		}
	}
	return valid;
}


/*-------------------------------------------------------
 | checks whether all algorithms produce the same result
 *------------------------------------------------------*/

inline bool check (DominatorGraph *g, int r, bool verbose = false) {
	int n = g->getNVertices();
	int *ref = new int [n+1];
	int *idom = new int [n+1];
	bool passed = true;
	int count = 0;

	for (Method m=IBFS; m<METHODS; m = (Method)((int)m+1)) {
		count ++;
		if (m==IBFS) {
			if (verbose) fprintf (stderr, "Running reference method (%s)... ", mnames[m]);
			run (IDFS, g, r, ref);
			if (verbose) fprintf (stderr, "done.\n");
			continue;
		} else {
			if (verbose) fprintf (stderr, "Checking %s... ", mnames[m]);
			for (int i=1; i<=n; i++) idom[i] = n+(int)m+i; //makes sure idoms have weird values
			run (m, g, r, idom);
			bool valid = compare (n, ref, idom, verbose);
			if (verbose) {
				if (valid) fprintf (stderr, "passed.\n");
				else fprintf (stderr, "FAILED.\n");
			}
			passed = passed && valid;
		}
	}

	delete [] ref;
	delete [] idom;

	if (passed) fprintf (stderr, "Tested %d methods: PASSED.\n", count);

	return passed;
}


/*-----------------------------------
 | get method code based on its name 
 *----------------------------------*/

Method getMethod (const char *name) {
	Method method = METHODS; //dummy initialization, or else compiler complains
	int i;
	for (i=0; i<METHODS; i++) { //check if there is a match
		if (strcmp(name, mnames[i])==0) {method = (Method)i; break;}
	} 
	return method;
}


/*----------------------
 | print list of graphs 
 *---------------------*/

void printList (const char *listname, int *marked, bool reverse, bool simplify) {
	int count = 0;
	FILE *input = fopen (listname, "r");
	fprintf (stderr, "Marked graphs:\n");
	
	if (!input) {
		fprintf (stderr, "Error opening file %s.\n", listname);
		exit(-1);
	}

	DominatorGraph graph;
	char buffer[1024];
	while (fscanf(input, "%s", buffer)==1) {
		graph.destroy();
		graph.readDimacs(buffer,reverse, simplify);
		if (graph.getSource()!=0) {
			if (marked[count]>2) fprintf (stdout, "%d %s\n", marked[count], buffer);
			count++;
		}
	}
	fclose(input);
}

/*----------------------------------------------------------------------------------
 | If glist is not NULL: read all graphs in list name and initialize glist with
 | pointers to the valid ones (graphs will be reversed or simplified if necessary).
 |
 | If glist is NULL: reads all graphs in the list, returns the number of valid 
 | ones.
 |
 | In both cases, a graph will be considered valid iff it has a source (after
 | being reversed, if reverse is true).
 *---------------------------------------------------------------------------------*/

int readList (const char *listname, DominatorGraph *glist, bool reverse, bool simplify) {
	int count = 0, ignored = 0;
	FILE *input = fopen (listname, "r");
	if (!input) {
		fprintf (stderr, "Error opening file \"%s\".\n", listname);
		exit(-1);
	}
	
	char buffer[1024];
	DominatorGraph graph;
	while (fscanf (input, "%s", buffer)==1) {
		//fprintf (stderr, "<%s>\n", buffer);
		if (glist) {
			glist[count].destroy();
			glist[count].readDimacs(buffer,reverse, simplify);
			if (glist[count].getSource()!=0) count++;
			else ignored ++;
		} else {
			graph.destroy();
			graph.readDimacs(buffer,reverse,simplify);
			if (graph.getSource()!=0) count++;
			else ignored ++;
		}
	}
	fclose (input);
	return count;
}


/*-----------------------------------------------------------------
 | Returns a list of pointers to all valid graphs in "listname". 
 | Also initializes 'count' (number of valid graphs in the list---
 | those with a source) and 'maxn' (maximum number of vertices 
 | among all valid graphs).
 *----------------------------------------------------------------*/

DominatorGraph *createGraphList (const char *listname, bool reverse, int &count, int &maxn, bool simplify) {
	//just count number of vertices
	count = readList (listname, NULL, reverse, simplify);
	fprintf (stderr, "Reading %d graphs... ", count);
	
	//actually build graphs and put them in the list
	DominatorGraph *glist = new DominatorGraph [count+1];
	readList (listname, glist, reverse, simplify);
	fprintf (stderr, "done.\n");
	
	maxn = 0;
	for (int g=0; g<count; g++) {
		if (glist[g].getNVertices() > maxn) maxn = glist[g].getNVertices();
	}


	return glist;
}


/*-----------------------------------------------------------------
 | check all algorithms on all graphs in a given series; aborts if 
 | some discrepancy---a different set of dominators---is found)
 *----------------------------------------------------------------*/

void checkSeries(const char *listname, bool reverse, bool simplify) {
	int count, maxn;
	DominatorGraph *glist = createGraphList (listname, reverse, count, maxn, simplify);

	for (int g=0; g<count; g++) {
		DominatorGraph *graph = &glist[g];
		int r0 = graph->getSource();
		int n = graph->getNVertices();
		fprintf (stderr, "Checking graph %d (%d vertices)... ", g+1, n);

		int r = r0;
		bool passed = check(graph, r, false);
		if (!passed) {
			fprintf (stderr, "FAILED!\n");
			fprintf (stderr, "Here's how they differ:\n\n");
			check(graph, r, true);
			exit (-1);
		} 
		fprintf (stderr, "passed.\n");
	}
}


/*-------------------------------------------------------
 | run a particular method on all graphs in a given list
 *------------------------------------------------------*/

void runSeries (const char *listname, Method method, bool reverse, bool simplify) {
	const bool dump_violators = false;
	
	int count, maxn;
	DominatorGraph *glist = createGraphList (listname, reverse, count, maxn, simplify);
	int *idom = new int [maxn+1];

	int runs = 0;
	double t = 0;
	RFWTimer timer(true);
	do {
		runs ++;
		for (int g=0; g<count; g++) {
			DominatorGraph *graph = &glist[g];
			int r = graph->getSource();
			run (method, graph, r, idom);
		}
	} while ((t=timer.getTime()) < MINTIME);


	/*---
	 | get data gathered by all runs
	 *--*/
	int vsum, asum; //vertices, arcs, size
	double dsum, itsum, ops, opsv, sp, spf;
	int *marked = new int[count];
	vsum = asum = 0;
	dsum = ops = opsv = itsum = sp = spf = 0.0;
	for (int g=0; g<count; g++) {
		DominatorGraph *graph = &glist[g];
		int n = graph->getNVertices();
		int m = graph->getNArcs();
		vsum += n; //vertices
		asum += m; //arcs
		dsum += (double)m/(double)n; //density
		ops += graph->ccount;
		opsv += graph->ccount / (double)n;

		if (dump_violators) {
			int itc = graph->icount;
			if (itc>2) {
				fprintf (stderr, "Graph %d has %d iterations.\n", g, itc);
			}
			marked[g] = (itc);
		}

		itsum += graph->icount;
		sp += graph->scount;
		if (n>1) {
			spf += (int) graph->scount / (double)(n-1);
		}
		//itvsum += (double)graph->icount / (double)n;
	}

	if (dump_violators) printList (listname, marked, reverse, simplify);
	delete [] marked;

	double avg = t / (double)runs;
	fprintf (stdout, "method %s\n", mnames[method]);
	fprintf (stdout, "reverse %d\n", (int)reverse);	
	fprintf (stdout, "series %s\n", listname);
	fprintf (stdout, "runs %d\n", runs);
	fprintf (stdout, "graphs %d\n", count);
	fprintf (stdout, "totaltime %.8f\n", t);
	fprintf (stdout, "avgtime %.8f\n", avg);
	fprintf (stdout, "avgtimem %.8f\n", 1000.0 * avg);
	fprintf (stdout, "avgtimeu %.8f\n", 1000000.0 * avg);
	fprintf (stdout, "gtimeu %.8f\n", 1000000.0 * avg / (count));
	fprintf (stdout, "vtimeu %.8f\n", 1000000.0 * avg / (vsum));
	fprintf (stdout, "atimeu %.8f\n", 1000000.0 * avg / (asum));
	fprintf (stdout, "stimeu %.8f\n", 1000000.0 * avg / (asum+vsum));

	fprintf (stdout, "simplified %d\n", (int)simplify);

	fprintf (stdout, "totals %d\n", asum + vsum);
	fprintf (stdout, "avgs %.8f\n", (double)(asum+vsum)/(double)count);
	fprintf (stdout, "totalv %d\n", vsum);
	fprintf (stdout, "avgv %.8f\n", (double)vsum/(double)count);
	fprintf (stdout, "totala %d\n", asum);
	fprintf (stdout, "avga %.8f\n", (double)asum/(double)count);
	fprintf (stdout, "totald %.8f\n", dsum);
	fprintf (stdout, "avgd %.8f\n", (double)dsum/(double)count);
	fprintf (stdout, "ops %.0f\n", ops);
	fprintf (stdout, "opsg %.8f\n", ops/(double)count);
	fprintf (stdout, "opsv %.8f\n", ops/(double)vsum);
	fprintf (stdout, "aopsv %.8f\n", opsv/(double)count);
	fprintf (stdout, "sp %.0f\n", sp);
	fprintf (stdout, "spa %.8f\n", sp/(double)vsum);
	fprintf (stdout, "spf %.8f\n", spf/(double)count);
	fprintf (stdout, "itcount %.0f\n", itsum);
	fprintf (stdout, "itcountg %.8f\n", (double)itsum / (double)count);
	//fprintf (stderr, "itcountv %.8f\n", (double)itsum / (double)vsum);
	//fprintf (stderr, "aitcountv %.8f\n", (double)itvsum / (double)count);

	


	delete [] idom;
	delete [] glist;

}


/*----------------------------------
 | run all tests for a given graph 
 *---------------------------------*/

void runTests (const char *filename, Method method, bool reverse, bool simplify, FILE *idomfile = NULL) {

	/*----------------
	 | read the graph 
	 *---------------*/
	DominatorGraph g;
	g.readDimacs(filename, reverse, simplify); //WARNING: MAKE SURE REVERSE IS INTERPRETED CORRECTLY
	int r = g.getSource();

	/*---------------------------------
	 | run the algorithm several times
	 *--------------------------------*/
	int *idom = new int [g.getNVertices()+1];
	int inner = 100000/g.getNVertices() + 1;
	if (MINTIME < 1) inner = 1;
	int runs = 0;

	RFWTimer timer(true);
	double t;
	do {
		for (int i=inner; i>0; i--) {
			runs ++;
			run (method, &g, r, idom);
		}
	} while ((t=timer.getTime()) < MINTIME);

	if (idomfile) {
		for (int i=1; i<=g.getNVertices(); i++) {
			fprintf (idomfile, "%d %d\n", i, idom[i]);
		}
	}

	delete [] idom;


	/*-------------------
	 | output statistics 
	 *------------------*/
	
	//graph characteristics
	fprintf (stdout, "filename %s\n", filename);
	g.outputGraphStatistics (stdout); //vertices, edges, size, density...

	//input parameters
	fprintf (stdout, "method %s\n", mnames[method]);
	fprintf (stdout, "reverse %d\n", (int)reverse);
	fprintf (stdout, "simplifed %d\n", (int)simplify);


	//running time measures
	double avg = t / (double)runs;
	fprintf (stdout, "totaltime %f\n", t);
	fprintf (stdout, "mintime %d\n", MINTIME);
	fprintf (stdout, "inner %d\n", inner);
	fprintf (stdout, "runs %d\n", runs);
	fprintf (stdout, "avgtime %.8f\n", avg);
	fprintf (stdout, "avgtimem %.8f\n", 1000.0 * avg);
	fprintf (stdout, "avgtimeu %.8f\n", 1000000.0 * avg);

	//special data (may be meaningless for certain methods)
	fprintf (stdout, "iterations %d\n", g.icount);
	fprintf (stdout, "semiparent %d\n", g.scount);
	if (g.getNVertices() > 1) {
		fprintf (stdout, "semiparentf %.8f\n", (double)g.scount/(double)(g.getNVertices()-1));
	}
	fprintf (stdout, "comparisons %d\n", g.ccount);
	fprintf (stdout, "rcomparisons %.8f\n", (double)g.ccount/(double)g.getNVertices());
}


void outputArray (FILE *file, int k, int *a) {
	for (int i=1; i<=k; i++) {
		fprintf (file, "a[%d] = %d\n", i, a[i]);
	}
}

/*---------------
 | main function 
 *--------------*/

int main(int argc, char *argv[]) {
	if (argc < 3) printUsage(argv[0]);

	FILE *idomfile = NULL;  //output file for immediate dominators
	bool reverse = false;   //compute dominators (false) or postdominators (true)
	bool simplify = false;  //eliminate parallel edges before computing dominators?

	//read options
	if (argc>3) {
		for (int i=3; i<argc; i++) {
			if (strcmp(argv[i],"-reverse")==0) {
				reverse = true;
				continue;
			}
			if (strcmp(argv[i],"-normal")==0) {
				reverse = false;
				continue;
			}
			if (strcmp(argv[i],"-simplify")==0) {
				simplify = true;
				continue;
			}
			if (strcmp(argv[i],"-full")==0) {
				simplify = false;
				continue;
			}

			if (strcmp(argv[i],"-idomfile")==0) {
				i++;
				if (i==argc) fatal ("-idomfile requires an argument");
				idomfile = fopen (argv[i], "w");
				if (!idomfile) fatal ("cannot open file for writing");
				continue;
			}

			if (strcmp(argv[i],"-mintime")==0) {
				i++;
				if (i==argc) fatal ("-mintime requires an argument");
				MINTIME = atoi(argv[i]);
				continue;
			}

			fprintf (stderr, "WARNING: unrecognized option \"%s\".\n", argv[i]);
			//fprintf (stderr, "Setting mintime do %d.\n", m);
		}
	}
		
	//read filename
	char *filename = argv[1];

	//check if it is a seris or not
	bool series = false;
	int len = strlen(filename);
	if (len>=7) {
		if (strcmp (&filename[len-7], ".series")==0) series = true;
	}

	//read method
	char *method = argv[2];

	//special case: checks all methods
	printBasics(stdout);
	if (strcmp(method, "-check") == 0) {
		if (series) {
			checkSeries (filename, reverse, simplify);
		} else {
			DominatorGraph g;
			g.readDimacs(filename, reverse, simplify); //WARNING: MAKE SURE REVERSE IS INTERPRETED CORRECTLY
			int r = g.getSource();
			check (&g, r);
		}
	} else {
		Method m = getMethod(method);
		if (m==METHODS) fatal ("uknown method");

		if (series) {
			runSeries (filename, m, reverse, simplify);
		} else {
			runTests (filename, m, reverse, simplify, idomfile);
		}
	}

	if (idomfile) fclose(idomfile);
	return 0;
}

