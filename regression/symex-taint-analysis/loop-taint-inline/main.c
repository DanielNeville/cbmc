#include <assert.h>

int main() {

	int j;

	__CPROVER_set_taint("main::1::j", "tainted");

	int x[4];

	int i = 0;

	for (i = 0; i < 4; i++) {

		if (i == 4)
			x[i] = j;
		else
			x[i] = 0;
	}

	assert(__CPROVER_is_taint("main::1::x[3]", "tainted"));

	return 0;
}
