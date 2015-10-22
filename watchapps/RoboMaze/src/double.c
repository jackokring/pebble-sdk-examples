#include "pebble.h"
#include "store.h"

#ifdef BULK_BUILD

double zero = 0.0;
double one = 1.0;
double ten = 10.0;
double tenten = 10000000000.0;
double tenth = 0.1;
double tententh = 0.0000000001;

double half(double x, double sgn) {		/* x/(1+sqrt(1+x*x)) */
	return x / (one + sqrt(one + sgn * x * x) ));
}

double halfa(double x) {
	return half(x, one);
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
//1001 qfn
//1010
//1011 
//1100 log with right input transform (is atanh)
//1101
//1110 atan
//1111

double eq(double x, bool over, bool sq, bool alt, bool fact) { //base e exponential and Q+
	double acc = 0;
	double lacc;
	double mul = x;
	double harm = 1;
	int start = 1;
	if(sq) x *= x;
	x = (alt ? -x : x);
	do {
		lacc = acc;
		acc += mul * (!over ? one : harm);
		start += sq + 1;
		harm = one/((double)start);
		mul *= x * (!fact ? one : harm * (!sq ? one : one/((double)start - 1)));
        } while(lacc != acc && start < 200);//term limit
	return acc;
}

double log(double x) { //base e
	x = irt(irt(irt(x)));//symetry and double triple roots
	return -eq((x-one) * inv(x+one), 1, 1, 0, 0) * 16.0;
}

double atan(double x) {
	return eq(halfa(halfa(x)), 1, 1, 1, 0) * 4.0F;
}

double circ(double x) {
	return sqrt(1.0F - square(x));
}

double exp(double x) {
	return eq(x, 0, 0, 0, 1) + 1.0F;
}

double qfn(double x) {
	return eq(x, 1, 0, 0, 1);
}

double invw(double x) {
	return x * exp(x);
}

double ein(double x) {
	return qfn(x) + log(x);
}

double lin(double x) {
	return ein(log(x));
}

//extra eight functions
//on root
double halfs(double x) {
	return half(x, -1.0F);
}

double halfc(double x) {
	return circ(x) * inv(x + 1);
}
//on logs
double asin(double x) {
	return 2.0F * atan(halfs(x));
}

double acos(double x) {
	return 2.0F * atan(halfc(x));
}
//on exps
double sin(double x) {
	return eq(x, 0, 1, 1, 1);
}

double cos(double x) {
	return sin(x + 1.57079632679F);
}
//on xtra
double tan(double x) {
	return sin(x) * inv(cos(x));
}

double entropy(double x) {
	return x * log(inv(x)) * 1.44269504089F;//base 2
}



#endif
