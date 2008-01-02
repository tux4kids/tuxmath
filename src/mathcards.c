/*
*  C Implementation: mathcards.c
*
* 	Description: implementation of backend for a flashcard-type math game. 
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
#include <time.h>

#include "mathcards.h"

/* "Globals" for mathcards.c: */
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

/* For keeping track of timing data */
float* time_per_question_list = NULL;
int length_time_per_question_list = 0;
int length_alloc_time_per_question_list = 0;

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

static MC_MathQuestion* randomize_list(MC_MathQuestion* list);
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

static MC_FlashCard*    create_card_from_node(MC_MathQuestion* node);
#endif

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
  #ifdef MC_DEBUG
  printf("\nEntering MC_Initialize()");
  #endif

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

    #ifdef MC_DEBUG
    printf("\nError: math_opts null or invalid");
    printf("\nLeaving MC_Initialize()\n");
    #endif

    fprintf(stderr, "\nUnable to initialize math_options");
    return 0;
  } 

  /* set general math options */
  math_opts->play_through_list = DEFAULT_PLAY_THROUGH_LIST;
  math_opts->repeat_wrongs = DEFAULT_REPEAT_WRONGS;
  math_opts->copies_repeated_wrongs = DEFAULT_COPIES_REPEATED_WRONGS;
  math_opts->allow_negatives = DEFAULT_ALLOW_NEGATIVES;
  math_opts->max_answer = DEFAULT_MAX_ANSWER;
  math_opts->max_questions = DEFAULT_MAX_QUESTIONS;
  math_opts->question_copies = DEFAULT_QUESTION_COPIES;
  math_opts->randomize = DEFAULT_RANDOMIZE;
  math_opts->fraction_to_keep = DEFAULT_FRACTION_TO_KEEP;
  /* set question formats:  */
  math_opts->format_add_answer_last = DEFAULT_FORMAT_ADD_ANSWER_LAST; 
  math_opts->format_add_answer_first = DEFAULT_FORMAT_ADD_ANSWER_FIRST;
  math_opts->format_add_answer_middle = DEFAULT_FORMAT_ADD_ANSWER_MIDDLE;
  math_opts->format_sub_answer_last = DEFAULT_FORMAT_SUB_ANSWER_LAST;
  math_opts->format_sub_answer_first = DEFAULT_FORMAT_SUB_ANSWER_FIRST;
  math_opts->format_sub_answer_middle = DEFAULT_FORMAT_SUB_ANSWER_MIDDLE;
  math_opts->format_mult_answer_last = DEFAULT_FORMAT_MULT_ANSWER_LAST;
  math_opts->format_mult_answer_first = DEFAULT_FORMAT_MULT_ANSWER_FIRST;
  math_opts->format_mult_answer_middle = DEFAULT_FORMAT_MULT_ANSWER_MIDDLE;
  math_opts->format_div_answer_last = DEFAULT_FORMAT_DIV_ANSWER_LAST;
  math_opts->format_div_answer_first = DEFAULT_FORMAT_DIV_ANSWER_FIRST;
  math_opts->format_div_answer_middle = DEFAULT_FORMAT_DIV_ANSWER_MIDDLE;

  /* set addition options: */
  math_opts->addition_allowed = DEFAULT_ADDITION_ALLOWED;
  math_opts->min_augend = DEFAULT_MIN_AUGEND;
  math_opts->max_augend = DEFAULT_MAX_AUGEND;
  math_opts->min_addend = DEFAULT_MIN_ADDEND;
  math_opts->max_addend = DEFAULT_MAX_ADDEND;
  /* set subtraction options: */
  math_opts->subtraction_allowed = DEFAULT_SUBTRACTION_ALLOWED;
  math_opts->min_minuend = DEFAULT_MIN_MINUEND;
  math_opts->max_minuend = DEFAULT_MAX_MINUEND;
  math_opts->min_subtrahend = DEFAULT_MIN_SUBTRAHEND;
  math_opts->max_subtrahend = DEFAULT_MAX_SUBTRAHEND;
  /* set multiplication options: */
  math_opts->multiplication_allowed = DEFAULT_MULTIPLICATION_ALLOWED;
  math_opts->min_multiplier = DEFAULT_MIN_MULTIPLIER;
  math_opts->max_multiplier = DEFAULT_MAX_MULTIPLIER;
  math_opts->min_multiplicand = DEFAULT_MIN_MULTIPLICAND;
  math_opts->max_multiplicand = DEFAULT_MAX_MULTIPLICAND;
  /* set division options: */
  math_opts->division_allowed = DEFAULT_DIVISION_ALLOWED;
  math_opts->min_divisor = DEFAULT_MIN_DIVISOR;
  math_opts->max_divisor = DEFAULT_MAX_DIVISOR;
  math_opts->min_quotient = DEFAULT_MIN_QUOTIENT;
  math_opts->max_quotient = DEFAULT_MAX_QUOTIENT;
  /* set typing practice options: */
  math_opts->typing_practice_allowed = DEFAULT_TYPING_PRACTICE_ALLOWED;
  math_opts->min_typing_num = DEFAULT_MIN_TYPING_NUM;
  math_opts->max_typing_num = DEFAULT_MAX_TYPING_NUM;

  /* if no negatives to be used, reset any negatives to 0 */
  if (!math_opts->allow_negatives)
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
  #ifdef MC_DEBUG
  printf("\nEntering MC_StartGame()");
  #endif

  /* if math_opts not set up yet, initialize it: */
  if (!initialized)
  {

    #ifdef MC_DEBUG
    printf("\nNot initialized - calling MC_Initialize()");
    #endif  

    MC_Initialize();
  }

  if (!math_opts)  
  {
    #ifdef MC_DEBUG
    printf("\nCould not initialize - bailing out");
    printf("\nLeavinging MC_StartGame()\n");
    #endif

    return 0;
  }
  /* we know math_opts exists if we make it to here */
  
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
    #ifdef MC_DEBUG
    printf("\nGame set up successfully");
    printf("\nLeaving MC_StartGame()\n");
    #endif

    return 1;
  }
  else
  {
    #ifdef MC_DEBUG
    printf("\nGame NOT set up successfully - no valid list");
    printf("\nLeaving MC_StartGame()\n");
    #endif

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
    question_list = randomize_list(wrong_quests);
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
  /* FIXME: could clean this up a bit with a copy_card() function */
  fc->num1 = question_list->card.num1;
  fc->num2 = question_list->card.num2;
  fc->num3 = question_list->card.num3;
  fc->operation = question_list->card.operation;
  fc->format = question_list->card.format;
  strncpy(fc->formula_string, question_list->card.formula_string, MC_FORMULA_LEN);  
  strncpy(fc->answer_string, question_list->card.answer_string, MC_ANSWER_LEN);  

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

  if (!math_opts->play_through_list)
  /* reinsert question into question list at random location */
  {
    #ifdef MC_DEBUG
    printf("\nReinserting question into list");
    #endif

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
    #ifdef MC_DEBUG
    printf("\nNot reinserting question into list");
    #endif
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
  #ifdef MC_DEBUG
  printf("\nEntering MC_NotAnsweredCorrectly()");
  #endif

  if (!fc)
  {
    fprintf(stderr, "\nMC_NotAnsweredCorrectly() passed invalid pointer as argument!\n");

    #ifdef MC_DEBUG
    printf("\nInvalid MC_FlashCard* argument!");
    printf("\nLeaving MC_NotAnsweredCorrectly()\n");
    #endif

    return 0;
  }

  #ifdef MC_DEBUG
  printf("\nQuestion was:");
  print_card(*fc);
  #endif

  answered_wrong++;
  questions_pending--;

  /* add question to wrong_quests list: */
  {
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
  if (math_opts->repeat_wrongs)
  {
    int i;

    #ifdef MC_DEBUG
    printf("\nAdding %d copies to question_list:", math_opts->copies_repeated_wrongs);
    #endif
 
    /* can put in more than one copy (to drive the point home!) */
    for (i = 0; i < math_opts->copies_repeated_wrongs; i++)
    {  
      ptr1 = create_node_from_card(fc);
      ptr2 = pick_random(quest_list_length, question_list);
      question_list = insert_node(question_list, ptr2, ptr1);
      quest_list_length++;
    }
    /* unanswered stays the same if a single copy recycled or */
    /* increases by 1 for each "extra" copy reinserted:       */
    unanswered += (math_opts->copies_repeated_wrongs - 1);
  }
  else
  {
    #ifdef MC_DEBUG
    printf("\nNot repeating wrong answers\n");
    #endif

    /* not repeating questions so list gets shorter:      */
    unanswered--;
  }

  #ifdef MC_DEBUG
  print_counters();
  printf("\nLeaving MC_NotAnswered_Correctly()\n");
  #endif

  return 1;
}
}

/* Tells user interface if all questions have been answered correctly! */
/* Requires that at list contained at least one question to start with */
/* and that wrongly answered questions have been recycled.             */
int MC_MissionAccomplished(void)
{
  if (starting_length
    && math_opts->repeat_wrongs
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


/* Simple Get()- and Set()-style functions for math options settings: */


/* Set general math options: */
void MC_SetMaxAnswer(int max)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMaxAnswer(): math_opts not valid!\n");
    return;
  }
  math_opts->max_answer = sane_value(max);
}


void MC_SetMaxQuestions(int max)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMaxQuestions(): math_opts not valid!\n");
    return;
  }
  if (max < 0)
  {
    fprintf(stderr, "\nMC_SetMaxQuestions(): max_questions cannot be negative!\n");
    return;
  }
  math_opts->max_questions = max;
}

void MC_SetAllowNegatives(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetAllowNegatives(): math_opts not valid!\n");
    return;
  }
  math_opts->allow_negatives = int_to_bool(opt);
  if (!opt)
  {
    clear_negatives();
  }
}


void MC_SetPlayThroughList(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetPlayThroughList(): math_opts not valid!\n");
    return;
  }
  math_opts->play_through_list = int_to_bool(opt);
}


void MC_SetRepeatWrongs(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetRepeatWrongs(): math_opts not valid!\n");
    return;
  }
  math_opts->repeat_wrongs = int_to_bool(opt);
}


void MC_SetCopiesRepeatedWrongs(int copies)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetCopiesRepeatedWrongs(): math_opts not valid!\n");
    return;
  }
  /* number of copies must be between 1 and 10: */
  if (copies < 1)
    copies = 1;
  if (copies > 10)
    copies = 10;
  math_opts->copies_repeated_wrongs = copies;
}



/*NOTE - list can contain more than one format */
void MC_SetFormatAnswerLast(int opt)       /* Enable questions like:  a + b = ?    */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatAnswerLast(): math_opts not valid!\n");
    return;
  }

  MC_SetFormatAddAnswerLast(opt);
  MC_SetFormatSubAnswerLast(opt);
  MC_SetFormatMultAnswerLast(opt);
  MC_SetFormatDivAnswerLast(opt);
} 


void MC_SetFormatAnswerFirst(int opt)      /* Enable questions like:  ? + b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatAnswerFirst(): math_opts not valid!\n");
    return;
  }

  MC_SetFormatAddAnswerFirst(opt);
  MC_SetFormatSubAnswerFirst(opt);
  MC_SetFormatMultAnswerFirst(opt);
  MC_SetFormatDivAnswerFirst(opt);
}

 
void MC_SetFormatAnswerMiddle(int opt)     /* Enable questions like:  a + ? = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatAnswerMiddle(): math_opts not valid!\n");
    return;
  }

  MC_SetFormatAddAnswerMiddle(opt);
  MC_SetFormatSubAnswerMiddle(opt);
  MC_SetFormatMultAnswerMiddle(opt);
  MC_SetFormatDivAnswerMiddle(opt);
} 



/* Addition-specific question formats: */
void MC_SetFormatAddAnswerLast(int opt)       /* Enable questions like:  a + b = ?    */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatAddAnswerLast(): math_opts not valid!\n");
    return;
  }
  math_opts->format_add_answer_last = int_to_bool(opt);
} 


void MC_SetFormatAddAnswerFirst(int opt)      /* Enable questions like:  ? + b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatAddAnswerFirst(): math_opts not valid!\n");
    return;
  }
  math_opts->format_add_answer_first = int_to_bool(opt);
}

 
void MC_SetFormatAddAnswerMiddle(int opt)     /* Enable questions like:  a + ? = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatAddAnswerMiddle(): math_opts not valid!\n");
    return;
  }
  math_opts->format_add_answer_middle = int_to_bool(opt);
} 



/* Subtraction-specific question formats: */
void MC_SetFormatSubAnswerLast(int opt)       /* Enable questions like:  a - b = ?    */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatSubAnswerLast(): math_opts not valid!\n");
    return;
  }
  math_opts->format_sub_answer_last = int_to_bool(opt);
} 


void MC_SetFormatSubAnswerFirst(int opt)      /* Enable questions like:  ? - b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatSubAnswerFirst(): math_opts not valid!\n");
    return;
  }
  math_opts->format_sub_answer_first = int_to_bool(opt);
}

 
void MC_SetFormatSubAnswerMiddle(int opt)     /* Enable questions like:  a - ? = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatSubAnswerMiddle(): math_opts not valid!\n");
    return;
  }
  math_opts->format_sub_answer_middle = int_to_bool(opt);
} 



/* Multiplication-specific question formats: */
void MC_SetFormatMultAnswerLast(int opt)       /* Enable questions like:  a * b = ?    */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatMultAnswerLast(): math_opts not valid!\n");
    return;
  }
  math_opts->format_mult_answer_last = int_to_bool(opt);
} 


void MC_SetFormatMultAnswerFirst(int opt)      /* Enable questions like:  ? * b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatMultAnswerFirst(): math_opts not valid!\n");
    return;
  }
  math_opts->format_mult_answer_first = int_to_bool(opt);
}

 
void MC_SetFormatMultAnswerMiddle(int opt)     /* Enable questions like:  a * ? = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatMultAnswerMiddle(): math_opts not valid!\n");
    return;
  }
  math_opts->format_mult_answer_middle = int_to_bool(opt);
} 


/* Division-specific question formats: */
void MC_SetFormatDivAnswerLast(int opt)       /* Enable questions like:  a / b = ?    */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatDivAnswerLast(): math_opts not valid!\n");
    return;
  }
  math_opts->format_div_answer_last = int_to_bool(opt);
} 


void MC_SetFormatDivAnswerFirst(int opt)      /* Enable questions like:  ? / b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatDivAnswerFirst(): math_opts not valid!\n");
    return;
  }
  math_opts->format_div_answer_first = int_to_bool(opt);
}

 
void MC_SetFormatDivAnswerMiddle(int opt)     /* Enable questions like:  a / ? = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetFormatDivAnswerMiddle(): math_opts not valid!\n");
    return;
  }
  math_opts->format_div_answer_middle = int_to_bool(opt);
} 



void MC_SetQuestionCopies(int copies)      /* how many times each question is put in list */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetQuestionCopies(): math_opts not valid!\n");
    return;
  }
  /* number of copies must be between 1 and 10: */
  if (copies < 1)
    copies = 1;
  if (copies > 10)
    copies = 10;
  math_opts->question_copies = copies;
}


void MC_SetRandomize(int opt)   
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetRandomize(): math_opts not valid!\n");
    return;
  }
  math_opts->randomize = int_to_bool(opt);
} 

void MC_SetFractionToKeep(float fract)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetRandomize(): math_opts not valid!\n");
    return;
  }
  /* must be between 0 and 1: */
  if (fract < 0)
    fract = 0;
  if (fract > 1)
    fract = 1;
  math_opts->fraction_to_keep = fract;
}


/* Set math operations to be used in game: */
void MC_SetAddAllowed(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetAddAllowed(): math_opts not valid!\n");
    return;
  }
  math_opts->addition_allowed = int_to_bool(opt);
}


void MC_SetSubAllowed(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetSubAllowed(): math_opts not valid!\n");
    return;
  }
  math_opts->subtraction_allowed = int_to_bool(opt);
}


void MC_SetMultAllowed(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMultAllowed(): math_opts not valid!\n");
    return;
  }
  math_opts->multiplication_allowed = int_to_bool(opt);
}


void MC_SetDivAllowed(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetDivAllowed(): math_opts not valid!\n");
    return;
  }
  math_opts->division_allowed = int_to_bool(opt);
}


void MC_SetTypingAllowed(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetTypingAllowed(): math_opts not valid!\n");
    return;
  }
  math_opts->typing_practice_allowed = int_to_bool(opt);
}



/* Set min and max for addition: */
void MC_SetAddMin(int opt)
{
  MC_SetAddMinAugend(opt);
  MC_SetAddMinAddend(opt);
}


void MC_SetAddMinAugend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetAddMinAugend(): math_opts not valid!\n");
    return;
  }
  math_opts->min_augend = sane_value(opt);
}


void MC_SetAddMinAddend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetAddMinAddend(): math_opts not valid!\n");
    return;
  }
  math_opts->min_addend = sane_value(opt);
}


void MC_SetAddMax(int opt)
{
  MC_SetAddMaxAugend(opt);
  MC_SetAddMaxAddend(opt);
}


void MC_SetAddMaxAugend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetAddMaxAugend(): math_opts not valid!\n");
    return;
  }
  math_opts->max_augend = sane_value(opt);
}


void MC_SetAddMaxAddend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetAddMaxAddend(): math_opts not valid!\n");
    return;
  }
  math_opts->max_addend = sane_value(opt);
}




/* Set min and max for subtraction: */
void MC_SetSubMin(int opt)
{
  MC_SetSubMinMinuend(opt);
  MC_SetSubMinSubtrahend(opt);
}


void MC_SetSubMinMinuend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MC_SetSubMinMinuend(): math_opts not valid!\n");
    return;
  }
  math_opts->min_minuend = sane_value(opt);
}


void MC_SetSubMinSubtrahend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetSubMinSubtrahend(): math_opts not valid!\n");
    return;
  }
  math_opts->min_subtrahend = sane_value(opt);
}


void MC_SetSubMax(int opt)
{
  MC_SetSubMaxMinuend(opt);
  MC_SetSubMaxSubtrahend(opt);
}


void MC_SetSubMaxMinuend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetSubMaxMinuend(): math_opts not valid!\n");
    return;
  }
  math_opts->max_minuend = sane_value(opt);
}


void MC_SetSubMaxSubtrahend(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetSubMaxSubtrahend(): math_opts not valid!\n");
    return;
  }
  math_opts->max_subtrahend = sane_value(opt);
}




/* Set min and max for multiplication: */
void MC_SetMultMin(int opt)
{
  MC_SetMultMinMultiplier(opt);
  MC_SetMultMinMultiplicand(opt);
}


void MC_SetMultMinMultiplier(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMultMinMultiplier(): math_opts not valid!\n");
    return;
  }
  math_opts->min_multiplier = sane_value(opt);
}


void MC_SetMultMinMultiplicand(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMultMinMultiplicand(): math_opts not valid!\n");
    return;
  }
  math_opts->min_multiplicand = sane_value(opt);
}


void MC_SetMultMax(int opt)
{
  MC_SetMultMaxMultiplier(opt);
  MC_SetMultMaxMultiplicand(opt);
}


void MC_SetMultMaxMultiplier(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMultMaxMultiplier(): math_opts not valid!\n");
    return;
  }
  math_opts->max_multiplier = sane_value(opt);
}


void MC_SetMultMaxMultiplicand(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetMultMaxMultiplicand(): math_opts not valid!\n");
    return;
  }
  math_opts->max_multiplicand = sane_value(opt);
}




/* Set min and max for division: */
void MC_SetDivMin(int opt)
{
  MC_SetDivMinDivisor(opt);
  MC_SetDivMinQuotient(opt);
}


void MC_SetDivMinDivisor(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetDivMinDivisor(): math_opts not valid!\n");
    return;
  }
  math_opts->min_divisor = sane_value(opt);
}


void MC_SetDivMinQuotient(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetDivMinQuotient(): math_opts not valid!\n");
    return;
  }
  math_opts->min_quotient = sane_value(opt);
}


void MC_SetDivMax(int opt)
{
  MC_SetDivMaxDivisor(opt);
  MC_SetDivMaxQuotient(opt);
}


void MC_SetDivMaxDivisor(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetDivMaxDivisor(): math_opts not valid!\n");
    return;
  }
  math_opts->max_divisor = sane_value(opt);
}


void MC_SetDivMaxQuotient(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetDivMaxQuotient(): math_opts not valid!\n");
    return;
  }
  math_opts->max_quotient = sane_value(opt);
}


/* Set min and max for division: */
void MC_SetTypeMin(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetTypeMin(): math_opts not valid!\n");
    return;
  }
  math_opts->min_typing_num = sane_value(opt);
}


void MC_SetTypeMax(int opt)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SetTypeMax(): math_opts not valid!\n");
    return;
  }
  math_opts->max_typing_num = sane_value(opt);
}


/*"Get" type methods to query option parameters */

/* Query general math options: */
int MC_MaxAnswer(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MaxAnswer(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_answer;
}


int MC_MaxQuestions(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MaxQuestions(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_questions;
}


int MC_AllowNegatives(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_AllowNegatives(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->allow_negatives;
}


int MC_PlayThroughList(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_PlayThroughList(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->play_through_list;
}


int MC_RepeatWrongs(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_RepeatWrongs(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->repeat_wrongs;
}


int MC_CopiesRepeatedWrongs(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_CopiesRepeatedWrongs(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->copies_repeated_wrongs;
}


float MC_FractionToKeep(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FractionToKeep(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->fraction_to_keep;
}



int MC_FormatAddAnswerLast(void)      /* a + b = ?                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatAddAnswerLast(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_add_answer_last;
} 


int MC_FormatAddAnswerFirst(void)     /* ? + b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatAddAnswerFirst(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_add_answer_first;
} 


int MC_FormatAddAnswerMiddle(void)    /* a + ? = c                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatAddAnswerMiddle(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_add_answer_middle;
} 


int MC_FormatSubAnswerLast(void)      /* a - b = ?                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatSubAnswerLast(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_sub_answer_last;
} 


int MC_FormatSubAnswerFirst(void)     /* ? - b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatSubAnswerFirst(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_sub_answer_first;
} 


int MC_FormatSubAnswerMiddle(void)    /* a - ? = c                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatSubAnswerMiddle(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_sub_answer_middle;
} 

int MC_FormatMultAnswerLast(void)      /* a * b = ?                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatMultAnswerLast(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_mult_answer_last;
} 


int MC_FormatMultAnswerFirst(void)     /* ? * b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatMultAnswerFirst(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_mult_answer_first;
} 


int MC_FormatMultAnswerMiddle(void)    /* a * ? = c                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatMultAnswerMiddle(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_mult_answer_middle;
} 


int MC_FormatDivAnswerLast(void)      /* a / b = ?                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatDivAnswerLast(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_div_answer_last;
} 


int MC_FormatDivAnswerFirst(void)     /* ? / b = c   */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatDivAnswerFirst(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_div_answer_first;
} 


int MC_FormatDivAnswerMiddle(void)    /* a / ? = c                                               */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_FormatAnswerMiddle(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->format_add_answer_middle;
} 



int MC_QuestionCopies(void)         /* how many times each question is put in list */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_QuestionCopies(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->question_copies;
} 


int MC_Randomize(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_Randomize(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->randomize;
} 



/* Query the allowed math operations: */
int MC_AddAllowed(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_AddAllowed(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->addition_allowed;
}


int MC_SubAllowed(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SubAllowed(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->subtraction_allowed;
}


int MC_MultAllowed(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MultAllowed(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->multiplication_allowed;
}


int MC_DivAllowed(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_DivAllowed(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->division_allowed;
}


int MC_TypingAllowed(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_TypeAllowed(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->typing_practice_allowed;
}


/* Query min and max for addition: */
int MC_AddMinAugend(void)               /* the "augend" is the first addend i.e. "a" in "a + b = c" */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_AddMinAugend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_augend;
}


int MC_AddMinAddend(void)               /* options for the other addend */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_AddMinAddend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_addend;
}


int MC_AddMaxAugend(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_AddMaxAugend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_augend;
}


int MC_AddMaxAddend(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_AddMaxAddend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_addend;
}



/* Query min and max for subtraction: */
int MC_SubMinMinuend(void)              /* minuend - subtrahend = difference */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SubMinMinuend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_minuend;
}


int MC_SubMinSubtrahend(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SubMinSubtrahend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_subtrahend;
}



int MC_SubMaxMinuend(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SubMaxMinuend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_minuend;
}



int MC_SubMaxSubtrahend(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_SubMaxSubtrahend(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_subtrahend;
}



/* Query min and max for multiplication: */
int MC_MultMinMultiplier(void)          /* multiplier * multiplicand = product */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MultMinMultiplier(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_multiplier;
}


int MC_MultMinMultiplicand(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MultMinMultiplicand(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_multiplicand;
}



int MC_MultMaxMultiplier(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MultMaxMultiplier(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_multiplier;
}



int MC_MultMaxMultiplicand(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_MultMaxMultiplicand(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_multiplicand;
}



/* Query min and max for division: */
int MC_DivMinDivisor(void)             /* dividend/divisor = quotient */
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_DivMinDivisor(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_divisor;
}


int MC_DivMinQuotient(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_DivMinQuotient(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_quotient;
}


int MC_DivMaxDivisor(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_DivMaxDivisor(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_divisor;
}


int MC_DivMaxQuotient(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_DivMaxQuotient(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_quotient;
}


/* Query min and max for typing practice: */
int MC_TypeMin(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_TypeMin(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->min_typing_num;
}


int MC_TypeMax(void)
{
  if (!math_opts)
  {
    fprintf(stderr, "\nMC_TypeMax(): math_opts not valid!\n");
    return MC_MATH_OPTS_INVALID;
  }
  return math_opts->max_typing_num;
}


/* prints struct to file */
void MC_PrintMathOptions(FILE* fp, int verbose)
{
  #ifdef MC_DEBUG
  printf("\nEntering MC_PrintMathOptions()\n");
  #endif

  /* bail out if no struct */
  if (!math_opts)
  {
    fprintf(stderr, "\nMath Options struct does not exist!\n");
    return;
  }

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
  fprintf (fp, "play_through_list = %d\n", math_opts->play_through_list);
  fprintf (fp, "question_copies = %d\n", math_opts->question_copies);
  fprintf (fp, "repeat_wrongs = %d\n", math_opts->repeat_wrongs);
  fprintf (fp, "copies_repeated_wrongs = %d\n", math_opts->copies_repeated_wrongs);

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
  fprintf (fp, "format_add_answer_last = %d\n", math_opts->format_add_answer_last);
  fprintf (fp, "format_add_answer_first = %d\n", math_opts->format_add_answer_first);
  fprintf (fp, "format_add_answer_middle = %d\n", math_opts->format_add_answer_middle);
  fprintf (fp, "format_sub_answer_last = %d\n", math_opts->format_sub_answer_last);
  fprintf (fp, "format_sub_answer_first = %d\n", math_opts->format_sub_answer_first);
  fprintf (fp, "format_sub_answer_middle = %d\n", math_opts->format_sub_answer_middle);
  fprintf (fp, "format_mult_answer_last = %d\n", math_opts->format_mult_answer_last);
  fprintf (fp, "format_mult_answer_first = %d\n", math_opts->format_mult_answer_first);
  fprintf (fp, "format_mult_answer_middle = %d\n", math_opts->format_mult_answer_middle);
  fprintf (fp, "format_div_answer_last = %d\n", math_opts->format_div_answer_last);
  fprintf (fp, "format_div_answer_first = %d\n", math_opts->format_div_answer_first);
  fprintf (fp, "format_div_answer_middle = %d\n", math_opts->format_div_answer_middle);

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
  fprintf (fp, "allow_negatives = %d\n", math_opts->allow_negatives);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# 'max_answer' is the largest absolute value allowed in    #\n"
                 "# any value in a question (not only the answer). Default   #\n"
                 "# is 144. It can be set as high as 999.                    #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "max_answer = %d\n", math_opts->max_answer);

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# 'max_questions' is limit of the length of the question   #\n"
                 "# list. Default is 5000 - only severe taskmasters will     #\n"
                 "# need to raise it.                                        #\n"
                 "############################################################\n\n");
  }  
  fprintf (fp, "max_questions = %d\n", math_opts->max_questions);  

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "# If 'randomize' selected, the list will be shuffled       #\n"
                 "# at the start of the game.  Default is 1 (yes).           #\n"
                 "############################################################\n\n");
  }
  fprintf (fp, "randomize = %d\n", math_opts->randomize);

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
  fprintf(fp, "addition_allowed = %d\n", math_opts->addition_allowed);
  fprintf(fp, "subtraction_allowed = %d\n", math_opts->subtraction_allowed);
  fprintf(fp, "multiplication_allowed = %d\n", math_opts->multiplication_allowed);
  fprintf(fp, "division_allowed = %d\n", math_opts->division_allowed);


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
  fprintf(fp, "min_augend = %d\n", math_opts->min_augend);
  fprintf(fp, "max_augend = %d\n", math_opts->max_augend);
  fprintf(fp, "min_addend = %d\n", math_opts->min_addend);
  fprintf(fp, "max_addend = %d\n", math_opts->max_addend);

  fprintf(fp, "\n# Subtraction operands: minuend - subtrahend = difference\n");
  fprintf(fp, "min_minuend = %d\n", math_opts->min_minuend);
  fprintf(fp, "max_minuend = %d\n", math_opts->max_minuend);
  fprintf(fp, "min_subtrahend = %d\n", math_opts->min_subtrahend);
  fprintf(fp, "max_subtrahend = %d\n", math_opts->max_subtrahend);

  fprintf(fp, "\n# Multiplication operands: multiplier * multiplicand = product\n");
  fprintf(fp, "min_multiplier = %d\n", math_opts->min_multiplier);
  fprintf(fp, "max_multiplier = %d\n", math_opts->max_multiplier);
  fprintf(fp, "min_multiplicand = %d\n", math_opts->min_multiplicand);
  fprintf(fp, "max_multiplicand = %d\n", math_opts->max_multiplicand);

  fprintf(fp, "\n# Division operands: dividend/divisor = quotient\n");
  fprintf(fp, "min_divisor = %d\n",math_opts->min_divisor);
  fprintf(fp, "max_divisor = %d\n", math_opts->max_divisor);
  fprintf(fp, "min_quotient = %d\n", math_opts->min_quotient);
  fprintf(fp, "max_quotient = %d\n", math_opts->max_quotient);

  fprintf(fp, "\n# Typing practice:\n");
  fprintf(fp, "min_typing_num = %d\n",math_opts->min_typing_num);
  fprintf(fp, "max_typing_num = %d\n",math_opts->max_typing_num);

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
  if (math_opts->min_augend < 0)
    math_opts->min_augend = 0;
  if (math_opts->max_augend < 0)
    math_opts->max_augend = 0;
  if (math_opts->min_addend < 0)
    math_opts->min_addend = 0;
  if (math_opts->max_addend < 0)
    math_opts->max_addend = 0;

  if (math_opts->min_minuend < 0)
    math_opts->min_minuend = 0;
  if (math_opts->max_minuend < 0)
    math_opts->max_minuend = 0;
  if (math_opts->min_subtrahend < 0)
    math_opts->min_subtrahend = 0;
  if (math_opts->max_subtrahend < 0)
    math_opts->max_subtrahend = 0;

  if (math_opts->min_multiplier < 0)
    math_opts->min_multiplier = 0;
  if (math_opts->max_multiplier < 0)
    math_opts->max_multiplier = 0;
  if (math_opts->min_multiplicand < 0)
    math_opts->min_multiplicand = 0;
  if (math_opts->max_multiplicand < 0)
    math_opts->max_multiplicand = 0;

  if (math_opts->min_divisor < 0)
    math_opts->min_divisor = 0;
  if (math_opts->max_divisor < 0)
    math_opts->max_divisor = 0;
  if (math_opts->min_quotient < 0)
    math_opts->min_quotient = 0;
  if (math_opts->max_quotient < 0)
    math_opts->max_quotient = 0;

  if (math_opts->min_typing_num < 0)
    math_opts->min_typing_num = 0;
  if (math_opts->max_typing_num < 0)
    math_opts->max_typing_num = 0;
}

/* using parameters from the mission struct, create linked list of "flashcards" */
/* FIXME should figure out how to proceed correctly if we run out of memory */
/* FIXME very redundant code - figure out way to iterate through different */
/* math operations and question formats                                    */
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

  if (math_opts->addition_allowed)
  {
    #ifdef MC_DEBUG
    printf("\nAddition problems");
    #endif
    for (i = math_opts->min_augend; i <= math_opts->max_augend; i++)
    {
      for (j = math_opts->min_addend; j <= math_opts->max_addend; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (validate_question(i, j, i + j))
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->question_copies; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 + num2 = ? */
            if (math_opts->format_add_answer_last)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_ADD, i + j, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 + ? = num3 */
            if (math_opts->format_add_answer_middle)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
                 && randomly_keep())
              {
                tmp_ptr = create_node(i, j, MC_OPER_ADD, i + j, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? + num2 = num3 */
            if (math_opts->format_add_answer_first)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
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

  if (math_opts->subtraction_allowed)
  {
    #ifdef MC_DEBUG
    printf("\nSubtraction problems");
    #endif
    for (i = math_opts->min_minuend; i <= math_opts->max_minuend; i++)
    {
      for (j = math_opts->min_subtrahend; j <= math_opts->max_subtrahend; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (validate_question(i, j, i - j))
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->question_copies; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 - num2 = ? */
            if (math_opts->format_sub_answer_last)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
                 && randomly_keep())

              {
                tmp_ptr = create_node(i, j, MC_OPER_SUB, i - j, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 - ? = num3 */
            if (math_opts->format_sub_answer_middle)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
                 && randomly_keep())

              {
                tmp_ptr = create_node(i, j, MC_OPER_SUB, i - j, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? - num2 = num3 */
            if (math_opts->format_sub_answer_first)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
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

  if (math_opts->multiplication_allowed)
  {
    #ifdef MC_DEBUG
    printf("\nMultiplication problems");
    #endif
    for (i = math_opts->min_multiplier; i <= math_opts->max_multiplier; i++)
    {
      for (j = math_opts->min_multiplicand; j <= math_opts->max_multiplicand; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (validate_question(i, j, i * j))
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->question_copies; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 x num2 = ? */
            if (math_opts->format_mult_answer_last)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
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
            if ((math_opts->format_mult_answer_middle)
             && (i != 0)) 
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
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
            if ((math_opts->format_mult_answer_first)
             && (j != 0))
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
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

  if (math_opts->division_allowed)
  {
    #ifdef MC_DEBUG
    printf("\nDivision problems");
    #endif
    for (i = math_opts->min_quotient; i <= math_opts->max_quotient; i++)
    {
      for (j = math_opts->min_divisor; j <= math_opts->max_divisor; j++)
      {
        /* check if max_answer exceeded or if question */
        /* contains undesired negative values:         */
        if (j                                     /* must avoid division by zero: */      
            &&
            validate_question(i * j, j, i))       /* division problems are generated as multiplication */
        {  
          /* put in the desired number of copies: */
          for (k = 0; k < math_opts->question_copies; k++)
          {
            /* put in questions in each selected format: */

            /* questions like num1 / num2 = ? */
            if (math_opts->format_div_answer_last)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
                 && randomly_keep())
              {
                tmp_ptr = create_node(i * j, j, MC_OPER_DIV, i, MC_FORMAT_ANS_LAST);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like num1 / ? = num3 */
            if ((math_opts->format_div_answer_middle)
               && (i))      /* This avoids creating indeterminate questions: 0/? = 0 */
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
                 && randomly_keep())
              {
                tmp_ptr = create_node(i * j, j, MC_OPER_DIV, i, MC_FORMAT_ANS_MIDDLE);
                top_of_list = insert_node(top_of_list, end_of_list, tmp_ptr);
                end_of_list = tmp_ptr;
                length++; 
              } 
            }

            /* questions like ? / num2  = num3 */
            if (math_opts->format_div_answer_first)
            {
              /* make sure max_questions not exceeded, */
              /* also check if question being randomly kept or discarded: */
              if ((length < math_opts->max_questions)
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

  if (math_opts->typing_practice_allowed)
  {
    #ifdef MC_DEBUG
    printf("\nTyping problems");
    #endif
    for (i = math_opts->min_typing_num; i <= math_opts->max_typing_num; i++)
    {
      /* check if max_answer exceeded or if question */
      /* contains undesired negative values:         */
      if (validate_question(i, i, i))
      {  
        /* put in the desired number of copies: */
        for (k = 0; k < math_opts->question_copies; k++)
        {
          /* make sure max_questions not exceeded, */
          /* also check if question being randomly kept or discarded: */
          if ((length < math_opts->max_questions)
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
  if (math_opts->randomize)
  {
    top_of_list = randomize_list(top_of_list); 
  }

  #ifdef MC_DEBUG
  length = list_length(top_of_list); 
  printf("\nlength after randomization:\t%d", length); 
  printf("\nLeaving generate_list()\n");
  #endif

  return top_of_list;
}


/* this is used by generate_list to see if a possible question */
/* meets criteria to be added to the list or not:              */
int validate_question(int n1, int n2, int n3)
{
  /* make sure none of values exceeds max_answer using absolute */
  /* value comparison:                                          */
  if (abs_value(n1) > abs_value(math_opts->max_answer)
   || abs_value(n2) > abs_value(math_opts->max_answer)
   || abs_value(n3) > abs_value(math_opts->max_answer))
  {
    return 0;
  }
  /* make sure none of values are negative if negatives not allowed: */
  if (!math_opts->allow_negatives)
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

  ptr->card.num1 = n1;
  ptr->card.num2 = n2;  
  ptr->card.num3 = ans;
  ptr->card.operation = op;
  ptr->card.format = f;
  ptr->next = NULL;
  ptr->previous = NULL;


  /* creating formula_string  and answer_string is a little more work: */
  {
    char oper_char;
    /* find out correct operation character */
    switch (op)
    {
      case MC_OPER_ADD:
      {
        oper_char = '+';
        break;
      }
      case MC_OPER_SUB:
      {
        oper_char = '-';
        break;
      }
      case MC_OPER_MULT:
      {
        oper_char = '*';
        break;
      }
      case MC_OPER_DIV:
      {
        oper_char = '/';
        break; 
      }
      case MC_OPER_TYPING_PRACTICE:
      {
        snprintf(ptr->card.formula_string, MC_FORMULA_LEN, "%d",ptr->card.num1);
        snprintf(ptr->card.answer_string, MC_ANSWER_LEN, "%d",ptr->card.num1);
        return ptr;  /* Don't need to look at formats for this case. */
      }
      default:
      {
        fprintf(stderr, "\nIn create_node(): invalid math operation\n");
        free(ptr);
        ptr = 0;

        return 0;
      }
    }

    switch (f) /* f is format argument */
    {
      case MC_FORMAT_ANS_LAST:  /* e.g. num1 + num2 = ? */
      {
        snprintf(ptr->card.formula_string, MC_FORMULA_LEN,"%d %c %d = ?",
                 ptr->card.num1,
                 oper_char,
                 ptr->card.num2);
        snprintf(ptr->card.answer_string, MC_ANSWER_LEN, "%d",ptr->card.num3);
       break;
      }
      case MC_FORMAT_ANS_MIDDLE:  /* e.g. num1 + ? = num3 */
      {
        snprintf(ptr->card.formula_string, MC_FORMULA_LEN,"%d %c ? = %d",
                 ptr->card.num1,
	         oper_char,
	         ptr->card.num3);
        snprintf(ptr->card.answer_string, MC_ANSWER_LEN, "%d",ptr->card.num2);
        break;
      }
      case MC_FORMAT_ANS_FIRST:  /* e.g. ? + num2 = num3 */
      {
        snprintf(ptr->card.formula_string, MC_FORMULA_LEN,"? %c %d = %d",
                 oper_char,
                 ptr->card.num2,
                 ptr->card.num3);
        snprintf(ptr->card.answer_string, MC_ANSWER_LEN, "%d",ptr->card.num1);
        break;
      }
      default:  /* should not get to here if MathCards behaves correctly */
      {
        fprintf(stderr, "\ncreate_node() - invalid question format\n");
        free(ptr);
        ptr = 0;

 
        return 0;
      }
    }
  }

  /* ptr should now point to a properly constructed node: */
  return ptr;
}


#ifdef MC_DEBUG
/* a "copy constructor", so to speak */
/* FIXME should properly return newly allocated list if more than one node DSB */
MC_MathQuestion* create_node_copy(MC_MathQuestion* other)
{
  return create_node(other->card.num1,
                     other->card.num2,
                     other->card.operation,
                     other->card.num3,
                     other->card.format);
}
#endif

MC_MathQuestion* create_node_from_card(MC_FlashCard* flashcard)
{
  return create_node(flashcard->num1,
                     flashcard->num2,
                     flashcard->operation,
                     flashcard->num3,
                     flashcard->format);
}

#ifdef MC_DEBUG
/* FIXME take care of strings */

MC_FlashCard* create_card_from_node(MC_MathQuestion* node)
{
  MC_FlashCard* fc;
  if (!node)
    return 0;
  fc = malloc(sizeof(MC_FlashCard));
  fc->num1 = node->card.num1;
  fc->num2 = node->card.num2;
  fc->num3 = node->card.num3;
  fc->operation = node->card.operation;
  fc->format = node->card.format;
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

  copy->card.num1 = original->card.num1;
  copy->card.num2 = original->card.num2;
  copy->card.num3 = original->card.num3;
  copy->card.operation = original->card.operation;
  copy->card.format = original->card.format;
  strncpy(copy->card.formula_string,
          original->card.formula_string,
          MC_FORMULA_LEN);
  strncpy(copy->card.answer_string,
          original->card.answer_string,
          MC_ANSWER_LEN);
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
    free (list);
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
  printf("\n%d,  %d \tOper %d \tAnswer %d \t Format %d\n",
           card.num1,
           card.num2,
           card.operation,
           card.num3,
           card.format);
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






/* This is a new implementation written in an attempt to avoid */
/* the O(n^2) performance problems seen with the old randomization */
/* function. The list is created as a vector, but is for now still */
/* made a linked list to minimize changes needed elsewhere.        */
/* NOTE - the function frees the old list and returns a pointer to */
/* a newly allocated shuffled list - maybe this is confusing.  As  */
/* long as it is used as in "ptr = new_randomize_list(ptr);", it */
/* should not cause problems. */
MC_MathQuestion* randomize_list(MC_MathQuestion* old_list)
{
  MC_MathQuestion* old_tmp = old_list;
  MC_MathQuestion** tmp_vect = NULL;
  MC_MathQuestion* new_list_head = NULL;

  int i = 0;
  int old_length = list_length(old_list);

  /* set random seed: */
  srand(time(0));  

  /* Allocate vector and copy in old list - this is needed because old_list */
  /* may have "holes" in it from deletions: */

  /* This just allocates the list of pointers, not space for the nodes themselves: */
  tmp_vect = (MC_MathQuestion**)malloc(sizeof(MC_MathQuestion*) * old_length);

  for (i = 0; i < old_length; i++)
  {
    tmp_vect[i] = (MC_MathQuestion*)malloc(sizeof(MC_MathQuestion));

    if (!copy_node(old_tmp, tmp_vect[i]))
    {
      int j = 0;
      fprintf(stderr, "Error during copying - cannot randomize list!\n");
      fprintf(stderr, "Problem occurred for i = %d\n", i);
      for (j = 0; j <= i; j++)
        free(tmp_vect[j]);
      free(tmp_vect);
      return NULL;
    }

    tmp_vect[i]->randomizer = rand();
    old_tmp = old_tmp->next;
  }

  /* free arg list now that copy made: */
  delete_list(old_list);

  /* Now just sort on 'tmp_vect[i]->randomizer' to shuffle list: */
  qsort(tmp_vect, old_length,
        sizeof(MC_MathQuestion*),
        comp_randomizer);

  /* Set pointers, as rest of program uses this as a linked list */
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
  tmp_vect[0]->previous = NULL;
  tmp_vect[old_length-1]->next = NULL;

  /* Now just return pointer to first element! */
  new_list_head = tmp_vect[0];
  free(tmp_vect);
  return new_list_head;
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
  if (first->card.num1 == other->card.num1
   && first->card.num2 == other->card.num2
   && first->card.operation == other->card.operation
   && first->card.format == other->card.format)
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
   && !math_opts->allow_negatives)
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
