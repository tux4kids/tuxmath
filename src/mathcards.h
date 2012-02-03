/*

   mathcards.h

Description: contains headers for a flashcard-type math game.
This is a sort of interface-independent backend that could be used with a different
user interface. Developed as an enhancement to Bill Kendrick's "Tux of Math Command"
(aka tuxmath).  If tuxmath were a C++ program, this would be a C++ class.

Author: David Bruce <davidstuartbruce@gmail.com>, (C) 2006

Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

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

typedef struct _MC_MathGame {
    MC_MathQuestion* question_list;
    MC_MathQuestion* wrong_quests;
    MC_MathQuestion* active_quests;
    MC_MathQuestion* next_wrong_quest;
    int quest_list_length;
    int answered_correctly;
    int answered_wrong;
    int questions_pending;
    int unanswered;
    int starting_length;

    /* For keeping track of timing data */
    float* time_per_question_list;
    int length_time_per_question_list;
    int length_alloc_time_per_question_list;
    MC_Options* math_opts;
} MC_MathGame;


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
int MC_Initialize(MC_MathGame* game);

/*  MC_StartGame() generates the list of math questions   */
/*  based on existing settings. It should be called at    */
/*  the beginning of each math game for the player.       */
/*  Returns 1 if resultant list contains 1 or more        */
/*  questions, 0 if list empty or not generated           */
/*  successfully.                                         */
int MC_StartGame(MC_MathGame* game);

/*  MC_StartGameUsingWrongs() is like MC_StartGame(),     */
/*  but uses the incorrectly answered questions from the  */
/*  previous game for the question list as a review form  */
/*  of learning. If there were no wrong answers (or no    */
/*  previous game), it behaves just like MC_StartGame().  */
/*  FIXME wonder if it should generate a message if the   */
/*  list is created from settings because there is no     */
/*  valid wrong question list?                            */
int MC_StartGameUsingWrongs(MC_MathGame* game);

/*  MC_NextQuestion() takes a pointer to an allocated     */
/*  MC_MathQuestion struct and fills in the fields for    */
/*  use by the user interface program. It basically is    */
/*  like taking the next flashcard from the pile.         */
/*  Returns 1 if question found, 0 if list empty/invalid  */
/*  or if argument pointer is invalid                     */
int MC_NextQuestion(MC_MathGame* game, MC_FlashCard* q);

/*  MC_AnsweredCorrectly() is how the user interface      */
/*  tells MathCards that the question has been answered   */
/*  correctly, and how long the student took to answer.   */
/*  Returns 1 if no errors.                               */
int MC_AnsweredCorrectly(MC_MathGame* game, int id, float t);
//int MC_AnsweredCorrectly_id(int id);

/*  MC_NotAnsweredCorrectly() is how the user interface    */
/*  tells MathCards that the question has not been        */
/*  answered correctly. Returns 1 if no errors.           */
int MC_NotAnsweredCorrectly(MC_MathGame* game, int id);

/*  Like MC_NextQuestion(), but takes "flashcard" from    */
/*  pile of incorrectly answered questions.               */
/*  Returns 1 if question found, 0 if list empty/invalid  */
int MC_NextWrongQuest(MC_MathGame* game, MC_FlashCard* q);

/*  Returns 1 if all have been answered correctly,        */
/*  0 otherwise.                                          */
int MC_MissionAccomplished(MC_MathGame* game);

/*  Returns number of questions left (either in list      */
/*  or "in play")                                         */
int MC_TotalQuestionsLeft(MC_MathGame* game);

/*  Returns questions left in list, NOT                   */
/*  including questions currently "in play".              */
int MC_ListQuestionsLeft(MC_MathGame* game);

/*  To keep track of how long students take to answer the */
/*  questions, one can report the time needed to answer   */
/*  an individual question:                               */
int MC_AddTimeToList(MC_MathGame* game, float t);
/*  Note that initialization of the list is handled by    */
/*  MC_StartGame.                                         */

/*  Tells MathCards to clean up - should be called when   */
/*  user interface program exits.                         */
void MC_EndGame(MC_MathGame* game);

/*  Prints contents of math_opts struct in human-readable   */
/*  form to given file. "verbose" tells the function to     */
/*  write a lot of descriptive "help"-type info for each    */
/*  option (intended to make config files self-documenting).*/
void MC_PrintMathOptions(MC_MathGame* game, FILE* fp, int verbose);

/* Additional functions used to generate game summary files: */
int MC_PrintQuestionList(MC_MathGame* game, FILE* fp);
int MC_PrintWrongList(MC_MathGame* game, FILE* fp);
int MC_StartingListLength(MC_MathGame* game);
int MC_WrongListLength(MC_MathGame* game);
int MC_NumAnsweredCorrectly(MC_MathGame* game);
int MC_NumNotAnsweredCorrectly(MC_MathGame* game);
float MC_MedianTimePerQuestion(MC_MathGame* game);
void print_card(MC_FlashCard card);

/********************************************
  Public functions for new mathcards architecture
 *********************************************/
/* Return the array index of the given text, e.g. randomize->47 */
unsigned int MC_MapTextToIndex(const char* text);
void MC_SetOpt(MC_MathGame* game, unsigned int index, int val); //access directly,for internal use
int MC_GetOpt(MC_MathGame* game, unsigned int index);
void MC_SetOp(MC_MathGame* game, const char* param, int val); //access by text, for config reading
int MC_GetOp(MC_MathGame* game, const char* param);
int MC_VerifyOptionListSane(void);
int MC_MaxFormulaSize(void); //amount of memory needed to safely hold strings
int MC_MaxAnswerSize(void);
MC_FlashCard MC_AllocateFlashcard();
void MC_CopyCard(const MC_FlashCard* src, MC_FlashCard* dest);
void MC_FreeFlashcard(MC_FlashCard* fc);
void MC_ResetFlashCard(MC_FlashCard* fc); //empty flashcard of strings & values
int MC_FlashCardGood(const MC_FlashCard* fc); //verifies a flashcard is valid
int MC_MakeFlashcard(char* buf, MC_FlashCard* fc);

/* Reorganize formula_string and answer_string to render the same equation
   in a different format */
void reformat_arithmetic(MC_FlashCard* card, MC_Format f);

#endif

