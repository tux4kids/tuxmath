/*
   mathcards.h:

   Contains headers for a flashcard-type math game.
   This is a sort of interface-independent backend that could be used with a different
   user interface. Developed as an enhancement to Bill Kendrick's "Tux of Math Command"
   (aka tuxmath).  If tuxmath were a C++ program, this would be a C++ class.

   Copyright 2006, 2008, 2009, 2010.
Authors: David Bruce, Brendan Luchen
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


mathcards.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/



#ifndef MATHCARDS_H
#define MATHCARDS_H

#include "transtruct.h"


/* different classes of problems TuxMath will ask */
typedef enum _MC_ProblemType {
    MC_PT_TYPING,
    MC_PT_ARITHMETIC,
    MC_PT_COMPARISON,
    MC_NUM_PTYPES
} MC_ProblemType;

/* type of math operation used in an arithmetic question */
typedef enum _MC_Operation {
    MC_OPER_ADD,
    MC_OPER_SUB,
    MC_OPER_MULT,
    MC_OPER_DIV,
    MC_NUM_OPERS
} MC_Operation;

/* math question formats: */
typedef enum _MC_Format {
    MC_FORMAT_ANS_LAST,     /* a + b = ? */
    MC_FORMAT_ANS_FIRST,    /* ? + b = c */
    MC_FORMAT_ANS_MIDDLE,    /* a + ? = c */
    MC_NUM_FORMATS
} MC_Format;


/*
   Indices for the various integer options. These are NOT the actual values!
   Actual values are accessed as such: options.iopts[PLAY_THROUGH_LIST] = val;
   Creating additional [integral] options is now centralized--it should only
   be necessary to add to this list, the list of text, and the list of
   defaults. (Besides actually using the new options!)
   */
enum {
    NOT_VALID_OPTION = -1     ,
    PLAY_THROUGH_LIST = 0     , /* play until all questions answered correctly */
    QUESTION_COPIES           , /* # times each question is put in list */
    REPEAT_WRONGS             , /* reuse incorrectly answered questions or not */
    COPIES_REPEATED_WRONGS    , /* how many copies of an incorrectly answered question to re-insert*/
    ALLOW_NEGATIVES           ,
    MAX_ANSWER                ,
    MAX_QUESTIONS             ,
    MAX_FORMULA_NUMS          ,
    MIN_FORMULA_NUMS          ,

    //NOTE: Do _not_ rearrange the FORMAT values because the functions
    //rely on index arithmetic to iterate through these, and will be
    //broken if the relative position changes!
    FORMAT_ANSWER_LAST        , /* question format is: a + b = ? */
    FORMAT_ANSWER_FIRST       , /* question format is: ? + b = c */
    FORMAT_ANSWER_MIDDLE      , /* question format is: a + ? = c */
    FORMAT_ADD_ANSWER_LAST    , /* a + b = ?    */
    FORMAT_ADD_ANSWER_FIRST   , /* ? + b = c    */
    FORMAT_ADD_ANSWER_MIDDLE  , /* a + ? = c    */
    FORMAT_SUB_ANSWER_LAST    , /* a - b = ?    */
    FORMAT_SUB_ANSWER_FIRST   , /* ? - b = c    */
    FORMAT_SUB_ANSWER_MIDDLE  , /* a - ? = c    */
    FORMAT_MULT_ANSWER_LAST   , /* a * b = ?    */
    FORMAT_MULT_ANSWER_FIRST  , /* ? * b = c    */
    FORMAT_MULT_ANSWER_MIDDLE , /* a * ? = c    */
    FORMAT_DIV_ANSWER_LAST    , /* a / b = ?    */
    FORMAT_DIV_ANSWER_FIRST   , /* ? / b = c    */
    FORMAT_DIV_ANSWER_MIDDLE  , /* a / ? = c    */

    ADDITION_ALLOWED          ,
    SUBTRACTION_ALLOWED       ,
    MULTIPLICATION_ALLOWED    ,
    DIVISION_ALLOWED          ,
    TYPING_PRACTICE_ALLOWED   ,
    ARITHMETIC_ALLOWED        ,
    COMPARISON_ALLOWED        ,

    MIN_AUGEND                , /* augend + addend = sum */
    MAX_AUGEND                ,
    MIN_ADDEND                ,
    MAX_ADDEND                ,

    MIN_MINUEND               , /* minuend - subtrahend = difference */
    MAX_MINUEND               ,
    MIN_SUBTRAHEND            ,
    MAX_SUBTRAHEND            ,

    MIN_MULTIPLIER            , /* multiplier * multiplicand = product */
    MAX_MULTIPLIER            ,
    MIN_MULTIPLICAND          ,
    MAX_MULTIPLICAND          ,

    MIN_DIVISOR               , /* dividend/divisor = quotient */
    MAX_DIVISOR               , /* note - generate_list() will prevent */
    MIN_QUOTIENT              , /* questions with division by zero.    */
    MAX_QUOTIENT              ,

    MIN_TYPING_NUM            , /* range for "typing tutor" mode, for  */
    MAX_TYPING_NUM            , /* kids just learning to use keyboard. */

    MIN_COMPARATOR            , /* left comparison operand */
    MAX_COMPARATOR            ,
    MIN_COMPARISAND           , /* right comparison operannd */
    MAX_COMPARISAND           ,

    RANDOMIZE                 , /* whether to shuffle cards */

    COMPREHENSIVE             , /* whether to generate all questions 'in order' */
    AVG_LIST_LENGTH           , /* the average number of questions in a list */
    VARY_LIST_LENGTH          , /* whether to randomly adjust list length */

    NOPTS
};

extern const char* const MC_OPTION_TEXT[];
extern const int MC_DEFAULTS[];
extern const char operchars[MC_NUM_OPERS];

/* default values for math_options */
#define MC_MAX_DIGITS 3 
#define MC_GLOBAL_MAX 999          /* This is the largest absolute value that */
/* can be entered for math question values.*/
#define MC_MATH_OPTS_INVALID -9999 /* Return value for accessor functions     */
/* if math_opts not valid                  */
//#define DEFAULT_FRACTION_TO_KEEP 1


/* FIXME I think this array-based options system is *much* more error-prone */
/* and confusing than the old struct with a named field for each option,    */
/* albeit more compact. I think it would be worth the effort to change the  */
/* code back to the old system - DSB                                        */
typedef struct _MC_Options
{
    int iopts[NOPTS];
} MC_Options;



/* struct for node in math "flashcard" list */
typedef struct MC_MathQuestion {
    MC_FlashCard card;
    struct MC_MathQuestion* next;
    struct MC_MathQuestion* previous;
    int randomizer;
} MC_MathQuestion;


/* "public" function prototypes: these functions are how */
/* a user interface communicates with MathCards:         */
/* TODO provide comments thoroughly explaining these functions */

/*  MC_Initialize() sets up the struct containing all of  */
/*  settings regarding math questions.  It should be      */
/*  called before any other function.  Many of the other  */
/*  functions will not work properly if MC_Initialize()   */
/*  has not been called. It only needs to be called once, */
/*  i.e when the program is starting, not at the beginning*/
/*  of each math game for the player. Returns 1 if        */
/*  successful, 0 otherwise.                              */
int MC_Initialize(void);

/*  MC_StartGame() generates the list of math questions   */
/*  based on existing settings. It should be called at    */
/*  the beginning of each math game for the player.       */
/*  Returns 1 if resultant list contains 1 or more        */
/*  questions, 0 if list empty or not generated           */
/*  successfully.                                         */
int MC_StartGame(void);

/*  Generates a list math questions for every level based */
/*  on existing settings. It should be called during reset*/
/*  of a level. Question_list is now generated at every   */
/*  to support intra-game feedback mechanism.             */
/*  @Param int - The number of questions to be generated  */
/*  Returns 1 on successful generation, 0 otherwise       */
int MC_generate_questionlist(int);

/*  MC_StartGameUsingWrongs() is like MC_StartGame(),     */
/*  but uses the incorrectly answered questions from the  */
/*  previous game for the question list as a review form  */
/*  of learning. If there were no wrong answers (or no    */
/*  previous game), it behaves just like MC_StartGame().  */
/*  FIXME wonder if it should generate a message if the   */
/*  list is created from settings because there is no     */
/*  valid wrong question list?                            */
int MC_StartGameUsingWrongs(void);

/*  MC_NextQuestion() takes a pointer to an allocated     */
/*  MC_MathQuestion struct and fills in the fields for    */
/*  use by the user interface program. It basically is    */
/*  like taking the next flashcard from the pile.         */
/*  Returns 1 if question found, 0 if list empty/invalid  */
/*  or if argument pointer is invalid                     */
int MC_NextQuestion(MC_FlashCard* q);

/*  MC_AnsweredCorrectly() is how the user interface      */
/*  tells MathCards that the question has been answered   */
/*  correctly, and how long the student took to answer.   */
/*  Returns 1 if no errors.                               */
int MC_AnsweredCorrectly(int id, float t);

/*  MC_NotAnsweredCorrectly() is how the user interface   */
/*  tells MathCards that the question has not been        */
/*  answered correctly. Returns 1 if no errors.           */
int MC_NotAnsweredCorrectly(int id);

/*  Like MC_NextQuestion(), but takes "flashcard" from    */
/*  pile of incorrectly answered questions.               */
/*  Returns 1 if question found, 0 if list empty/invalid  */
int MC_NextWrongQuest(MC_FlashCard* q);

/*  Returns 1 if all have been answered correctly,        */
/*  0 otherwise.                                          */
int MC_MissionAccomplished(void);

/*  Returns number of questions left (either in list      */
/*  or "in play")                                         */
int MC_TotalQuestionsLeft(void);

/*  Returns questions left in list, NOT                   */
/*  including questions currently "in play".              */
int MC_ListQuestionsLeft(void);

/*  To keep track of how long students take to answer the */
/*  questions, one can report the time needed to answer   */
/*  an individual question:                               */
int MC_AddTimeToList(float t);
/*  Note that initialization of the list is handled by    */
/*  MC_StartGame.                                         */

/*  Tells MathCards to clean up - should be called when   */
/*  user interface program exits.                         */
void MC_EndGame(void);

/*  Prints contents of math_opts struct in human-readable   */
/*  form to given file. "verbose" tells the function to     */
/*  write a lot of descriptive "help"-type info for each    */
/*  option (intended to make config files self-documenting).*/
void MC_PrintMathOptions(FILE* fp, int verbose);

/* Additional functions used to generate game summary files: */
int MC_PrintQuestionList(FILE* fp);
int MC_PrintWrongList(FILE* fp);
int MC_StartingListLength(void);
int MC_WrongListLength(void);
int MC_NumAnsweredCorrectly(void);
int MC_NumNotAnsweredCorrectly(void);
float MC_MedianTimePerQuestion(void);
void print_card(MC_FlashCard card);

/********************************************
  Public functions for new mathcards architecture
 *********************************************/
/* Return the array index of the given text, e.g. randomize->47 */
unsigned int MC_MapTextToIndex(const char* text);
void MC_SetOpt(unsigned int index, int val); //set an option
int MC_GetOpt(unsigned int index); //get an option :)
int MC_VerifyOptionListSane(void);
int MC_MaxFormulaSize(void); //amount of memory needed to safely hold strings
int MC_MaxAnswerSize(void);
MC_FlashCard MC_AllocateFlashcard();
int MC_MakeFlashcard(char* buf, MC_FlashCard* fc);
void MC_CopyCard(const MC_FlashCard* src, MC_FlashCard* dest);
void MC_FreeFlashcard(MC_FlashCard* fc);
void MC_ResetFlashCard(MC_FlashCard* fc); //empty flashcard of strings & values
int MC_FlashCardGood(const MC_FlashCard* fc); //verifies a flashcard is valid
/* Reorganize formula_string and answer_string to render the same equation
   in a different format */
void reformat_arithmetic(MC_FlashCard* card, MC_Format f);

#endif
