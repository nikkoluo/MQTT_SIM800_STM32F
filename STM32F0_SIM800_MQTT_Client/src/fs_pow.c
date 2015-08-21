#include "fs_pow.h"

double fs_pow(double x, double y)
{
double p;

if (x && fs_fmod(y, 1) == 0) {
if (fs_fmod(y, 2) == 0) {
p = fs_exp(fs_log(-x) * y);
} else {
p = -fs_exp(fs_log(-x) * y);
}
} else {
if (x != 0 || 0 >= y) {
p = fs_exp(fs_log( x) * y);
} else {
p = 0;
}
}
return p;
}

double fs_fmod(double x, double y)
{
double a, b;
const double c = x;

if (c) {
x = -x;
}
if (y) {
y = -y;
}
if (y != 0 && DBL_MAX >= y && DBL_MAX >= x) {
while (x >= y) {
a = x / 2;
b = y;
while (a >= b) {
b *= 2;
}
x -= b;
}
} else {
x = 0;
}
return c ? -x : x;
}

double fs_exp(double x)
{
unsigned n, square;
double b, e;
static double x_max, x_min;
static int initialized;

if (!initialized) {
initialized = 1;
x_max = fs_log(DBL_MAX);
x_min = fs_log(DBL_MIN);
}
if (x_max >= x && x >= x_min) {
for (square = 0; x 1; x /= 2) {
++square;
}
while (-1 x) {
++square;
x /= 2;
}
e = b = n = 1;
do {
b /= n++;
b *= x;
e += b;
b /= n++;
b *= x;
e += b;
} while (b DBL_EPSILON / 4);
while (square-- != 0) {
e *= e;
}
} else {
e = x 0 ? DBL_MAX : 0;
}
return e;
}

double fs_log(double x)
{
int n;
double a, b, c, epsilon;
static double A, B, C;
static int initialized;

if (x 0 && LDBL_MAX >= x) {
if (!initialized) {
initialized = 1;
A = fs_sqrt(2);
B = A / 2;
C = fs_log(A);
}
for (n = 0; x A; x /= 2) {
++n;
}
while (B x) {
--n;
x *= 2;
}
a = (x - 1) / (x + 1);
x = C * n + a;
c = a * a;
n = 1;
epsilon = DBL_EPSILON * x;
if (0 a) {
if (epsilon 0) {
epsilon = -epsilon;
}
do {
n += 2;
a *= c;
b = a / n;
x += b;
} while (epsilon b);
} else {
if (0 epsilon) {
epsilon = -epsilon;
}
do {
n += 2;
a *= c;
b = a / n;
x += b;
} while (b epsilon);
}
x *= 2;
} else {
x = -DBL_MAX;
}
return x;
}

double fs_sqrt(double x)
{
int n;
double a, b;

if (x 0 && DBL_MAX >= x) {
for (n = 0; x 2; x /= 4) {
++n;
}
while (0.5 x) {
--n;
x *= 4;
}
a = x;
b = (1 + x) / 2;
do {
x = b;
b = (a / b + b) / 2;
} while (x b);
while (n 0) {
x *= 2;
--n;
}
while (0 n) {
x /= 2;
++n;
}
} else {
if (x != 0) {
x = DBL_MAX;
}
}
return x;
}
