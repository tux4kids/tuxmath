/*
*  C Implementation: mathcards.c
*
*         Description: implementation of backend for a flashcard-type math game. 
        Developed as an enhancement to Bill Kendrick's "Tux of Math Command"
        (aka tuxmath).  (If tuxmath were a C++ program, this would be a C++ class).
        MathCards could be used as the basis for similar games using a different interface.
         
*
*
* Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2005
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "mathcards.h"

/* extern'd constants */

const char* const MC_OPTION_TEXT[NOPTS+1] = {
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

"AVG_LIST_LENGTH",
"VARY_LIST_LENGTH",

"END_OF_OPTS"
};

const int MC_DEFAULTS[] = {
  1,    //PLAY_THROUGH_LIST         
  1,    //REPEAT_WRONGS             
  1,    //COPIES_REPEATED_WRONGS    
  0,    //ALLOW_NEGATIVES           
  999,  //MAX_ANSWER                
  5000, //MAX_QUESTIONS             
  1,    //QUESTION_COPIES           
  3,    //MAX_FORMULA_NUMS          
  1,    //MIN_FORMULA_NUMS          
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
  1,    //RANDOMIZE
  100,  //AVG_LIST_LENGTH
  1     //VARY_LIST_LENGTH
};                      



/* "Globals" for mathcards.c: */
const char operchars[4] = "+-*/";

MC_Options* math_opts = 0;
MC_MathQuestion* question_list = 0;
MC_MathQuestion* wrong_quests = 0;
MC_MathQuestion* next_wrong_quest = 0;
int initialized = 0;
int quest_list_length = 0;
int answered_correctly = 0;
int answered_wrong = 0;
int questions_pending = 0;
int unanswered = 0;
int starting_length = 0;
int max_formula_size = 0; //max length in chars of a flashcard's formula
int max_answer_size = 0; //and of its answer

/* For keeping track of timing data */
float* time_per_question_list = NULL;
int length_time_per_question_list = 0;
int length_alloc_time_per_question_list = 0;

const MC_FlashCard DEFAULT_CARD = {NULL,NULL,0,0}; //empty card to signal error

/* "private" function prototypes:                        */
/*                                                       */
/* these are for internal use by MathCards only - like   */
/* the private functions of a C++ class. Declared static */
/* to give file scope rather than extern scope.          */

static MC_MathQuestion* generate_list(void);
static void clear_negatives(void);
static int validate_question(int n1, int n2, int n3);
static MC_MathQuestion* create_node(int n1, int n2, int op, int ans, int f);
static MC_MathQuestion* create_node_from_card(MC_FlashCard* flashcard);
static MC_MathQuestion* insert_node(MC_MathQuestion* first, MC_MathQuestion* current, MC_MathQuestion* new_node);
static MC_MathQuestion* append_node(MC_MathQuestion* list, MC_MathQuestion* new_node);
static MC_MathQuestion* remove_node(MC_MathQuestion* first, MC_MathQuestion* n);
static MC_MathQuestion* delete_list(MC_MathQuestion* list);
static int copy_node(MC_MathQuestion* original, MC_MathQuestion* copy);
static int list_length(MC_MathQuestion* list);
static int randomize_list(MC_MathQuestion** list);

int comp_randomizer(const void *a, const void *b);
static MC_MathQuestion* pick_random(int length, MC_MathQuestion* list);
static int compare_node(MC_MathQuestion* first, MC_MathQuestion* other);
static int already_in_list(MC_MathQuestion* list, MC_MathQuestion* ptr);
static int int_to_bool(int i);
static int sane_value(int i);
static int abs_value(int i);
static int randomly_keep(void);
static int floatCompare(const void *v1,const void *v2);

static void print_list(FILE* fp,MC_MathQuestion* list);
void print_vect_list(FILE* fp, MC_MathQuestion** vect, int length);
static void print_node(FILE* fp, MC_MathQuestion* ptr);

/* these functions are dead code unless compiling with debug turned on: */
#ifdef MC_DEBUG 
static void print_card(MC_FlashCard card);
static void print_counters(void);
static MC_MathQuestion* create_node_copy(MC_MathQuestion* other);

static MC_FlashCard    create_card_from_node(MC_MathQuestion* node);
#endif

/* Functions for new mathcards architecture */
//static MC_FlashCard allocate_flashcard(void); //allocate space for a flashcard
//static void free_flashcard(MC_FlashCard fc); //be sure to free flashcards when done
static void free_node(MC_MathQuestion* mq); //wrapper for free() that also frees card
static MC_FlashCard generate_random_flashcard(void);
static MC_FlashCard generate_random_ooo_card_of_length(int length);
static void copy_card(const MC_FlashCard* src, MC_FlashCard* dest); //deep copy a flashcard
static MC_MathQuestion* allocate_node(void); //allocate space for a node
static int compare_card(const MC_FlashCard* a, const MC_FlashCard* b); //test for identical cards

/*  MC_Initialize() sets up the struct containing all of  */
/*  settings regarding math questions.  It should be      */
/*  called before any other function.  Many of the other  */
/*  functions will not work properly if MC_Initialize()   */
/*  has not been called. It only needs to be called once, */  
/*  i.e when the program is starting, not at the beginning*/
/*  of each math game for the player. Returns 1 if        */
/*  successful, 0 otherwise.                              */
int MC_Initialize(void)
{
  int i;
  
  mcdprintf("\nEntering MC_Initialize()");
  /* check flag to see if we did this already */
  if (initialized)
  {

    #ifdef MC_DEBUG
    printf("\nAlready initialized");  
    MC_PrintMathOptions(stdout, 0); 
    printf("\nLeaving MC_Initialize()\n");
    #endif

    return 1;
  }
  math_opts = malloc(sizeof(MC_Options));
  /* bail out if no struct */
  if (!math_opts)
  {

    mcdprintf("\nError: math_opts null or invalid");
    mcdprintf("\nLeaving MC_Initialize()\n");

    fprintf(stderr, "\nUnable to initialize math_options");
    return 0;
  } 
  
  /* set defaults */
  math_opts->fraction_to_keep = DEFAULT_FRACTION_TO_KEEP;
  for (i = 0; i < NOPTS; ++i)
    {
    math_opts->iopts[i] = MC_DEFAULTS[i];
    }

  /* if no negatives to be used, reset any negatives to 0 */
  if (!math_opts->iopts[ALLOW_NEGATIVES])
  {
    clear_negatives();
  }

  initialized = 1;

  #ifdef MC_DEBUG
  MC_PrintMathOptions(stdout, 0); 
  printf("\nLeaving MC_Initialize()\n");
  #endif 

  return 1;
}



/*  MC_StartGame() generates the list of math questions   */
/*  based on existing settings. It should be called at    */
/*  the beginning of each math game for the player.       */
/*  Returns 1 if resultant list contains 1 or more        */
/*  questions, 0 if list empty or not generated           */
/*  successfully.                                         */
int MC_StartGame(void)
{
  mcdprintf("\nEntering MC_StartGame()");

  /* if math_opts not set up yet, initialize it: */
  if (!initialized)
  {

    mcdprintf("\nNot initialized - calling MC_Initialize()");

    MC_Initialize();
  }

  if (!math_opts)  
  {
    mcdprintf("\nCould not initialize - bailing out");
    mcdprintf("\nLeavinging MC_StartGame()\n");

    return 0;
  }
  /* we know math_opts exists if we make it to here */
  srand(time(NULL));
  
  /* clear out old lists if starting another game: (if not done already) */
  delete_list(question_list);
  question_list = NULL;
  delete_list(wrong_quests);
  wrong_quests = NULL;

  /* clear the time list */
  if (time_per_question_list != NULL) {
    free(time_per_question_list);
    time_per_question_list = NULL;
    length_time_per_question_list = 0;
    length_alloc_time_per_question_list = 0;
  }
  
  /* determine how much space needed for strings, based on user options */
  max_formula_size = MC_GetOpt(MAX_FORMULA_NUMS)
                   * ((int)(log10f(MC_GLOBAL_MAX) ) + 4) //sign/operator/spaces
                   + 2; //question mark for answer, and ending '\0'
  max_answer_size = (int)(log10f(MC_GLOBAL_MAX) ) + 2; //sign and ending '\0'
  
  mcdprintf("max answer, formula size: %d, %d\n", 
            max_answer_size, max_formula_size);
  /* set up new list with pointer to top: */
  question_list = generate_list();

  next_wrong_quest = 0;
  /* initialize counters for new game: */
  quest_list_length = list_length(question_list);
  /* Note: the distinction between quest_list_length and  */
  /* unanswered is that the latter includes questions     */
  /* that are currently "in play" by the user interface - */
  /* it is only decremented when an answer to the question*/
  /* is received.                                         */
  unanswered = starting_length = quest_list_length;
  answered_correctly = 0;
  answered_wrong = 0;
  questions_pending = 0;
  
  #ifdef MC_DEBUG
  print_counters();
  #endif
  
  /* make sure list now exists and has non-zero length: */
  if (question_list && quest_list_length)
  {
    mcdprintf("\nGame set up successfully");
    mcdprintf("\nLeaving MC_StartGame()\n");

    return 1;
  }
  else
  {
    mcdprintf("\nGame NOT set up successfully - no valid list");
    mcdprintf("\nLeaving MC_StartGame()\n");

    return 0;
  }
}

/*  MC_StartGameUsingWrongs() is like MC_StartGame(),     */
/*  but uses the incorrectly answered questions from the  */
/*  previous game for the question list as a review form  */
/*  of learning. If there were no wrong answers (or no    */
/*  previous game), it behaves just like MC_StartGame().  */
/*  FIXME wonder if it should return a different value if */
/*  the list is created from settings because there is no */
/*  valid wrong question list?                            */
int MC_StartGameUsingWrongs(void)
{
  #ifdef MC_DEBUG
  printf("\nEntering MC_StartGameUsingWrongs()");
  #endif

  /* Note: if not initialized, control will pass to       */
  /* MC_StartGame() via else clause so don't need to test */
  /* for initialization here                              */
  if (wrong_quests &&
      list_length(wrong_quests))
  {
    #ifdef MC_DEBUG
    printf("\nNon-zero length wrong_quests list found, will");
    printf("\nuse for new game list:");
    #endif

    /* initialize lists for new game: */
    delete_list(question_list);
    if(!randomize_list(&wrong_quests)) {
      fprintf(stderr, "Error during randomization of wrong_quests!\n");
      /* Punt on trying wrong question list, just run normal game */
      return MC_StartGame();
    }
    question_list = wrong_quests;
    wrong_quests = 0;
    next_wrong_quest = 0; 
   /* initialize counters for new game: */
    quest_list_length = list_length(question_list);
    unanswered = starting_length = quest_list_length;
    answered_correctly = 0;
    answered_wrong = 0;
    questions_pending = 0;

    #ifdef MC_DEBUG
    print_counters();
    print_list(stdout, question_list);
    printf("\nLeaving MC_StartGameUsingWrongs()\n");
    #endif

    return 1;
  }
  else /* if no wrong_quests list, go to MC_StartGame()   */
       /* to set up list based on math_opts               */
  {
    #ifdef MC_DEBUG
    printf("\nNo wrong questions to review - generate list from math_opts\n");
    printf("\nLeaving MC_StartGameUsingWrongs()\n");
    #endif

    return MC_StartGame();
  }
}


/*  MC_NextQuestion() takes a pointer to an allocated     */
/*  MC_MathQuestion struct and fills in the fields for    */
/*  use by the user interface program. It basically is    */
/*  like taking the next flashcard from the pile. The     */
/*  node containing the question is removed from the list.*/
/*  Returns 1 if question found, 0 if list empty/invalid  */
/*  or if argument pointer is invalid.                    */
int MC_NextQuestion(MC_FlashCard* fc)
{
  #ifdef MC_DEBUG
  printf("\nEntering MC_NextQuestion()");
  #endif

  /* (so we can free the node after removed from list:) */
  MC_MathQuestion* ptr;
  ptr = question_list;

  if (!fc )
  {
    fprintf(stderr, "\nInvalid MC_FlashCard* argument!\n");

    #ifdef MC_DEBUG
    printf("\nInvalid MC_FlashCard* argument!");
    printf("\nLeaving MC_NextQuestion()\n");
    #endif

    return 0;
  }

  if (!question_list ||
/*      !next_question || */
      !list_length(question_list) )
  {
    #ifdef MC_DEBUG
    printf("\nquestion_list invalid or empty");
    printf("\nLeaving MC_NextQuestion()\n");
    #endif

    return 0;
  }
  copy_card(&question_list->card, fc);
//  fc->num1 = question_list->card.num1;
//  fc->num2 = question_list->card.num2;
//  fc->num3 = question_list->card.num3;
//  fc->operation = question_list->card.operation;
//  fc->format = question_list->card.format;
//  strncpy(fc->formula_string, question_list->card.formula_string, MC_FORMULA_LEN);  
//  strncpy(fc->answer_string, question_list->card.answer_string, MC_ANSWER_LEN);  

  /* take first question node out of list and free it:   */
  question_list = remove_node(question_list, question_list);
  free(ptr);
  quest_list_length--;
  questions_pending++;

  #ifdef MC_DEBUG
  printf("\nnext question is:");
  print_card(*fc);
  print_counters();
  printf("\nLeaving MC_NextQuestion()\n");
  #endif

  return 1;
}

/*  MC_AnsweredCorrectly() is how the user interface      */
/*  tells MathCards that the question has been answered   */
/*  correctly. Returns 1 if no errors.                    */
int MC_AnsweredCorrectly(MC_FlashCard* fc)
{
  #ifdef MC_DEBUG
  printf("\nEntering MC_AnsweredCorrectly()");
  #endif

  if (!fc)
  {
    fprintf(stderr, "\nMC_AnsweredCorrectly() passed invalid pointer as argument!\n");

    #ifdef MC_DEBUG
    printf("\nInvalid MC_FlashCard* argument!");
    printf("\nLeaving MC_AnsweredCorrectly()\n");
    #endif

    return 0;
  }

  #ifdef MC_DEBUG
  printf("\nQuestion was:");
  print_card(*fc);
  #endif
  
  answered_correctly++;
  questions_pending--;

  if (!math_opts->iopts[PLAY_THROUGH_LIST])
  /* reinsert question into question list at random location */
  {
    mcdprintf("\nReinserting question into list");
    
    MC_MathQuestion* ptr1;
    MC_MathQuestion* ptr2;
    /* make new node using values from flashcard */
    ptr1 = create_node_from_card(fc);
    /* put it into list */
    ptr2 = pick_random(quest_list_length, question_list);
    question_list = insert_node(question_list, ptr2, ptr1);
    quest_list_length++;
    /* unanswered does not change - was not decremented when */
    /* question allocated!                                   */
  }
  else
  {
    mcdprintf("\nNot reinserting question into list");
    /* not recycling questions so fewer questions remain:      */
    unanswered--;
  }

  #ifdef MC_DEBUG
  print_counters();
  printf("\nLeaving MC_AnsweredCorrectly()\n");
  #endif

  return 1;
}

/*  MC_NotAnsweredCorrectly() is how the user interface    */
/*  tells MathCards that the player failed to answer the  */
/*  question correctly. Returns 1 if no errors.           */
/*  Note: this gets triggered only if a player's city     */
/*  gets hit by a question, not if they "miss".           */
int MC_NotAnsweredCorrectly(MC_FlashCard* fc)
{
  mcdprintf("\nEntering MC_NotAnsweredCorrectly()");

  if (!fc)
  {
    fprintf(stderr, "\nMC_NotAnsweredCorrectly() passed invalid pointer as argument!\n");

    mcdprintf("\nInvalid MC_FlashCard* argument!");
    mcdprintf("\nLeaving MC_NotAnsweredCorrectly()\n");
    
    return 0;
  }

  #ifdef MC_DEBUG
  printf("\nQuestion was:");
  print_card(*fc);
  #endif

  answered_wrong++;
  questions_pending--;

  /* add question to wrong_quests list: */
  
  MC_MathQuestion* ptr1;
  MC_MathQuestion* ptr2;

  ptr1 = create_node_from_card(fc);

  if (!already_in_list(wrong_quests, ptr1)) /* avoid duplicates */
  {
    #ifdef MC_DEBUG
    printf("\nAdding to wrong_quests list");
    #endif

    wrong_quests = append_node(wrong_quests, ptr1);
  }  
  else /* avoid memory leak */
  {
    free(ptr1);
  }

  /* if desired, put question back in list so student sees it again */
  if (math_opts->iopts[REPEAT_WRONGS])
  {
    int i;

    mcdprintf("\nAdding %d copies to question_list:", math_opts->iopts[COPIES_REPEATED_WRONGS]);
   
    /* can put in more than one copy (to drive the point home!) */
    for (i = 0; i < math_opts->iopts[COPIES_REPEATED_WRONGS]; i++)
    {  
      ptr1 = create_node_from_card(fc);
      ptr2 = pick_random(quest_list_length, question_list);
      question_list = insert_node(question_list, ptr2, ptr1);
      quest_list_length++;
    }
    /* unanswered stays the same if a single copy recycled or */
    /* increases by 1 for each "extra" copy reinserted:       */
    unanswered += (math_opts->iopts[COPIES_REPEATED_WRONGS] - 1);
  }
  else
  {
    mcdprintf("\nNot repeating wrong answers\n");
    
    /* not repeating questions so list gets shorter:      */
    unanswered--;
  }

  #ifdef MC_DEBUG
  print_counters();
  printf("\nLeaving MC_NotAnswered_Correctly()\n");
  #endif

  return 1;

}

/* Tells user interface if all questions have been answered correctly! */
/* Requires that at list contained at least one question to start with */
/* and that wrongly answered questions have been recycled.             */
int MC_MissionAccomplished(void)
{
  if (starting_length
    && math_opts->iopts[REPEAT_WRONGS]
    && !unanswered)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/*  Returns number of questions left (either in list       */
/*  or "in play")                                          */
int MC_TotalQuestionsLeft(void)
{
  return unanswered;
}

/*  Returns number of questions left in list, NOT       */
/*  including questions currently "in play".            */
int MC_ListQuestionsLeft(void)
{
  return quest_list_length;
}


/*  Store the amount of time a given flashcard was      */
/*  visible on the screen. Returns 1 if the request     */
/*  succeeds, 0 otherwise.                              */
int MC_AddTimeToList(float t)
{
  int newsize = 0;
  float *newlist;

  /* This list will be allocated in an STL-like manner: when the       */
  /* list gets full, allocate an additional amount of storage equal    */
  /* to the current size of the list, so that only O(logN) allocations */
  /* will ever be needed. We therefore have to keep track of 2 sizes:  */
  /* the allocated size, and the actual number of items currently on   */
  /* the list.                                                         */
  if (length_time_per_question_list >= length_alloc_time_per_question_list) {
    /* The list is full, allocate more space */
    newsize = 2*length_time_per_question_list;
    if (newsize == 0)
      newsize = 100;
    newlist = realloc(time_per_question_list,newsize*sizeof(float));
    if (newlist == NULL) {
      #ifdef MC_DEBUG
      printf("\nError: allocation for time_per_question_list failed\n");
      #endif
      return 0;
    }
    time_per_question_list = newlist;
    length_alloc_time_per_question_list = newsize;
  }

  /* Append the time to the list */
  time_per_question_list[length_time_per_question_list++] = t;
  return 1;
}

/* Frees heap memory used in program:                   */
void MC_EndGame(void)
{
  delete_list(question_list);
  question_list = 0;
  delete_list(wrong_quests);
  wrong_quests = 0;

  if (math_opts)
  {
    free(math_opts);
    math_opts = 0;
  }

  free(time_per_question_list);
  time_per_question_list = NULL;
  length_alloc_time_per_question_list = 0;
  length_time_per_question_list = 0;

  initialized = 0;
}



/* prints struct to file */
void MC_PrintMathOptions(FILE* fp, int verbose)
{
  int i, vcommentsprimed = 0;
  static char* vcomments[NOPTS]; //comments when writing out verbose
  if (!vcommentsprimed) //we only want to initialize these once
  {
    vcommentsprimed = 1;
    for (i = 0; i < NOPTS; ++i)
      vcomments[i] = NULL;
    //TODO place comments in the slots where they should be written
    
  }
  
  
  #ifdef MC_DEBUG
  printf("\nEntering MC_PrintMathOptions()\n");
  #endif

  /* bail out if no struct */
  if (!math_opts)
  {
    fprintf(stderr, "\nMath Options struct does not exist!\n");
    return;
  }
#ifdef MC_USE_NEWARC
  for (i = 0; i < NOPTS; ++i)
    {
    if (verbose && vcomments[i] != NULL)
      fprintf(fp, vcomments[i]);
    fprintf(fp, "%s = %d\n", MC_OPTION_TEXT[i], math_opts->iopts[i]);
    }
  return;
#endif

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#                  General Math Options                    #\n"
                 "#                                                          #\n"
                 "# If 'play_through_list' is true, Tuxmath will ask each    #\n"
                 "# question in an internally-generated list. The list is    #\n"
                 "# generated based on the question ranges selected below.   #\n"
                 "# The game ends when no questions remain.                  #\n"
                 "# If 'play_through_list' is false, the game continues      #\n"
                 "# until all cities are destroyed.                          #\n"
                 "# Default is 1 (i.e. 'true' or 'yes').                     #\n"
                 "#                                                          #\n"
                 "# 'question_copies' is the number of times each question   #\n"
                 "# will be asked. It can be 1 to 10 - Default is 1.         #\n"
                 "#                                                          #\n"
                 "# 'repeat_wrongs' tells Tuxmath whether to reinsert        #\n"
                 "# incorrectly answered questions into the list to be       #\n"
                 "# asked again. Default is 1 (yes).                         #\n"
                 "#                                                          #\n"
                 "# 'copies_repeated_wrongs' gives the number of times an    #\n"
                 "# incorrectly answered question will reappear. Default     #\n"
                 "# is 1.                                                    #\n"
                 "#                                                          #\n"
                 "# The defaults for these values result in a 'mission'      #\n" 
                 "# for Tux that is accomplished by answering all            #\n"
                 "# questions correctly with at least one surviving city.    #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "play_through_list = %d\n", math_opts->iopts[PLAY_THROUGH_LIST]);
  fprintf (fp, "question_copies = %d\n", math_opts->iopts[QUESTION_COPIES]);
  fprintf (fp, "repeat_wrongs = %d\n", math_opts->iopts[REPEAT_WRONGS]);
  fprintf (fp, "copies_repeated_wrongs = %d\n", math_opts->iopts[COPIES_REPEATED_WRONGS]);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# The 'format_<op>_answer_<place>  options control         #\n"
                 "# generation of questions with the answer in different     #\n"
                 "# places in the equation.  i.e.:                           #\n"
                 "#                                                          #\n"
                 "#    format_add_answer_last:    2 + 2 = ?                  #\n"
                 "#    format_add_answer_first:   ? + 2 = 4                  #\n"
                 "#    format_add_answer_middle:  2 + ? = 4                  #\n"
                 "#                                                          #\n"
                 "# By default, 'format_answer_first' is enabled and the     #\n"
                 "# other two formats are disabled.  Note that the options   #\n"
                 "# are not mutually exclusive - the question list may       #\n"
                 "# contain questions with different formats.                #\n"
                 "#                                                          #\n"
                 "# The formats are set independently for each of the four   #\n"
                 "# math operations.                                         #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "format_add_answer_last = %d\n", math_opts->iopts[FORMAT_ADD_ANSWER_LAST]);
  fprintf (fp, "format_add_answer_first = %d\n", math_opts->iopts[FORMAT_ADD_ANSWER_FIRST]);
  fprintf (fp, "format_add_answer_middle = %d\n", math_opts->iopts[FORMAT_ADD_ANSWER_MIDDLE]);
  fprintf (fp, "format_sub_answer_last = %d\n", math_opts->iopts[FORMAT_SUB_ANSWER_LAST]);
  fprintf (fp, "format_sub_answer_first = %d\n", math_opts->iopts[FORMAT_SUB_ANSWER_FIRST]);
  fprintf (fp, "format_sub_answer_middle = %d\n", math_opts->iopts[FORMAT_SUB_ANSWER_MIDDLE]);
  fprintf (fp, "format_mult_answer_last = %d\n", math_opts->iopts[FORMAT_MULT_ANSWER_LAST]);
  fprintf (fp, "format_mult_answer_first = %d\n", math_opts->iopts[FORMAT_MULT_ANSWER_FIRST]);
  fprintf (fp, "format_mult_answer_middle = %d\n", math_opts->iopts[FORMAT_MULT_ANSWER_MIDDLE]);
  fprintf (fp, "format_div_answer_last = %d\n", math_opts->iopts[FORMAT_DIV_ANSWER_LAST]);
  fprintf (fp, "format_div_answer_first = %d\n", math_opts->iopts[FORMAT_DIV_ANSWER_FIRST]);
  fprintf (fp, "format_div_answer_middle = %d\n", math_opts->iopts[FORMAT_DIV_ANSWER_MIDDLE]);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# 'allow_negatives' allows or disallows use of negative    #\n"
                 "# numbers as both operands and answers.  Default is 0      #\n"
                 "# (no), which disallows questions like:                    #\n"
                 "#          2 - 4 = ?                                       #\n"
                 "# Note: this option must be enabled in order to set the    #\n"
                 "# operand ranges to include negatives (see below). If it   #\n"
                 "# is changed from 1 (yes) to 0 (no), any negative          #\n"
                 "# operand limits will be reset to 0.                       #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "allow_negatives = %d\n", math_opts->iopts[ALLOW_NEGATIVES]);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# 'max_answer' is the largest absolute value allowed in    #\n"
                 "# any value in a question (not only the answer). Default   #\n"
                 "# is 144. It can be set as high as 999.                    #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "max_answer = %d\n", math_opts->iopts[MAX_ANSWER]);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# 'max_questions' is limit of the length of the question   #\n"
                 "# list. Default is 5000 - only severe taskmasters will     #\n"
                 "# need to raise it.                                        #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "max_questions = %d\n", math_opts->iopts[MAX_QUESTIONS]);  

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# If 'randomize' selected, the list will be shuffled       #\n"
                 "# at the start of the game.  Default is 1 (yes).           #\n"
                 "############################################################\n\n");
  }
  fprintf (fp, "randomize = %d\n", math_opts->iopts[RANDOMIZE]);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#                 Math Operations Allowed                  #\n"
                 "#                                                          #\n"
                 "# These options enable questions for each of the four math #\n"
                 "# operations.  All are 1 (yes) by default.                 #\n"
                 "############################################################\n\n");
  }
  fprintf(fp, "addition_allowed = %d\n", math_opts->iopts[ADDITION_ALLOWED]);
  fprintf(fp, "subtraction_allowed = %d\n", math_opts->iopts[SUBTRACTION_ALLOWED]);
  fprintf(fp, "multiplication_allowed = %d\n", math_opts->iopts[MULTIPLICATION_ALLOWED]);
  fprintf(fp, "division_allowed = %d\n", math_opts->iopts[DIVISION_ALLOWED]);


  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#      Minimum and Maximum Values for Operand Ranges       #\n"
                 "#                                                          #\n"
                 "# Operand limits can be set to any integer up to the       #\n"
                 "# value of 'max_answer'.  If 'allow_negatives' is set to 1 #\n"
                 "# (yes), either negative or positive values can be used.   #\n"
                 "# Tuxmath will generate questions for every value in the   #\n"
                 "# specified range. The maximum must be greater than or     #\n"
                 "# equal to the corresponding minimum for any questions to  #\n"
                 "# be generated for that operation.                         #\n"
                 "############################################################\n\n");
  }
  fprintf(fp, "\n# Addition operands: augend + addend = sum\n");
  fprintf(fp, "min_augend = %d\n", math_opts->iopts[MIN_AUGEND]);
  fprintf(fp, "max_augend = %d\n", math_opts->iopts[MAX_AUGEND]);
  fprintf(fp, "min_addend = %d\n", math_opts->iopts[MIN_ADDEND]);
  fprintf(fp, "max_addend = %d\n", math_opts->iopts[MAX_ADDEND]);

  fprintf(fp, "\n# Subtraction operands: minuend - subtrahend = difference\n");
  fprintf(fp, "min_minuend = %d\n", math_opts->iopts[MIN_MINUEND]);
  fprintf(fp, "max_minuend = %d\n", math_opts->iopts[MAX_MINUEND]);
  fprintf(fp, "min_subtrahend = %d\n", math_opts->iopts[MIN_SUBTRAHEND]);
  fprintf(fp, "max_subtrahend = %d\n", math_opts->iopts[MAX_SUBTRAHEND]);

  fprintf(fp, "\n# Multiplication operands: multiplier * multiplicand = product\n");
  fprintf(fp, "min_multiplier = %d\n", math_opts->iopts[MIN_MULTIPLIER]);
  fprintf(fp, "max_multiplier = %d\n", math_opts->iopts[MAX_MULTIPLIER]);
  fprintf(fp, "min_multiplicand = %d\n", math_opts->iopts[MIN_MULTIPLICAND]);
  fprintf(fp, "max_multiplicand = %d\n", math_opts->iopts[MAX_MULTIPLICAND]);

  fprintf(fp, "\n# Division operands: dividend/divisor = quotient\n");
  fprintf(fp, "min_divisor = %d\n",math_opts->iopts[MIN_DIVISOR]);
  fprintf(fp, "max_divisor = %d\n", math_opts->iopts[MAX_DIVISOR]);
  fprintf(fp, "min_quotient = %d\n", math_opts->iopts[MIN_QUOTIENT]);
  fprintf(fp, "max_quotient = %d\n", math_opts->iopts[MAX_QUOTIENT]);

  fprintf(fp, "\n# Typing practice:\n");
  fprintf(fp, "min_typing_num = %d\n",math_opts->iopts[MIN_TYPING_NUM]);
  fprintf(fp, "max_typing_num = %d\n",math_opts->iopts[MAX_TYPING_NUM]);

  #ifdef MC_DEBUG
  printf("\nLeaving MC_PrintMathOptions()\n");
  #endif
}



int MC_PrintQuestionList(FILE* fp)
{
  if (fp && question_list)
  {
    print_list(fp, question_list);
    return 1;
  }
  else
  {
    fprintf(stderr, "\nFile pointer and/or question list invalid\n");
    return 0;
  }
}

int MC_PrintWrongList(FILE* fp)
{
  if (!fp)
  {
    fprintf(stderr, "File pointer invalid\n");
    return 0;
  }

  if (wrong_quests)
  {
    print_list(fp, wrong_quests);
  }
  else
  {
    fprintf(fp, "\nNo wrong questions!\n");
  }

  return 1;
}


int MC_StartingListLength(void)
{
  return starting_length;
}


int MC_WrongListLength(void)
{
  return list_length(wrong_quests);
}

int MC_NumAnsweredCorrectly(void)
{
  return answered_correctly;
}


int MC_NumNotAnsweredCorrectly(void)
{
  return answered_wrong;
}


/* Report the median time per question */
float MC_MedianTimePerQuestion(void)
{
  if (length_time_per_question_list == 0)
    return 0;

  qsort(time_per_question_list,length_time_per_question_list,sizeof(float),floatCompare);
  return time_per_question_list[length_time_per_question_list/2];
}

/* Implementation of "private methods" - (cannot be called from outside
of this file) */



/* Resets negative values to zero - used when allow_negatives deselected. */
void clear_negatives(void)
{
  if (math_opts->iopts[MIN_AUGEND ]< 0)
    math_opts->iopts[MIN_AUGEND ]= 0;
  if (math_opts->iopts[MAX_AUGEND ]< 0)
    math_opts->iopts[MAX_AUGEND ]= 0;
  if (math_opts->iopts[MIN_ADDEND ]< 0)
    math_opts->iopts[MIN_ADDEND] = 0;
  if (math_opts->iopts[MAX_ADDEND] < 0)
    math_opts->iopts[MAX_ADDEND] = 0;

  if (math_opts->iopts[MIN_MINUEND] < 0)
    math_opts->iopts[MIN_MINUEND] = 0;
  if (math_opts->iopts[MAX_MINUEND] < 0)
    math_opts->iopts[MAX_MINUEND] = 0;
  if (math_opts->iopts[MIN_SUBTRAHEND] < 0)
    math_opts->iopts[MIN_SUBTRAHEND] = 0;
  if (math_opts->iopts[MAX_SUBTRAHEND] < 0)
    math_opts->iopts[MAX_SUBTRAHEND] = 0;

  if (math_opts->iopts[MIN_MULTIPLIER] < 0)
    math_opts->iopts[MIN_MULTIPLIER] = 0;
  if (math_opts->iopts[MAX_MULTIPLIER] < 0)
    math_opts->iopts[MAX_MULTIPLIER] = 0;
  if (math_opts->iopts[MIN_MULTIPLICAND] < 0)
    math_opts->iopts[MIN_MULTIPLICAND] = 0;
  if (math_opts->iopts[MAX_MULTIPLICAND] < 0)
    math_opts->iopts[MAX_MULTIPLICAND] = 0;

  if (math_opts->iopts[MIN_DIVISOR] < 0)
    math_opts->iopts[MIN_DIVISOR] = 0;
  if (math_opts->iopts[MAX_DIVISOR] < 0)
    math_opts->iopts[MAX_DIVISOR] = 0;
  if (math_opts->iopts[MIN_QUOTIENT] < 0)
    math_opts->iopts[MIN_QUOTIENT] = 0;
  if (math_opts->iopts[MAX_QUOTIENT] < 0)
    math_opts->iopts[MAX_QUOTIENT] = 0;

  if (math_opts->iopts[MIN_TYPING_NUM] < 0)
    math_opts->iopts[MIN_TYPING_NUM] = 0;
  if (math_opts->iopts[MAX_TYPING_NUM] < 0)
    math_opts->iopts[MAX_TYPING_NUM] = 0;
}

/* using parameters from the mission struct, create linked list of "flashcards" */
/* FIXME should figure out how to proceed correctly if we run out of memory */
/* FIXME very redundant code - figure out way to iterate through different */
/* math operations and question formats                                    */
#ifndef MC_USE_NEWARC
MC_MathQuestion* generate_list(void)
{
  MC_MathQuestion* top_of_list = NULL;
  MC_MathQuestion* end_of_list = NULL;
  MC_MathQuestion* tmp_ptr = NULL;

  int i, j, k;
  int length = 0;

  #ifdef MC_DEBUG
  printf("\nEntering generate_list()");
  MC_PrintMathOptions(stdout, 0);
  #endif
 
  /* add nodes for each math operation allowed */

  #ifdef MC_DEBUG
  printf("\ngenerating addition questions\n");
  #endif

  if (math_opts->iopts[ADDITION_ALLOWED])
  {
    #ifdef MC_DEBUG
    printf("\nAddition problems");
    #endif
    for (i = math_opts->iopts[MIN_AUGEND]; i <= math_opts->iopts[MAX_AUGEND]; i++)
    {
      for (j = math_opts->iopts[MIN_ADDEND]; j <= math_opts->iopts[MAX_ADDEND]; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (validate_question(i, j, i + j))
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->iopts[QUESTION_COPIES]; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 + num2 = ? */
            if (math_opts->iopts[FORMAT_ADD_ANSWER_LAST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_ADD, i + j, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 + ? = num3 */
            if (math_opts->iopts[FORMAT_ADD_ANSWER_MIDDLE])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_ADD, i + j, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? + num2 = num3 */
            if (math_opts->iopts[FORMAT_ADD_ANSWER_FIRST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())

              {
                tmp_ptr = create_node(i, j, MC_OPER_ADD, i + j, MC_FORMAT_ANS_FIRST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }
          }
        }
      }
    }
  }

  #ifdef MC_DEBUG
  printf("generating subtraction questions\n");
  #endif

  if (math_opts->iopts[SUBTRACTION_ALLOWED])
  {
    #ifdef MC_DEBUG
    printf("\nSubtraction problems");
    #endif
    for (i = math_opts->iopts[MIN_MINUEND]; i <= math_opts->iopts[MAX_MINUEND]; i++)
    {
      for (j = math_opts->iopts[MIN_SUBTRAHEND]; j <= math_opts->iopts[MAX_SUBTRAHEND]; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (validate_question(i, j, i - j))
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->iopts[QUESTION_COPIES]; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 - num2 = ? */
            if (math_opts->iopts[FORMAT_SUB_ANSWER_LAST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())

              {
                tmp_ptr = create_node(i, j, MC_OPER_SUB, i - j, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 - ? = num3 */
            if (math_opts->iopts[FORMAT_SUB_ANSWER_MIDDLE])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())

              {
                tmp_ptr = create_node(i, j, MC_OPER_SUB, i - j, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? - num2 = num3 */
            if (math_opts->iopts[FORMAT_SUB_ANSWER_FIRST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_SUB, i - j, MC_FORMAT_ANS_FIRST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }
          }
        }
      }
    }
  }

  #ifdef MC_DEBUG
  printf("generating multiplication questions\n");
  #endif

  if (math_opts->iopts[MULTIPLICATION_ALLOWED])
  {
    #ifdef MC_DEBUG
    printf("\nMultiplication problems");
    #endif
    for (i = math_opts->iopts[MIN_MULTIPLIER]; i <= math_opts->iopts[MAX_MULTIPLIER]; i++)
    {
      for (j = math_opts->iopts[MIN_MULTIPLICAND]; j <= math_opts->iopts[MAX_MULTIPLICAND]; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (validate_question(i, j, i * j))
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->iopts[QUESTION_COPIES]; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 x num2 = ? */
            if (math_opts->iopts[FORMAT_MULT_ANSWER_LAST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_MULT, i * j, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 x ? = num3 */
            /* (no questions like 0 x ? = 0) because answer indeterminate */
            if ((math_opts->iopts[FORMAT_MULT_ANSWER_MIDDLE])
             && (i != 0)) 
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_MULT, i * j, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? x num2 = num3 */
            /* (no questions like ? X 0 = 0) because answer indeterminate */
            if ((math_opts->iopts[FORMAT_MULT_ANSWER_FIRST])
             && (j != 0))
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_MULT, i * j, MC_FORMAT_ANS_FIRST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }
          }
        }
      }
    }
  }

  #ifdef MC_DEBUG
  printf("generating division questions\n");
  #endif

  if (math_opts->iopts[DIVISION_ALLOWED])
  {
    #ifdef MC_DEBUG
    printf("\nDivision problems");
    #endif
    for (i = math_opts->iopts[MIN_QUOTIENT]; i <= math_opts->iopts[MAX_QUOTIENT]; i++)
    {
      for (j = math_opts->iopts[MIN_DIVISOR]; j <= math_opts->iopts[MAX_DIVISOR]; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (j                                     /* must avoid division by zero: */      
            &&
            validate_question(i * j, j, i))       /* division problems are generated as multiplication */
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->iopts[QUESTION_COPIES]; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 / num2 = ? */
            if (math_opts->iopts[FORMAT_DIV_ANSWER_LAST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i * j, j, MC_OPER_DIV, i, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 / ? = num3 */
            if ((math_opts->iopts[FORMAT_DIV_ANSWER_MIDDLE])
               && (i))      /* This avoids creating indeterminate questions: 0/? = 0 */
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i * j, j, MC_OPER_DIV, i, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? / num2  = num3 */
            if (math_opts->iopts[FORMAT_DIV_ANSWER_FIRST])
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->iopts[MAX_QUESTIONS])
                 && randomly_keep())
              {
                tmp_ptr = create_node(i * j, j, MC_OPER_DIV, i, MC_FORMAT_ANS_FIRST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }
          }
        }
      }
    }
  }

  #ifdef MC_DEBUG
  printf("generating typing practice questions\n");
  #endif

  if (math_opts->iopts[TYPING_PRACTICE_ALLOWED])
  {
    #ifdef MC_DEBUG
    printf("\nTyping problems");
    #endif
    for (i = math_opts->iopts[MIN_TYPING_NUM]; i <= math_opts->iopts[MAX_TYPING_NUM]; i++)
    {
      /* check if max_answer exceeded or if question */
      /* contains undesired negative values:         */
      if (validate_question(i, i, i))
      {  
        /* put in the desired number of copies: */
        for (k = 0; k < math_opts->iopts[QUESTION_COPIES]; k++)
        {
          /* make sure max_questions not exceeded, */
          /* also check if question being randomly kept or discarded: */
          if ((length < math_opts->iopts[MAX_QUESTIONS])
               && randomly_keep())
          {
            tmp_ptr = create_node(i, i, MC_OPER_TYPING_PRACTICE, i, MC_FORMAT_ANS_LAST);
            top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
            end_of_list = tmp_ptr;
            length++; 
          } 
        }
      }
    }
  }
  #ifdef MC_DEBUG
  length = list_length(top_of_list); 
  printf("\nlength before randomization:\t%d", length); 
  #endif

  /*  now shuffle list if desired: */
  if (math_opts->iopts[RANDOMIZE])
  {
    if(!randomize_list(&top_of_list))
    { 
      fprintf(stderr, "Error during list randomization!\n");
      return NULL;
    }
  }

  #ifdef MC_DEBUG
  length = list_length(top_of_list); 
  printf("\nlength after randomization:\t%d", length); 
  printf("\nLeaving generate_list()\n");
  #endif

  return top_of_list;
}
#endif

/* this is used by generate_list to see if a possible question */
/* meets criteria to be added to the list or not:              */
int validate_question(int n1, int n2, int n3)
{
  /* make sure none of values exceeds max_answer using absolute */
  /* value comparison:                                          */
  if (abs_value(n1) > abs_value(math_opts->iopts[MAX_ANSWER])
   || abs_value(n2) > abs_value(math_opts->iopts[MAX_ANSWER])
   || abs_value(n3) > abs_value(math_opts->iopts[MAX_ANSWER]))
  {
    return 0;
  }
  /* make sure none of values are negative if negatives not allowed: */
  if (!math_opts->iopts[ALLOW_NEGATIVES])
  {
    if (n1 < 0 || n2 < 0 || n3 < 0)
    {
      return 0;
    }
  }
  return 1;  
}

/* create a new node and return a pointer to it */
MC_MathQuestion* create_node(int n1, int n2, int op, int ans, int f)
{
  MC_MathQuestion* ptr = NULL;

  ptr = (MC_MathQuestion*)malloc(sizeof(MC_MathQuestion));

  if (!ptr)
  {
    fprintf(stderr, "create_node() - malloc() failed!\n");
    return NULL;
  }
/*
  ptr->card.num1 = n1;
  ptr->card.num2 = n2;  
  ptr->card.num3 = ans;
  ptr->card.operation = op;
  ptr->card.format = f;
*/
  ptr->card = MC_AllocateFlashcard();
  ptr->next = NULL;
  ptr->previous = NULL;
 
  snprintf(ptr->card.formula_string, max_formula_size, "%d %c %d = ?",
           n1, op < MC_NUM_OPERS ? operchars[op] : '\0', n2);
  snprintf(ptr->card.answer_string, max_formula_size, "%d", ans);
  ptr->card.difficulty = 25 * (op + 1);


  /* ptr should now point to a properly constructed node: */
  return ptr;
}


#ifdef MC_DEBUG
/* a "copy constructor", so to speak */
/* FIXME should properly return newly allocated list if more than one node DSB */
MC_MathQuestion* create_node_copy(MC_MathQuestion* other)
{
  MC_MathQuestion* ret = malloc(sizeof(MC_MathQuestion) );
  if (ret)
    copy_card(&other->card, &ret->card);
  return ret;
}
#endif

MC_MathQuestion* create_node_from_card(MC_FlashCard* flashcard)
{
  MC_MathQuestion* ret = allocate_node();
  copy_card(flashcard, &ret->card);
  return ret;
}

#ifdef MC_DEBUG
/* FIXME take care of strings */

MC_FlashCard create_card_from_node(MC_MathQuestion* node)
{
  MC_FlashCard fc;
  if (!node)
    return DEFAULT_CARD;
  fc = MC_AllocateFlashcard();
  copy_card(&node->card, &fc);
  return fc;
}
#endif


/* FIXME take care of strings */
/* this one copies the contents, including pointers; both nodes must be allocated */
int copy_node(MC_MathQuestion* original, MC_MathQuestion* copy)
{
  if (!original)
  {
    fprintf(stderr, "\nIn copy_node(): invalid 'original' pointer arg.\n");
    return 0;
  }  
  if (!copy)
  {
    fprintf(stderr, "\nIn copy_node(): invalid 'copy' pointer arg.\n");
    return 0;
  }  

  copy_card(&original->card, &copy->card);
 
  copy->next = original->next;
  copy->previous = original->previous;
  copy->randomizer = original->randomizer;
  return 1;
}




/* this puts the node into the list AFTER the node pointed to by current */
/* and returns a pointer to the top of the modified list  */
MC_MathQuestion* insert_node(MC_MathQuestion* first, MC_MathQuestion* current, MC_MathQuestion* new_node)
{
  /* return pointer to list unchanged if new_node doesn't exist*/
  if (!new_node)
    return first;
  /* if current doesn't exist, new_node is first */
  if (!current)
  {
    new_node->previous = 0;
    new_node->next =0;
    first = new_node;
    return first;
  }

  if (current->next)  /* avoid error if at end of list */
    current->next->previous = new_node;
  new_node->next = current->next;
  current->next = new_node;
  new_node->previous = current;
  return first;
}



/* adds the new node to the end of the list */
MC_MathQuestion* append_node(MC_MathQuestion* list, MC_MathQuestion* new_node)
{
  MC_MathQuestion* ptr;
  /* return pointer to list unchanged if new_node doesn't exist*/
  if (!new_node)
  {
    return list;
  }

  /* if list does not exist, new_node is the first (and only) node */
  if (!list)
  {
    return new_node;
  }
  /* otherwise, go to end of list */
  ptr = list;
  while (ptr->next)
  {
    ptr = ptr->next;
  }

  ptr->next = new_node;
  new_node->previous = ptr;
  new_node->next = 0;
  return list;
}



/* this takes the node out of the list but does not delete it */
/* and returns a pointer to the top of the modified list  */
MC_MathQuestion* remove_node(MC_MathQuestion* first, MC_MathQuestion* n)
{
  if (!n || !first)
    return first;
  /* special case if first node being removed */
  if (n == first)
     first = first->next;

  if (n->previous)
    n->previous->next = n->next;
  if (n->next)
      n->next->previous = n->previous;
  n->previous = 0;
  n->next = 0;
  return first;
}



/* frees memory for entire list and returns null pointer */
MC_MathQuestion* delete_list(MC_MathQuestion* list)
{
  MC_MathQuestion* tmp_ptr;
  while (list)
  {
    tmp_ptr = list->next; 
    free_node (list);
    list = tmp_ptr;
  }
  return list;
}



void print_list(FILE* fp, MC_MathQuestion* list)
{
  if (!list)
  {
    fprintf(fp, "\nprint_list(): list empty or pointer invalid\n");
    return;
  }

  {
    MC_MathQuestion* ptr = list;
    while (ptr)
    {
      print_node(fp, ptr);
      ptr = ptr->next;
    }
  }
}

void print_vect_list(FILE* fp, MC_MathQuestion** vect, int length)
{
  if (!vect)
  {
    fprintf(fp, "\nprint_vect_list(): list empty or pointer invalid\n");
    return;
  }

  {
    int i = 0;
    for(i = 0; i < length; i++) 
      print_node(fp, vect[i]);
  }
  fprintf(stderr, "Leaving print_vect_list()\n");
}

/* Don't need this much now that formula_string part of card struct:  */
void print_node(FILE* fp, MC_MathQuestion* ptr)
{
  if (!ptr || !fp)
  {
    return;
  }

  fprintf(fp, "%s\n", ptr->card.formula_string);
  /*fprintf(fp, "randomizer = %d\n", ptr->randomizer);*/
}  


#ifdef MC_DEBUG
void print_card(MC_FlashCard card)
{
  printf("\nprint_card():");
  printf("formula_string = %s, answer_string = %s\n", 
         card.formula_string, card.answer_string);
}
#endif


#ifdef MC_DEBUG
/* This sends the values of all "global" counters and the */
/* lengths of the question lists to stdout - for debugging */
void print_counters(void)
{
  printf("\nquest_list_length = \t%d", quest_list_length);
  printf("\nlist_length(question_list) = \t%d", list_length(question_list));
  printf("\nstarting_length = \t%d", starting_length);
  printf("\nunanswered = \t%d", unanswered);
  printf("\nanswered_correctly = \t%d", answered_correctly);
  printf("\nanswered_wrong = \t%d", answered_wrong);
  printf("\nlist_length(wrong_quests) = \t%d", list_length(wrong_quests));
  printf("\nquestions_pending = \t%d", questions_pending);
}
#endif

int list_length(MC_MathQuestion* list)
{
  int length = 0;
  while (list)
  {
    length++;
    list = list->next;
  }
  return length;
}






/* This is a new implementation written in an attempt to avoid       */
/* the O(n^2) performance problems seen with the old randomization   */
/* function. The list is created as a vector, but is for now still   */
/* made a linked list to minimize changes needed elsewhere.          */
/* The argument is a pointer to the top of the old list.  This extra */
/* level of indirection allows the list to be shuffled "in-place".   */
/* The function returns 1 if successful, 0 on errors.                */

static int randomize_list(MC_MathQuestion** old_list)
{
  MC_MathQuestion* old_tmp = *old_list;
  MC_MathQuestion** tmp_vect = NULL;

  int i = 0;
  int old_length = list_length(old_tmp);

  /* set random seed: */
  srand(time(0));  


  /* Allocate vector and set ptrs to nodes in old list: */

  /* Allocate a list of pointers, not space for the nodes themselves: */
  tmp_vect = (MC_MathQuestion**)malloc(sizeof(MC_MathQuestion*) * old_length);
  /* Set each pointer in the vector to the corresponding node: */
  for (i = 0; i < old_length; i++)
  {
    tmp_vect[i] = old_tmp;
    tmp_vect[i]->randomizer = rand();
    old_tmp = old_tmp->next;
  }

  /* Now simply sort on 'tmp_vect[i]->randomizer' to shuffle list: */
  qsort(tmp_vect, old_length,
        sizeof(MC_MathQuestion*),
        comp_randomizer);

  /* Re-create pointers to provide linked-list functionality:      */
  /* (stop at 'old_length-1' because we dereference tmp_vect[i+1]) */
  for(i = 0; i < old_length - 1; i++)
  {
    if (!tmp_vect[i])
    {
      fprintf(stderr, "Invalid pointer!\n");
      return 0;
    }
    tmp_vect[i]->next = tmp_vect[i+1];
    tmp_vect[i+1]->previous = tmp_vect[i];
  }
  /* Handle end cases: */
  tmp_vect[0]->previous = NULL;
  tmp_vect[old_length-1]->next = NULL;

  /* Now arrange for arg pointer to indirectly point to first element! */
  *old_list = tmp_vect[0];
  free(tmp_vect);
  return 1;
}



/* This is needed for qsort(): */
int comp_randomizer (const void* a, const void* b)
{

  int int1 = (*(const struct MC_MathQuestion **) a)->randomizer;
  int int2 = (*(const struct MC_MathQuestion **) b)->randomizer;

  if (int1 > int2)
    return 1;
  else if (int1 == int2)
    return 0;
  else
    return -1;
}

MC_MathQuestion* pick_random(int length, MC_MathQuestion* list)
{
  int i;
  int rand_node;

  /* set random seed DSB */
  srand(time(0));  

  /* if length is zero, get out to avoid divide-by-zero error */
  if (0 == length)
  {
    return list;
  }

  rand_node = rand() % length;

  for (i=1; i < rand_node; i++)
  {
    if (list)
     list = list->next;
  }

  return list;
}

/* compares fields other than pointers */
int compare_node(MC_MathQuestion* first, MC_MathQuestion* other)
{
  if (!first || !other)
    return 0;
  if (compare_card(&first->card, &first->card) ) //cards are equal
    return 1;
  else
    return 0;
}

/* check to see if list already contains an identical node */
int already_in_list(MC_MathQuestion* list, MC_MathQuestion* ptr)
{
  if (!list || !ptr)
    return 0;

  while (list)
  {
    if (compare_node(list, ptr))
      return 1;
    list = list->next;
  }
  return 0;
}

/* to prevent option settings in math_opts from getting set to */
/* values other than 0 or 1                                    */
int int_to_bool(int i)
{
  if (i)
    return 1;
  else
    return 0;
}

/* prevent values from getting into math_opts that are outside */
/* the range that can be handled by the program (i.e. more     */
/* than three digits; also disallow negatives if that has been */
/* selected.                                                   */
int sane_value(int i)
{
  if (i > MC_GLOBAL_MAX)
    i = MC_GLOBAL_MAX;
  else if (i < -MC_GLOBAL_MAX)
    i = -MC_GLOBAL_MAX;
  
  if (i < 0 
   && math_opts
   && !math_opts->iopts[ALLOW_NEGATIVES])
  {
    i = 0;
  }

  return i;
}

int abs_value(int i)
{
  if (i > 0)
    return i;
  else
    return -i;
}


/* Returns true at probability set by math_opts->fraction_to_keep */
int randomly_keep(void)
{
  int random;

  if (!math_opts)
    return 0;

  /* Skip random number generation if keeping all (default) */
  if (1 == math_opts->fraction_to_keep)
    return 1;

  random = rand() % 1000;

  if (random < (math_opts->fraction_to_keep * 1000))
    return 1;
  else
    return 0;
}

/* Compares two floats (needed for sorting in MC_MedianTimePerQuestion) */
int floatCompare(const void *v1,const void *v2)
{
  float f1,f2;

  f1 = *((float *) v1);
  f2 = *((float *) v2);

  if (f1 < f2)
    return -1;
  else if (f1 > f2)
    return 1;
  else
    return 0;
}

#ifdef MC_USE_NEWARC

/****************************************************
Experimental functions for new mathcards architecture
****************************************************/

/* allocate space for an MC_Flashcard */
MC_FlashCard MC_AllocateFlashcard(void)
{
  MC_FlashCard ret;  
  ret.formula_string = malloc(max_formula_size * sizeof(char));
  ret.answer_string = malloc(max_answer_size * sizeof(char));
  if (!ret.formula_string || !ret.answer_string)
    {
    free(ret.formula_string);
    free(ret.answer_string);
    printf("Couldn't allocate space for a new flashcard!\n");
    ret = DEFAULT_CARD;
    }
  return ret;
}
void MC_FreeFlashcard(MC_FlashCard* fc)
{
  if (!fc) 
    return;
#ifndef MC_DEBUG
  mcdprintf("Freeing formula_string\n");
  if (fc->formula_string)
    {
    free(fc->formula_string);
    fc->formula_string = NULL;
    }
  mcdprintf("Freeing answer_string\n");
  if (fc->answer_string)
    {
    free(fc->answer_string);
    fc->answer_string = NULL;
    }
#endif
}

void copy_card(const MC_FlashCard* src, MC_FlashCard* dest)
{
  if (!src || !dest)
    return;
  mcdprintf("Copying '%s' to '%s', ", src->formula_string,dest->formula_string);
  mcdprintf("copying '%s' to '%s'\n", src->answer_string, dest->answer_string);
  strncpy(dest->formula_string, src->formula_string, max_formula_size);
  strncpy(dest->answer_string, src->answer_string, max_answer_size);
  dest->answer = src->answer;
  dest->difficulty = src->difficulty;
}

void free_node(MC_MathQuestion* mq) //no, not that freenode.
{
  if (!mq)
    return;
  MC_FreeFlashcard(&mq->card);
  free(mq);
}

MC_MathQuestion* allocate_node()
{
  MC_MathQuestion* ret = NULL;
  ret = malloc(sizeof(MC_MathQuestion) );
  if (!ret)
    printf("Could not allocate space for a new node!\n");
  else  
    ret->card = MC_AllocateFlashcard();
  return ret;
}

/* 
The function that does the central dirty work pertaining to flashcard
creation. Extensible to just about any kind of math problem, perhaps
with the exception of those with multiple answers, such as "8 + 2 > ?"
Simply specify how the problem is presented to the user, and the
answer the game should look for, as strings.
*/
MC_FlashCard generate_random_flashcard(void)
{
  int num;
  int length;
  MC_ProblemType pt;
  MC_FlashCard ret;
  
  mcdprintf("Entering generate_random_flashcard()\n");
  pt = rand() % MC_NUM_PTYPES;
  if (pt == MC_PT_TYPING) //typing practice
  {
    mcdprintf("Generating typing question\n");
    ret = MC_AllocateFlashcard();
    num = rand() % (MC_GetOpt(MAX_TYPING_NUM) - MC_GetOpt(MIN_TYPING_NUM) )
                + MC_GetOpt(MIN_TYPING_NUM);
    snprintf(ret.formula_string, max_formula_size, "%d", num);
    snprintf(ret.answer_string, max_answer_size, "%d", num);
    ret.difficulty = 10;
  }
  else //if (pt == MC_PT_ARITHMETIC)
  {
    length = rand() % (MC_GetOpt(MAX_FORMULA_NUMS)-MC_GetOpt(MIN_FORMULA_NUMS) )
                    +  MC_GetOpt(MIN_FORMULA_NUMS);
    mcdprintf("Generating question of length %d", length);
    ret = generate_random_ooo_card_of_length(length);
    
  }
  //TODO comparison problems (e.g. "6 ? 9", "<")
  
  mcdprintf("Exiting generate_random_flashcard()\n");
  
  return ret;
}

/* 
Recursively generate an order of operations problem. Hopefully this won't
raise performance issues.
*/
MC_FlashCard generate_random_ooo_card_of_length(int length)
{
  int r1 = 0;
  int r2 = 0;
  int ans = 0;
  char* tempstr[max_formula_size];
  MC_FlashCard ret;
  MC_Operation op;
    
  printf(".");
  if (length > MAX_FORMULA_NUMS)
    return DEFAULT_CARD;
  if (length <= 2)
  {
    ret = MC_AllocateFlashcard();
    for (op = rand() % MC_NUM_OPERS; //pick a random operation
         MC_GetOpt(op + ADDITION_ALLOWED) == 0; //make sure it's allowed
         op = rand() % MC_NUM_OPERS); 
         
    mcdprintf("Operation is %c\n", operchars[op]);
    if (op == MC_OPER_ADD)
    {
      r1 = rand() % (math_opts->iopts[MAX_AUGEND] - math_opts->iopts[MIN_AUGEND]) + math_opts->iopts[MIN_AUGEND];
      r2 = rand() % (math_opts->iopts[MAX_ADDEND] - math_opts->iopts[MIN_ADDEND]) + math_opts->iopts[MIN_ADDEND];
      ans = r1 + r2;
    }
    else if (op == MC_OPER_SUB)
    {
      r1 = rand() % (math_opts->iopts[MAX_MINUEND] - math_opts->iopts[MIN_MINUEND]) + math_opts->iopts[MIN_MINUEND];
      r2 = rand() % (math_opts->iopts[MAX_SUBTRAHEND] - math_opts->iopts[MIN_SUBTRAHEND]) + math_opts->iopts[MIN_SUBTRAHEND];
      ans = r1 - r2;
    }
    else if (op == MC_OPER_MULT)
    {
      r1 = rand() % (math_opts->iopts[MAX_MULTIPLIER] - math_opts->iopts[MIN_MULTIPLIER]) + math_opts->iopts[MIN_MULTIPLIER];
      r2 = rand() % (math_opts->iopts[MAX_MULTIPLICAND] - math_opts->iopts[MIN_MULTIPLICAND]) + math_opts->iopts[MIN_MULTIPLICAND];
      ans = r1 * r2;
    }
    else if (op == MC_OPER_DIV)
    {
      ans = rand() % (math_opts->iopts[MAX_QUOTIENT] - math_opts->iopts[MIN_QUOTIENT]) + math_opts->iopts[MIN_QUOTIENT];
      r2 = rand() % (math_opts->iopts[MAX_DIVISOR] - math_opts->iopts[MIN_DIVISOR]) + math_opts->iopts[MIN_DIVISOR];
      r1 = ans * r2;
    }
    else
      mcdprintf("Invalid operator: value %d\n", op);
 
    mcdprintf("Constructing answer_string\n");     
    snprintf(ret.answer_string, max_answer_size, "%d", ans);
    mcdprintf("Constructing formula_string\n");
    snprintf(ret.formula_string, max_formula_size, "%d %c %d", 
             r1, operchars[op], r2);
  }
  else //recurse
  {
    ret = generate_random_ooo_card_of_length(length - 1);
    
    if (strchr(ret.formula_string, '+') || strchr(ret.formula_string, '-') )
    {
      //if the expression has addition or subtraction, we can't assume that 
      //introducing multiplication or division will produce a predictable
      //result, so we'll limit ourselves to more addition/subtraction
      for (op = rand() % 2 ? MC_OPER_ADD : MC_OPER_SUB;
           MC_GetOpt(op + ADDITION_ALLOWED) == 0;
           op = rand() % 2 ? MC_OPER_ADD : MC_OPER_SUB);
    }
    else
    {
      //the existing expression can be treated as a number in itself, so we
      //can do anything to it and be confident of the result.
      for (op = rand() % MC_NUM_OPERS; //pick a random operation
         MC_GetOpt(op + ADDITION_ALLOWED) == 0; //make sure it's allowed
         op = rand() % MC_NUM_OPERS); 
    }
    
    //next append or prepend the new number
    if (op == MC_OPER_SUB || op == MC_OPER_DIV || //noncommutative, append only
        rand() % 2)
    {  
      if (op == MC_OPER_SUB)
        r1 = rand() % (math_opts->iopts[MAX_SUBTRAHEND] - math_opts->iopts[MIN_SUBTRAHEND]) + math_opts->iopts[MIN_SUBTRAHEND];
      else if (op == MC_OPER_DIV)
        r1 = 1;
      
      sprintf(tempstr, "%c %d", operchars[op], r1);
    }
    else //we're prepending
    {
    
    }
    
  }
  return ret;
}

MC_MathQuestion* generate_list(void)
{
  int i;
  int length = MC_GetOpt(AVG_LIST_LENGTH);
  MC_MathQuestion* list = NULL;
  MC_MathQuestion* end_of_list = NULL;
  MC_MathQuestion* tnode = NULL;
  
  //TODO handle AVG_LIST_LENGTH = 0, i.e. generate all valid questions
  //TODO randomize list length
  
  for (i = 0; i < length; ++i)
  {
    tnode = malloc(sizeof(MC_MathQuestion) );
    tnode->card = generate_random_flashcard();
    list = insert_node(list, end_of_list, tnode);
    end_of_list = tnode;
  }
    
  return list;
}

static int compare_card(const MC_FlashCard* a, const MC_FlashCard* b)
{
  if (strncmp(a->formula_string, b->formula_string, max_formula_size) )
    return 1;
  if (strncmp(a->answer_string, b->answer_string, max_answer_size) )
    return 1;
  if (a->answer != b->answer);
    return 1;
    
  return 0; //the cards are identical
}

/* Public functions */

unsigned int MC_MapTextToIndex(const char* text)
{
  int i;
  for (i = 0; i < NOPTS; ++i)
  {
    mcdprintf("%d: %s", i, MC_OPTION_TEXT[i] );
    if (!strcasecmp(text, MC_OPTION_TEXT[i]) )
      return i;
  }
  printf("Sorry, don't recognize option '%s'\n", text);
  return NOT_VALID_OPTION;
}

//TODO more intuitive function names for access by index vs. by text
void MC_SetOpt(unsigned int index, int val)
{
  if (index >= NOPTS)
  {
    printf("Invalid option index: %du\n", index);
    return;
  }
  math_opts->iopts[index] = val;
}

void MC_SetOp(const char* param, int val)
{
  MC_SetOpt(MC_MapTextToIndex(param), val);
}

int MC_GetOpt(unsigned int index)
{
  if (index >= NOPTS)
  {
    printf("Invalid option index: %du\n", index);
    return MC_MATH_OPTS_INVALID;
  }  
  if (!math_opts)
  {
    printf("Invalid options list!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->iopts[index];
}

int MC_GetOp(const char* param)
{
  return MC_GetOpt(MC_MapTextToIndex(param) );
}

void MC_SetFractionToKeep(float val)
{
  math_opts->fraction_to_keep = val;
}

float MC_GetFractionToKeep(void)
{
  return math_opts->fraction_to_keep;
}

int MC_VerifyOptionListSane(void)
{
  return MC_OPTION_TEXT[NOPTS] == "END_OF_OPTS";
}

int MC_MaxFormulaSize(void)
{
  return max_formula_size;
}

int MC_MaxAnswerSize(void)
{
  return max_answer_size;
}

void MC_ResetFlashCard(MC_FlashCard* fc)
{
  if (!fc || !fc->formula_string || !fc->answer_string)
    return;
  strncpy(fc->formula_string, " ", max_formula_size);
  strncpy(fc->answer_string, " ", max_answer_size);
  fc->answer = 0;
  fc->difficulty = 0;
}

int MC_FlashCardGood(const MC_FlashCard* fc)
{
  return fc && fc->formula_string && fc->answer_string;
}
#endif
