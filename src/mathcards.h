/*

        mathcards.h
        
        Description: contains headers for a flashcard-type math game. 
        This is a sort of interface-independent backend that could be used with a different
        user interface. Developed as an enhancement to Bill Kendrick's "Tux of Math Command"
        (aka tuxmath).  If tuxmath were a C++ program, this would be a C++ class.
        
        Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2006
        
        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/
#ifndef MATHCARDS_H
#define MATHCARDS_H

#define MC_DEBUG
#ifdef MC_DEBUG
#define mcdprintf(...) printf(__VA_ARGS__)
#else
#define mcdprintf(...) 0
#endif

#define MC_USE_NEWARC

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
enum {
  MC_FORMAT_ANS_LAST,     /* a + b = ? */
  MC_FORMAT_ANS_FIRST,    /* ? + b = c */
  MC_FORMAT_ANS_MIDDLE    /* a + ? = c */
};


/*
Indices for the various integer options. These are NOT the actual values!
Actual values are accessed as such: options.iopts[PLAY_THROUGH_LIST] = val;
Creating additional [integral] options is now centralized--it should only
be necessary to add to this list, the list of text, and the list of 
defaults. (Besides actually using the new options!)
*/

#define NOT_VALID_OPTION          -1
#define PLAY_THROUGH_LIST         0  /* play until all questions answered correctly */
#define REPEAT_WRONGS             1  /* reuse incorrectly answered questions or not */
#define COPIES_REPEATED_WRONGS    2  /* how many copies of an incorrectly answered question to re-insert*/
#define ALLOW_NEGATIVES           3  
#define MAX_ANSWER                4                                            
#define MAX_QUESTIONS             5                                            
#define QUESTION_COPIES           6  /* # times each question is put in list */
#define MAX_FORMULA_NUMS          7                                            
#define MIN_FORMULA_NUMS          8                                            
                                                                               
#define FORMAT_ANSWER_LAST        9  /* question format is: a + b = ? */
#define FORMAT_ANSWER_FIRST       10 /* question format is: ? + b = c */
#define FORMAT_ANSWER_MIDDLE      11 /* question format is: a + ? = c */
#define FORMAT_ADD_ANSWER_LAST    12 /* a + b = ?    */
#define FORMAT_ADD_ANSWER_FIRST   13 /* ? + b = c    */                           
#define FORMAT_ADD_ANSWER_MIDDLE  14 /* a + ? = c    */                        
#define FORMAT_SUB_ANSWER_LAST    15 /* a - b = ?    */
#define FORMAT_SUB_ANSWER_FIRST   16 /* ? - b = c    */
#define FORMAT_SUB_ANSWER_MIDDLE  17 /* a - ? = c    */
#define FORMAT_MULT_ANSWER_LAST   18 /* a * b = ?    */
#define FORMAT_MULT_ANSWER_FIRST  19 /* ? * b = c    */
#define FORMAT_MULT_ANSWER_MIDDLE 20 /* a * ? = c    */
#define FORMAT_DIV_ANSWER_LAST    21 /* a / b = ?    */
#define FORMAT_DIV_ANSWER_FIRST   22 /* ? / b = c    */                           
#define FORMAT_DIV_ANSWER_MIDDLE  23 /* a / ? = c    */                        
                                                                               
#define ADDITION_ALLOWED          24                                           
#define SUBTRACTION_ALLOWED       25                                           
#define MULTIPLICATION_ALLOWED    26                                           
#define DIVISION_ALLOWED          27                                           
#define TYPING_PRACTICE_ALLOWED   28                                           
                                                                               
#define MIN_AUGEND                29 /* augend + addend = sum */
#define MAX_AUGEND                30                                           
#define MIN_ADDEND                31                                           
#define MAX_ADDEND                32                                           
                                                                               
#define MIN_MINUEND               33 /* minuend - subtrahend = difference */
#define MAX_MINUEND               34                                           
#define MIN_SUBTRAHEND            35                                           
#define MAX_SUBTRAHEND            36                                           
                                                                               
#define MIN_MULTIPLIER            37 /* multiplier * multiplicand = product */
#define MAX_MULTIPLIER            38                                           
#define MIN_MULTIPLICAND          39  
#define MAX_MULTIPLICAND          40                                           
                                                                               
#define MIN_DIVISOR               41 /* dividend/divisor = quotient */
#define MAX_DIVISOR               42 /* note - generate_list() will prevent */
#define MIN_QUOTIENT              43 /* questions with division by zero.    */
#define MAX_QUOTIENT              44                                           
                                                                               
#define MIN_TYPING_NUM            45 /* range for "typing tutor" mode, for  */
#define MAX_TYPING_NUM            46 /* kids just learning to use keyboard. */
                                                                               
#define RANDOMIZE                 47 /* whether to shuffle cards */            

#define AVG_LIST_LENGTH           48 
#define VARY_LIST_LENGTH          49

#define NOPTS                     50 

extern const char* const MC_OPTION_TEXT[];
extern const int MC_DEFAULTS[];  
extern const char operchars[MC_NUM_OPERS];

/* default values for math_options */
#define MC_GLOBAL_MAX 999          /* This is the largest absolute value that */
                                   /* can be entered for math question values.*/
#define MC_MATH_OPTS_INVALID -9999 /* Return value for accessor functions     */
                                   /* if math_opts not valid                  */
//#define DEFAULT_FRACTION_TO_KEEP 1


typedef struct _MC_Options 
{
  int iopts[NOPTS];
  //float fraction_to_keep; //being a float, we can't keep this in the same array
} MC_Options;             //it'll stay a special case, unless more float options

#ifndef MC_USE_NEWARC
/* struct for individual "flashcard" */
typedef struct MC_FlashCard {
  int num1;
  int num2;
  int num3;
  int operation;
  int format;
  char formula_string[MC_FORMULA_LEN];
  char answer_string[MC_ANSWER_LEN];
} MC_FlashCard;
#else
/* experimental struct for a more generalized flashcard */
typedef struct _MC_FlashCard {
  char* formula_string;
  char* answer_string;
  int answer;
  int difficulty;
} MC_FlashCard;
#endif
                                                                  
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
/*  correctly. Returns 1 if no errors.                    */
int MC_AnsweredCorrectly(MC_FlashCard* q);

/*  MC_NotAnsweredCorrectly() is how the user interface    */
/*  tells MathCards that the question has not been        */
/*  answered correctly. Returns 1 if no errors.           */
int MC_NotAnsweredCorrectly(MC_FlashCard* q);

/*  Like MC_NextQuestion(), but takes "flashcard" from    */
/*  pile of incorrectly answered questions.               */
/*  Returns 1 if question found, 0 if list empty/invalid  */
int MC_NextWrongQuest(MC_FlashCard* q);

/*  Returns 1 if all have been answered correctly,        */
/* 0 otherwise.                                           */
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

/********************************************
Public functions for new mathcards architecture
*********************************************/
/* Return the array index of the given text, e.g. randomize->47 */
unsigned int MC_MapTextToIndex(const char* text);
void MC_SetOpt(unsigned int index, int val); //access directly,for internal use
int MC_GetOpt(unsigned int index);
void MC_SetOp(const char* param, int val); //access by text, for config reading
int MC_GetOp(const char* param);
void MC_SetFractionToKeep(float val);
float MC_GetFractionToKeep(void);
int MC_VerifyOptionListSane(void);
int MC_MaxFormulaSize(void);
int MC_MaxAnswerSize(void);
MC_FlashCard MC_AllocateFlashcard();
void MC_FreeFlashcard(MC_FlashCard* fc);
void MC_ResetFlashCard(MC_FlashCard* fc);
int MC_FlashCardGood(const MC_FlashCard* fc); //verifies a flashcard is valid

#endif
