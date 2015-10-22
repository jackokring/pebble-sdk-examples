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

/* use initial estimate and y'=y*(3-x*y*y)/2 with iterations */
double irt(double x) {
	u32 i;
        double x2;
        const double threehalfs = 1.5F;
	x2 = x * 0.5F;
        i  = * ( long * ) &x;                       // evil doubleing point bit level hacking
        i  = 0x5f3759df - ( i >> 1 );
        x  = * ( double * ) &i;
	for(i = 0; i < 4; i++)
        	x  *= ( threehalfs - ( x2 * square(x) ) );   //iteration
        return x;
}

double sqrt(double x) {

}

double half(double x, double sgn) {		/* x/(1+sqrt(1+x*x)) */
	return x / (one + sqrt(one + sgn * x * x) ));
}

double halfa(double x) {
	return half(x, one);
}

double shanks(double * list, int idx) {
	return (list[idx+1]*list[idx-1] - list[idx]*list[idx])/(list[idx+1] + list[idx-1] - two*list[idx]);
}

double accel(double * list) {//calculate a nested shanks estimate of convergence
	double tmp[9];
	for(int i = 1; i < 8; ++i) {
		tmp[i] = shanks(list, i);
	}
	//tmp[0] = list[0];
	tmp[8] = list[8];
	for(int i = 7; i > 4; --i) {
		list[i] = shanks(tmp, i);
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
//1001 qfn
//1010
//1011 
//1100 log with right input transform (is atanh)
//1101
//1110 atan
//1111

double eq(double x, bool over, bool sq, bool alt, bool fact) { //base e exponential and Q+
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
	x = irt(irt(irt(x)));//symetry and double triple roots
	return -eq((x-one) * inv(x+one), true, true, false, false) * 16.0;
}

double atan(double x) {
	return eq(halfa(halfa(x)), true, true, true, false) * 4.0F;
}

double circ(double x) {
	return sqrt(one - square(x));
}

double exp(double x) {
	return eq(x, false, false, fasle, true) + one;
}

double qfn(double x) {
	return eq(x, true, false, false, true);
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
	return half(x, -one);
}

double halfc(double x) {
	return circ(x) * inv(x + one);
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
	return sin(x) * inv(cos(x));
}

double entropy(double x) {
	return x * log(inv(x)) * 1.44269504089F;//base 2
}



#endif
