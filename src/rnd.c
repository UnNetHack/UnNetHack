/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef USE_MERSENNE_TWISTER
extern gsl_rng *rng_state;
#define RND(x)	(int)(gsl_rng_get(rng_state) % (long)(x))

#else
/* "Rand()"s definition is determined by [OS]conf.h */
# if defined(UNIX) || defined(RANDOM)
#define RND(x)	(int)(Rand() % (long)(x))
# else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x)	(int)((Rand()>>3) % (x))
# endif
#endif /* USE_MERSENNE_TWISTER */

int
rn2(x)		/**< 0 <= rn2(x) < x */
register int x;
{
	if (x <= 0) {
		warning("rn2(%d) attempted", x);
		return(0);
	}
	x = RND(x);
	return(x);
}

int
rnl(x)		/**< 0 <= rnl(x) < x; sometimes subtracting Luck */
register int x;	/**< good luck approaches 0, bad luck approaches (x-1) */
{
	register int i;

	if (x <= 0) {
		warning("rnl(%d) attempted", x);
		return(0);
	}

	i = RND(x);

	if (Luck && rn2(50 - Luck)) {
	    i -= (x <= 15 && Luck >= -5 ? Luck/3 : Luck);
	    if (i < 0) i = 0;
	    else if (i >= x) i = x-1;
	}

	return i;
}

int
rnd(x)		/**< 1 <= rnd(x) <= x */
register int x;
{
	if (x <= 0) {
		warning("rnd(%d) attempted", x);
		return(1);
	}
	x = RND(x)+1;
	return(x);
}

int
d(n,x)		/**< n <= d(n,x) <= (n*x) */
register int n, x;
{
	register int tmp = n;

	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		warning("d(%d,%d) attempted", n, x);
		return(1);
	}
	while(n--) tmp += RND(x);
	return(tmp); /* Alea iacta est. -- J.C. */
}

int
rne(x)
register int x;
{
	register int n, utmp;

	utmp = (u.ulevel < 15) ? 5 : u.ulevel/3;
	n = 1;
	/* Slightly higher probabilities for higher n than in NetHack 3.4.3
	 * p(n) = \left(\frac{2}{x+2}\right)^{n-1} \frac{x}{x+2} */
	while (n < utmp && rnf(2,x+2))
		n++;
	return n;

	/* was:
	 *	tmp = 1;
	 *	while(!rn2(x)) tmp++;
	 *	return(min(tmp,(u.ulevel < 15) ? 5 : u.ulevel/3));
	 * which is clearer but less efficient and stands a vanishingly
	 * small chance of overflowing tmp
	 */
}

int
rnz(i)
int i;
{
	register long x = i;
	register long tmp = 1000;
	tmp += rn2(1000);
	tmp *= rne(4);
	if (rn2(2)) { x *= tmp; x /= 1000; }
	else { x *= 1000; x /= tmp; }
	return((int)x);
}

int
rnf(numerator, denominator) /**< @returns (rnf(n,d) < n/d) */
int numerator, denominator;
{
	return rn2(denominator) < numerator;
}

/*rnd.c*/
