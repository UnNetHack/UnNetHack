/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

import Decl._
import You._
import java.util.Random

// MOTODO: Use a better random number generator, especially if running
// as a server.

object Rnd {

private val random = new Random()

/* "Rand()"s definition is determined by [OS]conf.h */
private def RND(x: Int): Int = (random.nextLong % x.toLong).toInt

/**< 0 <= rn2(x) < x */
def rn2(x: Int): Int =
{
	if (x <= 0) {
		warning("rn2(%d) attempted", x);
		return(0);
	}
	RND(x)
}

/**< 0 <= rnl(x) < x; sometimes subtracting Luck */
/**< good luck approaches 0, bad luck approaches (x-1) */
def rnl(x: Int): Int =
{
	if (x <= 0) {
		warning("rnl(%d) attempted", x);
		return(0);
	}

	var i = RND(x);

	if (Luck && rn2(50 - Luck)) {
	    i = i - (if(x <= 15 && Luck >= -5) Luck/3 else Luck)
	    if (i < 0) i = 0;
	    else if (i >= x) i = x-1;
	}

	return i;
}

/**< 1 <= rnd(x) <= x */
def rnd(x: Int): Int =
{
	if (x <= 0) {
		warning("rnd(%d) attempted", x);
		return(1);
	}
	RND(x)+1
}

/**< n <= d(n,x) <= (n*x) */
def d(_n: Int, x: Int): Int =
{
	var n = _n
	var tmp = n
	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		warning("d(%d,%d) attempted", n, x);
		return(1);
	}
	while(n >= 0) {
		tmp = tmp + RND(x)
		n = n -1
	}
	return(tmp); /* Alea iacta est. -- J.C. */
}

def rne(x: Int): Int = 
{
	var utmp = if(u.ulevel < 15) 5 else u.ulevel/3;
	var n = 1

	/* Slightly higher probabilities for higher n than in NetHack 3.4.3
	 * p(n) = \left(\frac{2}{x+2}\right)^{n-1} \frac{x}{x+2} */
	while (n < utmp && rnf(2,x+2))
		n = n + 1
	return n;

	/* was:
	 *	tmp = 1;
	 *	while(!rn2(x)) tmp++;
	 *	return(min(tmp,(u.ulevel < 15) ? 5 : u.ulevel/3));
	 * which is clearer but less efficient and stands a vanishingly
	 * small chance of overflowing tmp
	 */
}

def rnz(i: Int): Int =
{
	var x = i.toLong
	var tmp = 1000L * rn2(1000)
	tmp = tmp * rne(4);
	if (rn2(2) != 0) { x = x * tmp; x = x / 1000; }
	else { x = x * 1000; x = x / tmp; }
	x.toInt
}

// MOTODO: Original comment is incorrect, it's not < n/d, it's just < n.
def rnf(numerator: Int, denominator: Int): Boolean = /**< @returns (rnf(n,d) < n/d) */
{
	rn2(denominator) < numerator
}
}
