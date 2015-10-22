#include "pebble.h"
#include "store.h"

#ifdef BULK_BUILD

double zero = 0.0;
double one = 1.0;
double two = 2.0;
double ten = 10.0;
double tenten = 10000000000.0;
double tenth = 0.1;
double tententh = 0.0000000001;

double log(double x);
double exp(double x);

double sqrt(double x) {
	return exp(log(x) / two);
}

double circ(double x, double scale) {
	return sqrt(one + scale * x * x);
}

static double half(double x, double sgn) {		/* x/(1+sqrt(1+x*x)) */
	return x / (one + circ(x, sgn));
}

double halfa(double x) {
	return half(x, one);
}

static double shanks(double * list, int idx) {
	return (list[idx+1]*list[idx-1] - list[idx]*list[idx])/(list[idx+1] + list[idx-1] - two*list[idx]);
}

static void copy(double * dest, double * src) {
	for(uint32_t i = 0; i < sizeof(double) * 9; ++i) dest[i] = src[i];
}

double accel(double * list) {//calculate a nested shanks estimate of convergence
	double tmp[9];
	copy(tmp, list);
	for(int j = 0; j < 4; ++j) {
		for(int i = 1 + j; i < 8 - j; ++i) {
			tmp[i] = shanks(list, i);
		}
		copy(list, tmp);
	}
	for(int j = 0; j < 2; ++j) {
		for(int i = 7 - j; i > 4 + j; --i) {
			tmp[i] = shanks(list, i);
		}
		copy(list, tmp);
	}
	return /* list[7]= */ shanks(list, 7);
}

//OSAF FN (flags and function produced)
//0000
//0001 expm1
//0010
//0011 expm1(ix)
//0100
//0101 sinh
//0110
//0111 sin
//1000
//1001 qfn (which must add log and a const for Ei)
//1010
//1011 
//1100 log with right input transform (is atanh)
//1101
//1110 atan
//1111

static double eq(double x, bool over, bool sq, bool alt, bool fact) { //base e exponential and Q+
	double mul = x;
	double harm = one;
	double a[9];
	double acc = zero;
	if(sq) x *= x;
	x = (alt ? -x : x);
	for(int start = 0; start < 9; ++start) {
		a[start] = (acc += mul * (!over ? one : harm));
		harm = one/((sq?two:one)*(double)start);
		mul *= x * (!fact ? one : harm * (!sq ? one : one/((two*(double)start) - 1)));
        }
	return accel(a);
}

double log(double x) { //base e
	return -eq((x-one) / (x+one), true, true, false, false) * two;
}

double atan(double x) {
	return eq(halfa(halfa(x)), true, true, true, false) * 4.0;
}

double exp(double x) {
	return eq(x, false, false, false, true) + one;
}

double ein(double x) {
	return eq(x, true, false, false, true) + log(x);
}

double lin(double x) {
	return ein(log(x));
}

//extra eight functions
//on root
double halfs(double x) {
	return half(x, -one);
}

double halfc(double x) {
	return circ(x, -one) / (x + one);
}
//on logs
double asin(double x) {
	return two * atan(halfs(x));
}

double acos(double x) {
	return two * atan(halfc(x));
}
//on exps
double sin(double x) {
	return eq(x, false, true, true, true);
}

double cos(double x) {
	return sin(x + 1.57079632679F);
}
//on xtra
double tan(double x) {
	return sin(x) / cos(x);
}

double entropy(double x) {
	return -x * log(x) * 1.44269504089;//base 2
}



#endif
