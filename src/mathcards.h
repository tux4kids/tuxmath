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

//#define MC_DEBUG
#ifdef MC_DEBUG
#define mcdprintf(...) printf(__VA_ARGS__)
#else
#define mcdprintf(...) 0
#endif

#define MC_FORMULA_LEN 16
#define MC_ANSWER_LEN 5

#define MC_USE_NEWARC

/* type of math operation used in a given question */
enum {
  MC_OPER_ADD,
  MC_OPER_SUB,
  MC_OPER_MULT,
  MC_OPER_DIV,
  MC_OPER_TYPING_PRACTICE,
  MC_NUM_OPERS
};

/* math question formats: */
enum {
  MC_FORMAT_ANS_LAST,     /* a + b = ? */
  MC_FORMAT_ANS_FIRST,    /* ? + b = c */
  MC_FORMAT_ANS_MIDDLE    /* a + ? = c */
};


/*
Indices for the various integer options. These are NOT the actual values!
Actual values are accessed as such: options.iopts[PLAY_THROUGH_LIST] = val;
Creating additional [integral] options is now centralized--this should be 
the only place where it's necessary to add code. (Besides actually using 
the new options!)
*/

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

#define NOPTS                     48 

const char const** MC_OPTION_TEXT = {
"PLAY_THROUGH_LIST",
"REPEAT_WRONGS",
"COPIES_REPEATED_WRONGS",
"ALLOW_NEGATIVES",
"MAX_ANSWER",
"MAX_QUESTIONS",
"QUESTION_COPIES",
"MAX_FORMULA_NUMS",
"MIN_FORMULA_NUMS",

"FORMAT_ANSWER_LAST",
"FORMAT_ANSWER_FIRST",
"FORMAT_ANSWER_MIDDLE",
"FORMAT_ADD_ANSWER_LAST",
"FORMAT_ADD_ANSWER_FIRST",
"FORMAT_ADD_ANSWER_MIDDLE",
"FORMAT_SUB_ANSWER_LAST",
"FORMAT_SUB_ANSWER_FIRST",
"FORMAT_SUB_ANSWER_MIDDLE",
"FORMAT_MULT_ANSWER_LAST",
"FORMAT_MULT_ANSWER_FIRST",
"FORMAT_MULT_ANSWER_MIDDLE",
"FORMAT_DIV_ANSWER_LAST",
"FORMAT_DIV_ANSWER_FIRST",
"FORMAT_DIV_ANSWER_MIDDLE",

"ADDITION_ALLOWED",
"SUBTRACTION_ALLOWED",
"MULTIPLICATION_ALLOWED",
"DIVISION_ALLOWED",
"TYPING_PRACTICE_ALLOWED",

"MIN_AUGEND",
"MAX_AUGEND",
"MIN_ADDEND",
"MAX_ADDEND",

"MIN_MINUEND",
"MAX_MINUEND",
"MIN_SUBTRAHEND",
"MAX_SUBTRAHEND",
 
"MIN_MULTIPLIER",
"MAX_MULTIPLIER",
"MIN_MULTIPLICAND",
"MAX_MULTIPLICAND",

"MIN_DIVISOR",
"MAX_DIVISOR",
"MIN_QUOTIENT",
"MAX_QUOTIENT",

"MIN_TYPING_NUM",
"MAX_TYPING_NUM",

"RANDOMIZE",

"END_OF_OPTS"
};
  
/* default values for math_options */
#define MC_GLOBAL_MAX 999          /* This is the largest absolute value that */
                                   /* can be entered for math question values.*/
#define MC_MATH_OPTS_INVALID -9999 /* Return value for accessor functions     */
                                   /* if math_opts not valid                  */
#define DEFAULT_FRACTION_TO_KEEP 1
                                          
const int* MC_DEFAULTS = {
  1,    //PLAY_THROUGH_LIST         
  1,    //REPEAT_WRONGS             
  1,    //COPIES_REPEATED_WRONGS    
  0,    //ALLOW_NEGATIVES           
  999,  //MAX_ANSWER                
  5000, //MAX_QUESTIONS             
  1,    //QUESTION_COPIES           
  2,    //MAX_FORMULA_NUMS          
  5,    //MIN_FORMULA_NUMS          
        //                          
  1,    //FORMAT_ANSWER_LAST        
  0,    //FORMAT_ANSWER_FIRST       
  0,    //FORMAT_ANSWER_MIDDLE      
  1,    //FORMAT_ADD_ANSWER_LAST    
  0,    //FORMAT_ADD_ANSWER_FIRST   
  0,    //FORMAT_ADD_ANSWER_MIDDLE  
  1,    //FORMAT_SUB_ANSWER_LAST    
  0,    //FORMAT_SUB_ANSWER_FIRST   
  0,    //FORMAT_SUB_ANSWER_MIDDLE  
  1,    //FORMAT_MULT_ANSWER_LAST   
  0,    //FORMAT_MULT_ANSWER_FIRST  
  0,    //FORMAT_MULT_ANSWER_MIDDLE 
  1,    //FORMAT_DIV_ANSWER_LAST    
  0,    //FORMAT_DIV_ANSWER_FIRST   
  0,    //FORMAT_DIV_ANSWER_MIDDLE  
        //                          
  1,    //ADDITION_ALLOWED          
  1,    //SUBTRACTION_ALLOWED       
  1,    //MULTIPLICATION_ALLOWED    
  1,    //DIVISION_ALLOWED          
  0,    //TYPING_PRACTICE_ALLOWED   
        //                          
  0,    //MIN_AUGEND                
  12,   //MAX_AUGEND                
  0,    //MIN_ADDEND                
  12,   //MAX_ADDEND                
        //                          
  0,    //MIN_MINUEND               
  12,   //MAX_MINUEND               
  0,    //MIN_SUBTRAHEND            
  12,   //MAX_SUBTRAHEND            
        //                          
  0,    //MIN_MULTIPLIER            
  12,   //MAX_MULTIPLIER            
  0,    //MIN_MULTIPLICAND          
  12,   //MAX_MULTIPLICAND          
        //                          
  0,    //MIN_DIVISOR                                          
  12,   //MAX_DIVISOR               
  0,    //MIN_QUOTIENT              
  12,   //MAX_QUOTIENT              
        //                          
  0,    //MIN_TYPING_NUM            
  12,   //MAX_TYPING_NUM            
        //                          
  1     //RANDOMIZE                 
};                      

#ifndef MC_USE_NEWARC
/* This struct contains all options that determine what */
/* math questions are asked during a game */
typedef struct MC_Options {
  /* general math options */
  int play_through_list;
  int repeat_wrongs;
  int copies_repeated_wrongs;
  int allow_negatives;
  int max_answer;
  int max_questions;
  int question_copies;         /* how many times each question is put in list */
  int randomize;               /* whether to shuffle cards */
  float fraction_to_keep;      /* Can use to have list contain a random subset */
                               /* of the questions meeting selection criteria. */

  /*  math question formats:   NOTE - list can contain more than one format*/
  /* operation-specific question formats:  */
  int format_add_answer_last;      /* a + b = ?    */ 
  int format_add_answer_first;     /* ? + b = c    */
  int format_add_answer_middle;    /* a + ? = c    */
  int format_sub_answer_last;      /* a - b = ?    */ 
  int format_sub_answer_first;     /* ? - b = c    */
  int format_sub_answer_middle;    /* a - ? = c    */
  int format_mult_answer_last;     /* a * b = ?    */ 
  int format_mult_answer_first;    /* ? * b = c    */
  int format_mult_answer_middle;   /* a * ? = c    */
  int format_div_answer_last;      /* a / b = ?    */ 
  int format_div_answer_first;     /* ? / b = c    */
  int format_div_answer_middle;    /* a / ? = c    */

  /* addition options */
  int addition_allowed;
  int min_augend;              /* the "augend" is the first addend i.e. "a" in "a + b = c" */
  int max_augend;
  int min_addend;              /* options for the other addend */
  int max_addend;
  /* subtraction options */
  int subtraction_allowed;
  int min_minuend;             /* minuend - subtrahend = difference */
  int max_minuend;
  int min_subtrahend;
  int max_subtrahend;
  /* multiplication options */
  int multiplication_allowed;
  int min_multiplier;          /* multiplier * multiplicand = product */
  int max_multiplier;
  int min_multiplicand;
  int max_multiplicand;
  /* division options */
  int division_allowed;
  int min_divisor;             /* dividend/divisor = quotient */
  int max_divisor;
  int min_quotient;
  int max_quotient;
  /* typing practice options */
  int typing_practice_allowed;
  int min_typing_num;
  int max_typing_num;

} MC_Options;
#else
typedef struct _MC_Options 
{
  int iopts[NOPTS];
  float fraction_to_keep; //being a float, we can't keep this in the same array
} MC_Options;             //it'll stay a special case, unless more float options
#endif                    //are introduced.

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

/* Simple "Set/Get" type functions for option parameters: */

/* Simple functions to set option parameters: */

/* Set general math options:   */
//void MC_SetPlayThroughList(int opt);
//void MC_SetRepeatWrongs(int opt);
//void MC_SetQuestionCopies(int copies);         /* how many times each question is put in list */
//void MC_SetCopiesRepeatedWrongs(int copies);
//void MC_SetMaxAnswer(int max);
//void MC_SetMaxQuestions(int max); 
//void MC_SetAllowNegatives(int opt);
//void MC_SetRandomize(int opt);           
//void MC_SetFractionToKeep(float fract);
//
///* Set question formats for all operations:     */
///* NOTE - list can contain more than one format */
///* Use these to set format the same for all four operations: */
//void MC_SetFormatAnswerLast(int opt);      /* a + b = ?, a - b = ?, a * b = ?, a / b = ?  */ 
//void MC_SetFormatAnswerFirst(int opt);     /* ? + b = c, etc   */
//void MC_SetFormatAnswerMiddle(int opt);    /* a + ? = c, etc   */
///* Uset these to set operation-specific question formats:                  */
//void MC_SetFormatAddAnswerLast(int opt);      /* a + b = ? */
//void MC_SetFormatAddAnswerFirst(int opt);     /* ? + b = c */
//void MC_SetFormatAddAnswerMiddle(int opt);    /* a + ? = c */
//void MC_SetFormatSubAnswerLast(int opt);      /* a - b = ? */
//void MC_SetFormatSubAnswerFirst(int opt);     /* ? - b = c */
//void MC_SetFormatSubAnswerMiddle(int opt);    /* a - ? = c */
//void MC_SetFormatMultAnswerLast(int opt);     /* a * b = ? */
//void MC_SetFormatMultAnswerFirst(int opt);    /* ? * b = c */
//void MC_SetFormatMultAnswerMiddle(int opt);   /* a * ? = c */
//void MC_SetFormatDivAnswerLast(int opt);      /* a / b = ? */
//void MC_SetFormatDivAnswerFirst(int opt);     /* ? / b = c */
//void MC_SetFormatDivAnswerMiddle(int opt);    /* a / ? = c */ 
//
///* Set the allowed math operations: */
//void MC_SetAddAllowed(int opt);
//void MC_SetSubAllowed(int opt);
//void MC_SetMultAllowed(int opt);
//void MC_SetDivAllowed(int opt);
//void MC_SetTypingAllowed(int opt);
//
///* Set min and max for addition: */
//void MC_SetAddMin(int opt);                    /* augend + addend = sum */
//void MC_SetAddMinAugend(int opt);              /* the "augend" is the first addend i.e. "a" in "a + b = c" */
//void MC_SetAddMinAddend(int opt);              /* options for the other addend */
//void MC_SetAddMax(int opt);
//void MC_SetAddMaxAugend(int opt);
//void MC_SetAddMaxAddend(int opt);
//
///* Set min and max for subtraction: */
//void MC_SetSubMin(int opt);
//void MC_SetSubMinMinuend(int opt);             /* minuend - subtrahend = difference */
//void MC_SetSubMinSubtrahend(int opt);
//void MC_SetSubMax(int opt);
//void MC_SetSubMaxMinuend(int opt);
//void MC_SetSubMaxSubtrahend(int opt);
//
///* Set min and max for multiplication: */
//void MC_SetMultMin(int opt);
//void MC_SetMultMinMultiplier(int opt);         /* multiplier * multiplicand = product */
//void MC_SetMultMinMultiplicand(int opt);
//void MC_SetMultMax(int opt);
//void MC_SetMultMaxMultiplier(int opt);
//void MC_SetMultMaxMultiplicand(int opt);
//
///* Set min and max for division: */
//void MC_SetDivMin(int opt);
//void MC_SetDivMinDivisor(int opt);            /* dividend/divisor = quotient */
//void MC_SetDivMinQuotient(int opt);
//void MC_SetDivMax(int opt);
//void MC_SetDivMaxDivisor(int opt);
//void MC_SetDivMaxQuotient(int opt);
//
///* Set min and max for typing practice: */
//void MC_SetTypeMin(int opt);
//void MC_SetTypeMax(int opt);
//
///* "Get" type functions to query option parameters: */
//
///* Query general math options: */
//int MC_PlayThroughList(void);
//int MC_RepeatWrongs(void);
//int MC_CopiesRepeatedWrongs(void);
//int MC_MaxAnswer(void);
//int MC_MaxQuestions(void);
//int MC_AllowNegatives(void);
//int MC_QuestionCopies(void);         /* how many times each question is put in list */
//int MC_Randomize(void);         
//float MC_FractionToKeep(void);
//
//int MC_FormatAddAnswerLast(void);      /* a + b = ?   */ 
//int MC_FormatAddAnswerFirst(void);     /* ? + b = c   */
//int MC_FormatAddAnswerMiddle(void);    /* a + ? = c   */
//int MC_FormatSubAnswerLast(void);      /* a - b = ?   */ 
//int MC_FormatSubAnswerFirst(void);     /* ? - b = c   */
//int MC_FormatSubAnswerMiddle(void);    /* a - ? = c   */
//int MC_FormatMultAnswerLast(void);      /* a * b = ?   */ 
//int MC_FormatMultAnswerFirst(void);     /* ? * b = c   */
//int MC_FormatMultAnswerMiddle(void);    /* a * ? = c   */
//int MC_FormatDivAnswerLast(void);      /* a / b = ?   */ 
//int MC_FormatDivAnswerFirst(void);     /* ? / b = c   */
//int MC_FormatDivAnswerMiddle(void);    /* a / ? = c   */
//
//
///* Query the allowed math operations: */
//int MC_AddAllowed(void);
//int MC_SubAllowed(void);
//int MC_MultAllowed(void);
//int MC_DivAllowed(void);
//int MC_TypingAllowed(void);
//
///* Query min and max for addition: */
//int MC_AddMinAugend(void);              /* the "augend" is the first addend i.e. "a" in "a + b = c" */
//int MC_AddMinAddend(void);              /* options for the other addend */
//int MC_AddMaxAugend(void);
//int MC_AddMaxAddend(void);
//
///* Query min and max for subtraction: */
//int MC_SubMinMinuend(void);             /* minuend - subtrahend = difference */
//int MC_SubMinSubtrahend(void);
//int MC_SubMaxMinuend(void);
//int MC_SubMaxSubtrahend(void);
//
///* Query min and max for multiplication: */
//int MC_MultMinMultiplier(void);         /* multiplier * multiplicand = product */
//int MC_MultMinMultiplicand(void);
//int MC_MultMaxMultiplier(void);
//int MC_MultMaxMultiplicand(void);
//
///* Query min and max for division: */
//int MC_DivMinDivisor(void);            /* dividend/divisor = quotient */
//int MC_DivMinQuotient(void);
//int MC_DivMaxDivisor(void);
//int MC_DivMaxQuotient(void);
//
///* Query min and max for typing practice: */
//int MC_TypeMin(void);
//int MC_TypeMax(void);

/********************************************
Public functions for new mathcards architecture
*********************************************/
unsigned int MC_MapTextToIndex(const char* text); //the array index of the text
void MC_SetOpt(unsigned int index, int val); //access directly,for internal use
int MC_GetOpt(unsigned int index);
void MC_SetOp(const char* param, int val); //access by text, for config reading
int MC_GetOp(const char* param);
void MC_SetFractionToKeep(float val);
float MC_GetFractionToKeep(void);
int MC_VerifyOptionListSane(void);

#endif
