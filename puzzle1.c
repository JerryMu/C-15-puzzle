#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>


/** 
 * READ THIS DESCRIPTION
 *
 * node data structure: containing state, g, f
 * you can extend it with more information if needed
 */
typedef struct node{
	int state[16];
	int g;
	int f;
} node;

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

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

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
void print_state( int* s )
{
	int i;
	
	for( i = 0; i < 16; i++ )
		printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}
      
void printf_comma (long unsigned int n) {
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

/* return the sum of manhattan distances from state to goal */
int manhattan( int* state )
{
	int sum = 0;
	int i = 0;
	int x_gap = 0;
	int y_gap = 0;

	/**
	 * FILL WITH YOUR CODE
	 */
	 for(i = 0 ;i < 16 ;i++	){
	 	 if(state[i]!=0){
			 x_gap = abs(i%4-state[i]%4);
			 y_gap = abs(i/4-state[i]/4);
			 sum+= x_gap + y_gap;
	 	 }
	 }
	 //optimise 1 corner
	/* if(state[3]!=3){
	 	 if(state[2]==2){
	 	 	 sum+=2;
	 	 }
	 	 if(state[7]==7){
	 	 	 sum+=2;
	 	 }
	 }
	 if(state[12]!=12){
	 	 if(state[8]==8){
	 	 	 sum+=2;
	 	 }
	 	 if(state[13]==13){
	 	 	 sum+=2;
	 	 }
	 }
	 if(state[15]!=15){
	 	 if(state[14]==14){
	 	 	 sum+=2;
	 	 }
	 	 if(state[11]==11){
	 	 	 sum+=2;
	 	 }
	 }*/
	 //optimise 2 linear
	 int row[4], column[4];
	 for(i = 0; i<4;i++){
	 	 row[i] = 0;
	 	 column[i] = 0;
	 }
	 for(i = 0 ; i < 16 ;i++){
		 if(state[i]!=i){
		 	 if(i>0 && i<=3 && state[i] > 0 && state[i] <= 3){
	 	 	 	 row[0]+=1;
	 	 	 }else if(i>=4 && i<=7 && state[i] >= 4 && state[i] <= 7){
	 	 	 	 row[1]+=1;
	 	 	 }else if(i>=8 && i<=11&& state[i] >= 8 && state[i] <= 11){
	 	 	 	 row[2]+=1;
	 	 	 }else if(i>=12 && i<=15&& state[i] >= 12 && state[i] <= 15){
	 	 	 	 row[3]+=1;
	 	 	 }
	 	 	 if((i == 4 || i == 8 || i == 12 )&& 
			 (state[i] == 4 || state[i] == 8 || state[i] == 12)){
	 	 	 	 column[0]+=1;
	 	 	 }else if((i == 1 || i == 5 || i == 9 || i == 13 )&& 
			 (state[i] == 1 || state[i] == 5 || state[i] == 9 || state[i] == 13)){
	 	 	 	 column[1]+=1;
	 	 	 }else if((i == 2 || i == 6 || i == 10 || i == 14 )&& 
			 (state[i] == 2 || state[i] == 6 || state[i] == 10 || state[i] == 14)){
	 	 	 	 column[2]+=1;
	 	 	 }else if((i == 3 || i == 7 || i == 11 || i == 15 )&& 
			 (state[i] == 3 || state[i] == 7 || state[i] == 11 || state[i] == 15)){
	 	 	 	 column[3]+=1;
	 	 	 }
		 
	 	 }
	 }
 	 for(i = 0; i<4;i++){
	 	 if(row[i]>1){
	 	 	 sum+=(row[i]-1)*2;
	 	 }
		 if(column[i]>1){
	 	 	 sum+=(column[i]-1)*2;
	 	 }
	 } 
	 
	 
	
	return( sum );
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

/* Recursive IDA */
node* ida(node *input_node, int threshold, int* newThreshold,int last_operation)
{	/**
	 * FILL WITH YOUR CODE
	 *
	 * Algorithm in Figure 2 of handout
	 */
	node* r = NULL;
	int i = 0;
	

	 for(i = 0;i < 4; i++){
	 	 if(applicable(i)==1 && i!= last_operation){
	 	 	 generated+=1;
			 apply(input_node,i);
			 input_node->g = input_node->g + 1;
			 input_node->f = input_node->g+manhattan(input_node->state);
			 if(input_node->f > threshold){
			 	 expanded+=1;
			 	 if(*newThreshold > input_node->f){
			 	 	 *newThreshold = input_node->f;
			 	 }
			 }else{
			 	 if(manhattan(input_node->state) == 0){
			 	 	 return input_node;
			 	 }
			 	 r = ida(input_node,threshold,newThreshold,i);
			 	 if(r!=NULL){
			 	 	 return r;
			 	 }
			 }
			 input_node->f = 0;
			 input_node->g -= 1;
			 if(i == 0 || i == 2){
			 	 apply(input_node,i+1);
			 }else{
			 	 apply(input_node,i-1);
			 }
	 	 
		 }
	}
	return( NULL );
 }


/* main IDA control loop */
int IDA_control_loop(  ){
	node* r = NULL;
	int threshold;
	int *newThreshold = (int*)malloc(sizeof(int));
	/* initialize statistics */
	generated = 0;
	expanded = 0;

	/* compute initial threshold B */
	initial_node.f = threshold = manhattan( initial_node.state );

	printf( "Initial Estimate = %d\nThreshold = ", threshold);
	

	/**
	 * FILL WITH YOUR CODE
	 *
	 * Algorithm in Figure 1 of handout
	 */
	 while (!r){
		*newThreshold = 10000000; 

		initial_node.g = 0;
		r = ida(&initial_node,threshold,newThreshold,4);
		if(r == NULL){
			 threshold = *newThreshold; 
		}
		print_state(initial_node.state);
		printf("\n%d\n",threshold);
		
	 }
	 free(newThreshold);
	 
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

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
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

	print_state( initial_node.state );


	/* solve */
	float t0 = compute_current_time();
	
	solution_length = IDA_control_loop();				

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
	fclose(initFile);
	
	return( 0 );
}


