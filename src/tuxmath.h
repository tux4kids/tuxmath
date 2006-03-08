/*
  tuxmath.h

  For TuxMath
  Contains global data for configuration of math questions and
  for general game options, as well as constants and defaults.

  Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2006


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  Added March 2, 2006

  Copyright: See COPYING file that comes with this distribution
  (briefly - GNU GPL v2 or later)
*/

#ifndef TUXMATH_H
#define TUXMATH_H

/* This struct contains all options that determine what */
/* math questions are asked during a game */
typedef struct math_option_type {
  /* general math options */
  int allow_neg_answer;
  int max_answer;
  int max_questions;
  int format_answer_last;
  int format_answer_first;
  int format_answer_middle;
  int question_copies;
  /* addition options */
  int addition_allowed;
  int min_augend;  /* the "augend" is the first addend i.e. "a" in "a + b = c" */
  int max_augend;
  int min_addend;  /* options for the other addend */
  int max_addend;
  /* subtraction options */
  int subtraction_allowed;
  int min_minuend;  /* minuend - subtrahend = difference */
  int max_minuend;
  int min_subtrahend;
  int max_subtrahend;
  /* multiplication options */
  int multiplication_allowed;
  int min_multiplier;  /* multiplier * multiplicand = product */
  int max_multiplier;
  int min_multiplicand;
  int max_multiplicand;
  /* division options */
  int division_allowed;
  int min_divisor;    /* dividend/divosor = quotient */
  int max_divisor;
  int min_quotient;
  int max_quotient;
} math_option_type;

/* this struct contains all other options regarding general */
/* gameplay but not having to do with math questions per se */
typedef struct game_option_type {
  /* general game options */
  int use_sound;
  int fullscreen;
  int use_bkgd;
  int demo_mode;
  int oper_override;
  int use_keypad;
  int reuse_questions;
  float speed;
  int allow_speedup;
  int max_comets;
  /* not sure the rest of these belong in here */
  int num_cities;  /* MUST BE AN EVEN NUMBER! */
  int num_bkgds;
  int max_city_colors;
} game_option_type;

/* make option data accessible to rest of program */
extern math_option_type* math_options; /* used by setup.c, options.c, game.c */
extern game_option_type* game_options; /* used by setup.c, options.c, game.c */

/* default values for math_options */
#define DEFAULT_ALLOW_NEG_ANSWER 0
#define DEFAULT_MAX_ANSWER 144
#define DEFAULT_MAX_QUESTIONS 5000
#define DEFAULT_FORMAT_ANSWER_LAST 1      /* question format is: a + b = ? */
#define DEFAULT_FORMAT_ANSWER_FIRST 0     /* question format is: ? + b = c */
#define DEFAULT_FORMAT_ANSWER_MIDDLE 0    /* question format is: a + ? = c */
#define DEFAULT_QUESTION_COPIES 1         /* how many times each question is put in list */

#define DEFAULT_ADDITION_ALLOWED 1
#define DEFAULT_MIN_AUGEND 0              /* the "augend" is the first addend i.e. "a" in "a + b = c" */
#define DEFAULT_MAX_AUGEND 12
#define DEFAULT_MIN_ADDEND 0
#define DEFAULT_MAX_ADDEND 12

#define DEFAULT_SUBTRACTION_ALLOWED 1     /* minuend - subtrahend = difference */
#define DEFAULT_MIN_MINUEND 0
#define DEFAULT_MAX_MINUEND 24
#define DEFAULT_MIN_SUBTRAHEND 0
#define DEFAULT_MAX_SUBTRAHEND 12

#define DEFAULT_MULTIPLICATION_ALLOWED 1
#define DEFAULT_MIN_MULTIPLIER 0          /* multiplier * multiplicand = product */
#define DEFAULT_MAX_MULTIPLIER 12
#define DEFAULT_MIN_MULTIPLICAND 0
#define DEFAULT_MAX_MULTIPLICAND 6

#define DEFAULT_DIVISION_ALLOWED 0        /* dividend/divisor = quotient */
#define DEFAULT_MIN_DIVISOR 0             /* note - generate_list() will prevent */
#define DEFAULT_MAX_DIVISOR 3             /* questions with division by zero.    */
#define DEFAULT_MIN_QUOTIENT 0
#define DEFAULT_MAX_QUOTIENT 3

/* default values for game_options */
#define DEFAULT_USE_SOUND 1
#define DEFAULT_FULLSCREEN 0
#define DEFAULT_USE_BKGD 1
#define DEFAULT_DEMO_MODE 0
#define DEFAULT_OPER_OVERRIDE 0
#define DEFAULT_USE_KEYPAD 1
#define DEFAULT_REUSE_QUESTIONS 0
#define DEFAULT_SPEED 1
#define DEFAULT_ALLOW_SPEEDUP 0
#define DEFAULT_MAX_COMETS 2	/* CHANGED FROM 10 BY DSB */
#define DEFAULT_NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */
#define DEFAULT_NUM_BKGDS 5
#define DEFAULT_MAX_CITY_COLORS 4

#endif
