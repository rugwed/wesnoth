/*
   Copyright (C) 2006 - 2015 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Contains the exception interfaces used to signal
 * completion of a scenario, campaign or turn.
 */

#ifndef GAME_END_EXCEPTIONS_HPP_INCLUDED
#define GAME_END_EXCEPTIONS_HPP_INCLUDED

#include "lua_jailbreak_exception.hpp"

#include "config.hpp"
#include "make_enum.hpp"

#include <string>
#include <exception>
#include <boost/optional.hpp>

MAKE_ENUM(LEVEL_RESULT,
	(VICTORY,	"victory")
	(DEFEAT,	"defeat")
	(QUIT,		"quit")
	(OBSERVER_END,	"observer_end")
)
MAKE_ENUM_STREAM_OPS1(LEVEL_RESULT)

/**
 * Exception used to escape form the ai code in case of [end_turn].
 */
class ai_end_turn_exception
	: public tlua_jailbreak_exception, public std::exception
{
public:

	ai_end_turn_exception()
		: tlua_jailbreak_exception()
		, std::exception()
	{
	}
	const char * what() const throw() { return "ai_end_turn_exception"; }
private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(ai_end_turn_exception)
};

/**
 * Exception used to signal the end of a player turn.
 */
class restart_turn_exception
	: public tlua_jailbreak_exception, public std::exception
{
public:

	restart_turn_exception()
		: tlua_jailbreak_exception()
		, std::exception()
	{
	}
	const char * what() const throw() { return "restart_turn_exception"; }

private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(restart_turn_exception)
};

/**
 * Struct used to transmit info caught from an end_turn_exception.
 */
struct end_level_struct {
	bool is_quit;
};

/**
 * Exception used to signal the end of a scenario.
 */
class end_level_exception
	: public tlua_jailbreak_exception
	, public std::exception
{
public:

	end_level_exception(bool isquit = false)
		: tlua_jailbreak_exception()
		, std::exception()
		, is_quit(isquit)
	{
	}

	bool is_quit;

	end_level_struct to_struct() {
		end_level_struct els = {is_quit};
		return els;
	}
	
	const char * what() const throw() { return "end_level_exception"; }
private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(end_level_exception)
};

/**
 * The two end_*_exceptions are caught and transformed to this signaling object
 */
typedef boost::optional<end_level_struct> possible_end_play_signal;

#define HANDLE_END_PLAY_SIGNAL( X )\
do\
{\
	try {\
		X;\
	} catch (end_level_exception & e) {\
		return possible_end_play_signal(e.to_struct());\
	}\
}\
while(0)



#define PROPOGATE_END_PLAY_SIGNAL( X )\
do\
{\
	possible_end_play_signal temp;\
	temp = X;\
	if (temp) {\
		return temp;\
	}\
}\
while(0)



#define HANDLE_AND_PROPOGATE_END_PLAY_SIGNAL( X )\
do\
{\
	possible_end_play_signal temp;\
	HANDLE_END_PLAY_SIGNAL( temp = X );\
	if (temp) {\
		return temp;\
	}\
}\
while(0)


/**
 * The non-persistent part of end_level_data
 */
struct transient_end_level{

	transient_end_level();

	bool carryover_report;             /**< Should a summary of the scenario outcome be displayed? */
	bool linger_mode;                  /**< Should linger mode be invoked? */
	std::string custom_endlevel_music; /**< Custom short music played at the end. */
	bool reveal_map;                   /**< Should we reveal map when game is ended? (Multiplayer only) */
};

/**
 * Additional information on the game outcome which can be provided by WML.
 */
struct end_level_data
{
	end_level_data();


	bool prescenario_save;             /**< Should a prescenario be created the next game? */
	bool replay_save;                  /**< Should a replay save be made? */
	bool proceed_to_next_level;        /**< whether to proceed to the next scenario, equals (res == VICTORY) in sp. We need to save this in saves during linger mode. > */
	transient_end_level transient;
	bool is_victory;
	void write(config& cfg) const;

	void read(const config& cfg);

	config to_config() const
	{
		config r;
		write(r);
		return r;
	}
};
inline void throw_quit_game_exception()
{
	// Distinguish 'Quit' from 'Regular' end_level_exceptions to solve the following problem:
	//   If a player quits the game during an event after an [endlevel] occurs, the game won't
	//   Quit but continue with the [endlevel] instead.
	throw end_level_exception(true);
}
#endif /* ! GAME_END_EXCEPTIONS_HPP_INCLUDED */
