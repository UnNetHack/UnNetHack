/* NetHack may be freely redistributed.  See license for details. */

/* used by ckmailstatus() to pass information to the mail-daemon in newmail() */

object Mail {

val MSG_OTHER = 0	/* catch-all; none of the below... */
val MSG_MAIL = 1	/* unimportant, uninteresting mail message */
val MSG_CALL = 2	/* annoying phone/talk/chat-type interruption */
val MSG_HINT = 3	/* gameplay-hint */

class mail_info {
	var message_typ = 0		/* MSG_foo value */
	var display_txt: String = null	/* text for daemon to verbalize */
	var object_nam: String = null	/* text to tag object with */
	var response_cmd: String = null	/* command to eventually execute */
}

}
