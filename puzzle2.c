#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>


#define LEFT 0
#define ABOVE 1
#define RIGHT 2 
#define BELOW 3
#define SIZE 4
typedef int ST[SIZE][SIZE];  			

/** 
 * READ THIS DESCRIPTION
 *
 * node data structure: containing state, g, f
 * you can extend it with more information if needed
 */
typedef struct node{
	int state[16];
	ST State;
	int g;
	int f;
	int last_move; //store last movement of the node
	int g_node; //store generated_nodes
	int p_node; //store pruned_nodes
} node;
node* Ans;
int ans;
/**
 * Global Variables
 */

// used to track the position of the blank in a state,
// so it doesn't have to be searched every time we check if an operator is applicable
// When we apply an operator, blank_pos is updated
int blank_pos;

// Initial node of the problem
node initial_node;

// Statistics about the number of generated and expendad nodes
unsigned long generated;
unsigned long expanded;


/**
 * The id of the four available actions for moving the blank (empty slot). e.x.
 * Left: moves the blank to the left, etc. 
 */

//#define LEFT 0
//#define UP 2
//#define DOWN 3

/*
 * Helper arrays for the applicable function
 * applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down 
 */
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


/* print state */
void print_state( int* s, int A[][SIZE])
{
	int num = 0;
	for(int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++ ) {
			A[i][j] = s[num*4+j];
			printf("%d ", A[i][j]);
		}
		puts("");
		num++;
	}
 
 int i;
	printf("Initial State:\n");
	for( i = 0; i < 16; i++ )
		printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}
      
void printf_comma(long unsigned int n) {
    if (n < 0) {
        printf ("-");
        printf_comma (-n);
        return;
    }
    if (n < 1000) {
        printf ("%lu", n);
        return;
    }
    printf_comma (n/1000);
    printf (",%03lu", n%1000);
}

void potential_actions(int B[], int A[][SIZE]) {
	int i,j;
	for (i=0; i<SIZE; i++){
		for (j=0; j<SIZE; j++){
			/* top row */
			if (A[i][j]==0) {
				if (i==0){
					if (j==0) {
						B[RIGHT] = 1;
						B[BELOW] = 1;
					}
					if (j==3) {
						B[LEFT] = 1;
						B[BELOW] = 1;
					}
					if (j!=0 && j!=3){
						B[LEFT] = 1;
						B[RIGHT] = 1;
						B[BELOW] = 1;
					}
				}
				/* bottom row */
				if (i==3) {
					if (j==0) {
						B[RIGHT] = 1;
						B[ABOVE] = 1;
					}
					if (j==3) {
						B[LEFT] = 1;
						B[ABOVE] = 1;
					}
					if (j!=0 && j!=3) {
						B[LEFT] = 1;
						B[RIGHT] = 1;
						B[ABOVE] = 1;
					}
				}
				/* remaining first and last column */
				if (i!=0 && i!=3 ) {
					if (j==0) {
						B[ABOVE] = 1;
						B[RIGHT] = 1;
						B[BELOW] = 1;
					}
					if (j==3) {
						B[ABOVE] = 1;
						B[LEFT] = 1;
						B[BELOW] = 1;
					}
				}
				/* middle */
				if ( (i==1 || i==2) && (j==1 || j==2)){
					B[ABOVE] = 1;
					B[LEFT] = 1;
					B[RIGHT] = 1;
					B[BELOW] = 1;
				}
			}
		}
	}
}

/* return 1 if op is applicable in state, otherwise return 0 */
int applicable( int op )
{
     return( ap_ops[op][blank_pos] );
}


/* apply operator */
void apply( node* n, int op )
{
	int t;
	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == 0 ? -1 : (op == 1 ? 1 : (op == 2 ? -4 : 4)));
	//apply op
	n->state[blank_pos] = n->state[t];
	n->state[t] = 0;
	
	//update blank pos
	blank_pos = t;
}

void array_copy(int A[][SIZE], int B[][SIZE]){
	int i,j;
	for (i=0; i<SIZE; i++){
		for (j=0; j<SIZE; j++){
			A[i][j] = B[i][j]; 
		}
	}
}
		

int calc_heuristic(int A[][SIZE]){
	int i,j;
	int h = 0;
	
	for (i=0; i<SIZE; i++){
		for (j=0; j<SIZE; j++){
			
			if (A[i][j]!=0){
				/* calculate manhattan distance cell by cell exclude blank cell */
				h += abs(i-((A[i][j]-(A[i][j]%SIZE))/SIZE))+abs(j-(A[i][j]%SIZE)); 	
				
				/* optimization 1: corner-tiles heuristic */
				// check conditon for three corners exclude top left corner 
				// if the corner tile is not goal tile and the neighbouring tiles are correctly 
				// positioned then add two moves for each correctly positioned neighbouring tile
				// since it needs to move somewhere else and then move back to make way for 
				// corner tile to be correctly positioned 
				if (i == 0 && j==3 ) {
					if (A[i][j]!=3) {
						if (A[i][j-1]==2){
							h=h+2;
						}	
						if (A[i+1][j]==7){
							h=h+2;
						}			
					}	
				}
				if (i == 3 && j==0 ) {
					if (A[i][j]!=12) {
						if (A[i-1][j]==8){
							h=h+2;
						}	
						if (A[i][j+1]==13){
							h=h+2;
						}			
					}
				}
				if (i == 3 && j==3 ) {
					if (A[i][j]!=15) {
						if (A[i-1][j]==11){
							h=h+2;
						}	
						if (A[i][j-1]==14){
							h=h+2;
						}			
					}
				}
			}
		}
	}	
	return h;
}
void next_state(int a, int B[], int A[][SIZE]){
	int i,j;
	for(i=0;i<SIZE;i++) {
		for(j=0;j<SIZE;j++) {
			if (A[i][j] == 0) {
				//swap the left tile and the blank tile 
				if (a==LEFT) {
					A[i][j] = A[i][j-1];
					A[i][j-1] = 0;
					return;
				}
				//swap the upper tile and the blank tile 
				if (a==ABOVE) {
					A[i][j] = A[i-1][j];
					A[i-1][j] = 0;
					return;
				}
				//swap the right tile and the blank tile 
				if (a==RIGHT) {
					A[i][j] = A[i][j+1];
					A[i][j+1] = 0;
					return;
				}
				//swap the lower tile and the blank tile 
				if (a==BELOW) {
					A[i][j] = A[i+1][j];
					A[i+1][j] = 0;
					return;
				}
			}
		}	
	}	
}





node *IDA(node *Node, int B, int *B_temp, int *generated_nodes, int *pruned_nodes) {
	int a;
	int action_index[SIZE] = {};
	
	node *r;
	r = NULL;
	node new_node;
	potential_actions(action_index, Node->State);
	
	
	for (a=0; a<SIZE; a++) {
		
		if (action_index[a] != 0 && a != Node->last_move) {
		/* implement the 'no immediate going back' optimization */
			if(a==LEFT) {
				new_node.last_move = RIGHT;
			}
			if(a==RIGHT) {
				new_node.last_move = LEFT;
			}
			if(a==ABOVE) {
				new_node.last_move = BELOW;
			}
			if(a==BELOW) {
				new_node.last_move = ABOVE;
			}
			*generated_nodes = *generated_nodes + 1;
			array_copy(new_node.State, Node->State);
			next_state(a, action_index, new_node.State);
			new_node.g = Node->g + 1;
			new_node.f = new_node.g + calc_heuristic(new_node.State);
			if (new_node.f > B) {
				*pruned_nodes = *pruned_nodes + 1;
				if (new_node.f < *B_temp) {
					/* update the temp bound */
					*B_temp = new_node.f;
				} 
			} else {
				if (calc_heuristic(new_node.State) == 0) {
					/* reach the goal node */
					printf("\n");
					//printf("Solution = %d\n", new_node.g);
					ans = new_node.g;
                    generated = new_node.g_node = *generated_nodes;
                    expanded = new_node.p_node = *pruned_nodes;
					new_node.g_node = *generated_nodes;
					new_node.p_node = *pruned_nodes;
					r = &new_node;
					return r;
				}
				r = IDA(&new_node, B, B_temp, generated_nodes, pruned_nodes);
				if (r!=NULL) {
					return r;
				}
			}
		
		}
	}
	return NULL;
}



/* main IDA control loop */
int IDA_control_loop(ST A){
	node* r = NULL;
	
	int threshold;
	
	/* initialize statistics */
	generated = 0;
	expanded = 0;

	/* compute initial threshold B */
	//initial_node.f = threshold = calc_heuristic();
	
	
	
	int B=0;
	int B_temp=0;
	int generated_nodes=0;
	int pruned_nodes=0;
	
	
	B = calc_heuristic(A); // initialize B
	printf( "Initial Estimate = %d\nThreshold = ", B );
	while(r==NULL) {
		printf("+++++++\n\n");
		B_temp = INT_MAX;
		node Node;
		array_copy(Node.State, A);
		Node.g = 0;
		Node.last_move = 0;
		r = IDA(&Node, B, &B_temp, &generated_nodes, &pruned_nodes);
		if (r==NULL) {
			B = B_temp; // update the threshold
			printf("%d ", B_temp);
		}
    }
	Ans = r;
    //return ans;
	/*------------------------------------------------------------------------------*/

	/**
	 * FILL WITH YOUR CODE
	 *
	 * Algorithm in Figure 1 of handout
	 */

	if(r)
		return r->g;
	else
		return -1;
}


static inline float compute_current_time()
{
	struct rusage r_usage;
	
	getrusage( RUSAGE_SELF, &r_usage );	
	float diff_time = (float) r_usage.ru_utime.tv_sec;
	diff_time += (float) r_usage.ru_stime.tv_sec;
	diff_time += (float) r_usage.ru_utime.tv_usec / (float)1000000;
	diff_time += (float) r_usage.ru_stime.tv_usec / (float)1000000;
	return diff_time;
}



int main( int argc, char **argv )
{
	int i, solution_length;
	int A[SIZE][SIZE];

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
	//initial_File(initFile);
	
	char buffer[256];

	if( fgets(buffer, sizeof(buffer), initFile) != NULL ){
		char* tile = strtok( buffer, " " );
		for( i = 0; tile != NULL; ++i )
			{
				initial_node.state[i] = atoi( tile );
				blank_pos = (initial_node.state[i] == 0 ? i : blank_pos);
				tile = strtok( NULL, " " );
			}		
	}
	else{
		fprintf( stderr, "Filename empty\"\n" );
		return( -2 );

	}
       
	if( i != 16 )
	{
		fprintf( stderr, "invalid initial state\n" );
		return( -1 );
	}

	/* initialize the initial node */
	initial_node.g=0;
	initial_node.f=0;

	print_state( initial_node.state, A);

	/* solve */
	float t0 = compute_current_time();
	
	solution_length = IDA_control_loop(A);				

	float tf = compute_current_time();

	/* report results */
	printf( "\nSolution = %d\n", solution_length);
	printf( "Generated = ");
	printf_comma(generated);		
	printf("\nExpanded = ");
	printf_comma(expanded);		
	printf( "\nTime (seconds) = %.2f\nExpanded/Second = ", tf-t0 );
	printf_comma((unsigned long int) expanded/(tf+0.00000001-t0));
	printf("\n\n");
	/* aggregate all executions in a file named report.dat, for marking purposes */
	FILE* report = fopen( "report.dat", "a" );

	fprintf( report, "%s", argv[1] );
	fprintf( report, "\n\tSoulution = %d, Generated = %lu, Expanded = %lu", solution_length, generated, expanded);
	fprintf( report, ", Time = %f, Expanded/Second = %f\n\n", tf-t0, (float)expanded/(tf-t0));
	fclose(report);
	
	return( 0 );
}


