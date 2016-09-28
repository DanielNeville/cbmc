#include <assert.h>

int main(){

	int z;

	int x = z + 1;

	__CPROVER_set_taint("main::1::x", "tainted");

	x++;
	
	// taint propagation from x to y.
	int y = x; // int y = (z + 1)


	// taint propagation from y to c.
	int c = y + 7;

	// assert c is tainted.
	assert(__CPROVER_is_taint("main::1::c", "tainted"));

	return 0;
}

