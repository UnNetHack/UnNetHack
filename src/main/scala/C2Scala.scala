/* Copyright (c) 2014 Sheldon Young */

/** Ease the translation from C to Scala.  All of this needs go away as soon as possible. */
object C2Scala {

	val NULL = null

	/** Simplify conversion of if(someInt) expressions */
	implicit def int2Boolean(a: Int): Boolean = (a != 0)

	//
	// C functions and types
	//

	type regex_t = scala.util.matching.Regex
  type size_t = Int
	type time_t = Int

	def atoi(s: String): Int = s.toInt

	def strcmp(a: String, b: String): Int = a.compareTo(b)

	/** See <a href="http://www.cplusplus.com/reference/cstring/strncmp/">strncmp(1)</a>.*/
	def strncmp(a: String, b: String, n: Int): Int = a.take(n).compareTo(b.take(n))

	def strncmpi(a: String, b: String, n: Int): Int = a.take(n).compareToIgnoreCase(b.take(n))

	def strlen(a: String): Int = a.length

	//
	// Unix-like functions
	//

	def chmod(file: String, mode: Int): Unit = ()

	def getpid(): Int = 0

	def umask(mask: Int): Unit = ()

	def getpwnam(name: String): passwd = new passwd(name, 0)
	
	//def getpwuid(uid: Int): passwd = new passwd(Hack.getlogin(), 0)

	class passwd(val pw_name: String, val pw_uid: Int) { }

}
