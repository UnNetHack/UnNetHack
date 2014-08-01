/* Copyright (c) Mike Stephenson 1991.				  */
/* NetHack may be freely redistributed.  See license for details. */
/* Conversion to Scala copyright (c) 2014 Sheldon Young. */

object Quest {

class q_score {			/* Quest "scorecard" */
	var first_start = false	/* only set the first time */
	var met_leader = false		/* has met the leader */
	var not_ready: Byte = 0		/* rejected due to alignment, etc. */
	var pissed_off = false		/* got the leader angry */
	var got_quest = false		/* got the quest assignment */

	var first_locate = false	/* only set the first time */
	var met_intermed = false	/* used if the locate is a person. */
	var got_final = false		/* got the final quest assignment */

	var made_goal: Byte = 0		/* # of times on goal level */
	var met_nemesis = false	/* has met the nemesis before */
	var killed_nemesis = false	/* set when the nemesis is killed */
	var in_battle = false		/* set when nemesis fighting you */

	var cheater = false		/* set if cheating detected */
	var touched_artifact = false	/* for a special message */
	var offered_artifact = false	/* offered to leader */
	var got_thanks = false		/* final message from leader */

	/* keep track of leader presence/absence even if leader is
	   polymorphed, raised from dead, etc */
	var leader_is_dead = false
	var leader_m_id = 0
}

val MAX_QUEST_TRIES = 7	/* exceed this and you "fail" */
val MIN_QUEST_ALIGN = 3	/* at least this align.record to start */
  /* note: align 3 matches "aligned" as reported by enlightenment (cmd.c) */
val MIN_QUEST_LEVEL = 10	/* at least this u.ulevel to start */
  /* note: exp.lev. 10 is threshold level for 4th rank (class title, role.c) */

}
