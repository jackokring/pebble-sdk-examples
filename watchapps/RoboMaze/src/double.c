#include "pebble.h"
#include "store.h"

#ifdef BULK_BUILD

dub zero = 0.0;
dub one = 1.0;
dub two = 2.0;
dub ten = 10.0;
dub tenten = 10000000000.0;
dub tenth = 0.1;
dub tententh = 0.0000000001;
dub logE10;//for digit conversion
dub PI2;//pi over 2

dub init();//fill constants
dub dlit(int m, int e);//build literal (e=0 converts to dub)
dub mul(dub a, dub b);//multiply
dub sub(dub a, dub b);//subtract
bool pos(dub a);//positive or negative
dub irt(dub a);//inverse root
int trunc(dub a);//truncate to an integer

// Simplifications
extern dub neg(dub a);//additive inverse
extern dub inv(dub a);//multiplicative inverse
extern dub div(dub a, dub b);//divide
extern dub add(dub a, dub b);//add
extern dub pow(dub a, int x);//integer power

dub sqrt(dub x) {
	return exp(log(x) / two);
}

dub circ(dub x, dub sgn) {
	return sqrt(one - sgn * x * x);
}

static dub half(dub x, dub sgn) {		/* x/(1+sqrt(1+x*x)) */
	return x / (one + circ(x, sgn));
}

dub halft(dub x) {
	return half(x, -one);
}

static dub shanks(dub * list, int idx) {
	return (list[idx+1]*list[idx-1] - list[idx]*list[idx])/(list[idx+1] + list[idx-1] - two*list[idx]);
}

static void copy(dub * dest, dub * src) {
	for(uint32_t i = 0; i < sizeof(dub) * 9; ++i) dest[i] = src[i];
}

dub accel(dub * list) {//calculate a nested shanks estimate of convergence
	dub tmp[9];
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

static dub eq(dub x, bool over, bool sq, bool alt, bool fact) { //base e exponential and Q+
	dub mul = x;
	dub harm = one;
	dub a[9];
	dub acc = zero;
	if(sq) x *= x;
	x = (alt ? -x : x);
	for(int start = 0; start < 9; ++start) {
		a[start] = (acc += mul * (!over ? one : harm));
		harm = one/((sq?two:one)*(dub)start);
		mul *= x * (!fact ? one : harm * (!sq ? one : one/((two*(dub)start) - 1)));
        }
	return accel(a);
}

dub log(dub x) { //base e
	bool neg = false;
	if(x > 1.0) {
		x = one / x;
		neg = true;
	}
	x = irt(irt(irt(irt(x))));//in range close enough to 1
	x = eq((x-one) / (x+one), true, true, false, false) * 32.0;
	return neg?-x:x;
}

dub atan(dub x) {
	return eq(x, true, true, true, false);
}

dub exp(dub x) {
	//use man exp with integer power raise or 1/x raise for neg ex
	return eq(x, false, false, false, true) + one;
}

dub ein(dub x) {
	return eq(x, true, false, false, true) + log(x);
}

dub lin(dub x) {
	return ein(log(x));
}

dub halfs(dub x) {
	return half(x, one);
}

dub halfc(dub x) {
	return circ(x, one) / (x + one);
}

dub asin(dub x) {
	return two * atan(halfs(x));
}

dub acos(dub x) {
	return two * atan(halfc(x));
}

dub sin(dub x) {
	//use modulo pi/2
	return eq(x, false, true, true, true);
}

dub cos(dub x) {
	return sin(x + 1.57079632679F);
}

dub tan(dub x) {
	return sin(x) / cos(x);
}

dub entropy(dub x) {
	return -x * log(x) * 1.44269504089;//base 2
}

#endif
