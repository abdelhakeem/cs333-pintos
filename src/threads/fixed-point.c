#include "threads/fixed-point.h"
#include <stdint.h>

fixed_p int_to_fixed_p (int n){
	return n * FIXED_P_F;
}

int fixed_p_to_int (fixed_p x){
	return x / FIXED_P_F;
}

fixed_p fixed_p_multiply (fixed_p x, fixed_p y){
	return ((int64_t) x) * y / FIXED_P_F;
}

fixed_p fixed_p_divde (fixed_p x, fixed_p y){
	return 	((int64_t) x) * FIXED_P_F / y;
}