//==============================================================================
// FLOAT LIBRARY FOR PEBBLE By Simon Jackson
//==============================================================================

// The constants (actually variables
extern double zero;
extern double one;
extern double two;
extern double ten;
extern double tenten;
extern double tenth;
extern double tententh; 

// Simple basic trig and circular processing (18 functions) - RADIANS!!!!
double sqrt(double x);//square root
double halfa(double x);//tan half angle formula
double accel(double * list);//shanks acceleration on an array of dimension[9]
double log(double x);//base e log
double atan(double x);//arctan
double scale(double x);//a second parameter botch
double circ(double x);//a simple root as in reletivity if scale is -1/c^2
double exp(double x);//exponential
double ein(double x);//exponential integral at a more natural origin (ok for limits subtraction of constant)
double lin(double x);//logarithmic integral minus a constant (natural origin)
double halfs(double x);//sine half angle formula
double halfc(double x);//cosine half angle formula
double asin(double x);//arcsine
double acos(double x);//arccos
double sin(double x);//sine
double cos(double x);//cosine
double tan(double x);//tangent
double entropy(double x);//bit entropy from probability

// More complex functions
/* double hurwitz(double z);//hurwitz zeta (use scale as s)
double gamma(double z);//gamma function

// And various diff' eq'n usefuls */
