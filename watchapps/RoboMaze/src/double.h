//==============================================================================
// FLOAT LIBRARY FOR PEBBLE By Simon Jackson
//==============================================================================

typedef struct {
  int32_t man;
  int16_t exp; 
} dub;

// The constants (actually variables)
extern dub zero;
extern dub one;
extern dub two;
extern dub ten;
extern dub tenten;
extern dub tenth;
extern dub tententh;
extern dub logE10;//for digit conversion
extern dub PI2;//pi over 2

// Basis functions
extern dub init();//fill constants
extern dub dlit(int32_t m, int16_t e);//build literal
extern dub mul(dub a, dub b);//multiply
extern dub sub(dub a, dub b);//subtract
extern bool pos(dub a);//positive or negative
extern dub irt(dub a);//inverse root

// Simplifications
extern dub neg(dub a);//additive inverse
extern dub inv(dub a);//multiplicative inverse
extern dub div(dub a, dub b);//divide
extern dub add(dub a, dub b);//add

// Simple basic trig and circular processing (18 functions) - RADIANS!!!!
extern dub sqrt(dub x);//square root
extern dub halft(dub x);//tan half angle formula
extern dub accel(dub * list);//shanks acceleration on an array of dimension[9]
extern dub log(dub x);//base e log
extern dub atan(dub x);//arctan
extern dub circ(dub x, dub scale);//a simple root as in reletivity if scale is 1/c^2
extern dub exp(dub x);//exponential
extern dub ein(dub x);//exponential integral at a more natural origin (ok for limits subtraction of constant)
extern dub lin(dub x);//logarithmic integral minus a constant (natural origin)
extern dub halfs(dub x);//sine half angle formula
extern dub halfc(dub x);//cosine half angle formula
extern dub asin(dub x);//arcsine
extern dub acos(dub x);//arccos
extern dub sin(dub x);//sine
extern dub cos(dub x);//cosine
extern dub tan(dub x);//tangent
extern dub entropy(dub x);//bit entropy from probability

// More complex functions
/* 
dub mixer(dub s, dub z, prelog, postlog);//calculation of belows (prelog takes k, post log takes s)
dub hurwitz(dub s, dub z);//hurwitz zeta
dub lgamma(dub z);//ln(gamma(z)) function
dub polygamma(dub n, dub z);// [(-1)^n *] gamma(n) * hurwitz(n, z);//better than n+1 def i.e n=2 is digamma
dub bernoulli(dub n, dub z);// n * hurwitz(1-n, z)

// And various diff' eq'n usefuls */
