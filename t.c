#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
	int sum = 0;
	int i = 0;
	int gap = 0;

	/**
	 * FILL WITH YOUR CODE
	 */
	 for(i = 0 ;i < 16 ;i++	){
	 	 gap = abs(i-state[i]);
	 	 sum+= gap/4 + gap%4;
	 }
}
