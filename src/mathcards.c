/* mathcards.c

   implementation of backend for a flashcard-type math game.

   Copyright 2005, 2008, 2009, 2010, 2011.
Authors:  David Bruce, Tim Holy, Brendan Luchen, "Povik".
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

mathcards.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <time.h>


#include "transtruct.h"
#include "mathcards.h"
#include "globals.h"

/* extern'd constants */

const char* const MC_OPTION_TEXT[NOPTS+1] = {
    "PLAY_THROUGH_LIST",
    "QUESTION_COPIES",
    "REPEAT_WRONGS",
    "COPIES_REPEATED_WRONGS",
    "ALLOW_NEGATIVES",
    "MAX_ANSWER",
    "MAX_QUESTIONS",
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
    "ARITHMETIC_ALLOWED",
    "COMPARISON_ALLOWED",

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

    "MIN_COMPARATOR" ,
    "MAX_COMPARATOR" ,
    "MIN_COMPARISAND",
    "MAX_COMPARISAND",

    "RANDOMIZE",

    "COMPREHENSIVE",
    "AVG_LIST_LENGTH",
    "VARY_LIST_LENGTH",

    "END_OF_OPTS"
};



const int MC_DEFAULTS[] = {
    1,    //PLAY_THROUGH_LIST
    1,    //QUESTION_COPIES
    1,    //REPEAT_WRONGS
    1,    //COPIES_REPEATED_WRONGS
    0,    //ALLOW_NEGATIVES
    999,  //MAX_ANSWER
    5000, //MAX_QUESTIONS
    2,    //MAX_FORMULA_NUMS
    2,    //MIN_FORMULA_NUMS
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
    1,    //ARITHMETIC_ALLOWED
    0,    //COMPARISON_ALLOWED
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
    0,    //MIN_COMPARATOR
    12,   //MAX_COMPARATOR
    0,    //MIN_COMPARISAND
    12,   //MAX_COMPARISAND

    1,    //RANDOMIZE

    0,    //COMPREHENSIVE
    100,  //AVG_LIST_LENGTH
    0     //VARY_LIST_LENGTH  FIXME what is the purpose of this?
};



/* "Globals" for mathcards.c: */
#define PI_VAL 3.1415927
#define NPRIMES 9
const int smallprimes[NPRIMES] = {2, 3, 5 ,7, 11, 13, 17, 19, 23};
//const char operchars[4] = "+-*/";
const char operchars[4] = "+-x/";

static int id = 0;

const MC_FlashCard DEFAULT_CARD = {{'\0'}, {'\0'}, 0, 0, 0}; //empty card to signal error

/* "private" function prototypes:                        */
/*                                                       */
/* these are for internal use by MathCards only - like   */
/* the private functions of a C++ class. Declared static */
/* to give file scope rather than extern scope.          */

static MC_MathQuestion* generate_list(MC_MathGame* game);
static void clear_negatives(MC_MathGame* game);
//static int validate_question(int n1, int n2, int n3);
//static MC_MathQuestion* create_node(int n1, int n2, int op, int ans, int f);
//static MC_MathQuestion* create_node_from_card(const MC_FlashCard* flashcard);
static MC_MathQuestion* create_node_copy(MC_MathQuestion* other);
static MC_MathQuestion* insert_node(MC_MathQuestion* first, MC_MathQuestion* current, MC_MathQuestion* new_node);
static MC_MathQuestion* append_node(MC_MathQuestion* list, MC_MathQuestion* new_node);
static MC_MathQuestion* remove_node(MC_MathQuestion* first, MC_MathQuestion* n);
static MC_MathQuestion* delete_list(MC_MathQuestion* list);
//static int copy_node(MC_MathQuestion* original, MC_MathQuestion* copy);
static int list_length(MC_MathQuestion* list);
static int randomize_list(MC_MathQuestion** list);

int comp_randomizer(const void* a, const void* b);
static MC_MathQuestion* pick_random(int length, MC_MathQuestion* list);
static int compare_node(MC_MathQuestion* first, MC_MathQuestion* other);
static int already_in_list(MC_MathQuestion* list, MC_MathQuestion* ptr);
//static int int_to_bool(int i);
//static int sane_value(int i);
//static int abs_value(int i);
static int floatCompare(const void* v1,const void* v2);

static void print_list(FILE* fp,MC_MathQuestion* list);
void print_vect_list(FILE* fp, MC_MathQuestion** vect, int length);

static void print_counters(MC_MathGame *game);
//static MC_FlashCard    create_card_from_node(MC_MathQuestion* node);


/* Functions for new mathcards architecture */
static void free_node(MC_MathQuestion* mq); //wrapper for free() that also frees card
static MC_FlashCard generate_random_flashcard(MC_MathGame* game);
static MC_FlashCard generate_random_ooo_card_of_length(MC_MathGame* game, int length, int reformat);
static MC_MathQuestion* allocate_node(void); //allocate space for a node
static int compare_card(const MC_FlashCard* a, const MC_FlashCard* b); //test for identical cards
static int find_divisor(MC_MathGame* game, int a); //return a random positive divisor of a
static int calc_num_valid_questions(MC_MathGame* game);
static MC_MathQuestion* add_all_valid(MC_MathGame* game, MC_ProblemType pt, MC_MathQuestion* list, MC_MathQuestion** end_of_list);
static MC_MathQuestion* find_node(MC_MathQuestion* list, int num);
//Determine how many points to give player based on question
//difficulty and how fast it was answered.
//TODO we may want to play with this a bit
static int calc_score(int difficulty, float t);

//Create formula_string in i18n-friendly fashion:
static int create_formula_str(char* form_str, int n1, int n2, int op, int format);





/*  MC_Initialize() sets up the struct containing all of  */
/*  settings regarding math questions.  It should be      */
/*  called before any other function.  Many of the other  */
/*  functions will not work properly if MC_Initialize()   */
/*  has not been called. It only needs to be called once, */
/*  i.e when the program is starting, not at the beginning*/
/*  of each math game for the player. Returns 1 if        */
/*  successful, 0 otherwise.                              */
int MC_Initialize(MC_MathGame* game)
{
    int i;

    DEBUGMSG(debug_mathcards,"\nEntering MC_Initialize()");

    if(!game)
        return 0;
    if(game->math_opts != NULL)
        return 1; //already initialized

    game->math_opts = malloc(sizeof(MC_Options));

    /* bail out if no struct */
    if (!game->math_opts)
    {
        DEBUGMSG(debug_mathcards,"\nError: malloc couldn't allocate math_opts for some reason\n");
        DEBUGMSG(debug_mathcards,"\nLeaving MC_Initialize()\n");

        fprintf(stderr, "\nUnable to initialize math_options");
        return 0;
    }

    /* set defaults */
    for (i = 0; i < NOPTS; ++i)
    {
        game->math_opts->iopts[i] = MC_DEFAULTS[i];
    }

    /* if no negatives to be used, reset any negatives to 0 */
    if (!game->math_opts->iopts[ALLOW_NEGATIVES])
    {
        clear_negatives(game);
    }


    DEBUGCODE(debug_mathcards)
    {
        MC_PrintMathOptions(game, stdout, 0);
        printf("\nLeaving MC_Initialize()\n");
    }

    return 1;
}



/*  MC_StartGame() generates the list of math questions   */
/*  based on existing settings. It should be called at    */
/*  the beginning of each math game for the player.       */
/*  Returns 1 if resultant list contains 1 or more        */
/*  questions, 0 if list empty or not generated           */
/*  successfully.                                         */
int MC_StartGame(MC_MathGame* game)
{

    DEBUGMSG(debug_mathcards,"\nEntering MC_StartGame()");

    /* if math_opts not set up yet, initialize it: */
    if (!game->math_opts)
    {
        DEBUGMSG(debug_mathcards, "\nNot initialized - calling MC_Initialize()");
        MC_Initialize(game);
    }

    if (!game->math_opts)
    {
        DEBUGMSG(debug_mathcards, "\nCould not initialize - bailing out");
        DEBUGMSG(debug_mathcards, "\nLeaving MC_StartGame()\n");
        return 0;
    }

    /* we know math_opts exists if we make it to here */
    srand(time(NULL));

    /* clear out old lists if starting another game: (if not done already) */
    delete_list(game->question_list);
    game->question_list = NULL;
    delete_list(game->wrong_quests);
    game->wrong_quests = NULL;
    delete_list(game->active_quests);
    game->active_quests = NULL;

    /* clear the time list */
    if (game->time_per_question_list != NULL)
    {
        free(game->time_per_question_list);
        game->time_per_question_list = NULL;
        game->length_time_per_question_list = 0;
        game->length_alloc_time_per_question_list = 0;
    }

    game->question_list = generate_list(game);
    game->next_wrong_quest = NULL;
    /* initialize counters for new game: */
    game->quest_list_length = list_length(game->question_list);


    /* Note: the distinction between quest_list_length and  */
    /* unanswered is that the latter includes questions     */
    /* that are currently "in play" by the user interface - */
    /* it is only decremented when an answer to the question*/
    /* is received.                                         */
    game->unanswered = game->starting_length = game->quest_list_length;
    game->answered_correctly = 0;
    game->answered_wrong = 0;
    game->questions_pending = 0;

    if (debug_status & debug_mathcards) {
        print_counters(game);
    }

    /* make sure list now exists and has non-zero length: */
    if (game->question_list && game->quest_list_length)
    {
        DEBUGMSG(debug_mathcards, "\nGame set up successfully");
        DEBUGMSG(debug_mathcards, "\nLeaving MC_StartGame()\n");

        return 1;
    }
    else
    {
        DEBUGMSG(debug_mathcards, "\nGame NOT set up successfully - no valid list");
        DEBUGMSG(debug_mathcards, "\nLeaving MC_StartGame()\n");

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
int MC_StartGameUsingWrongs(MC_MathGame* game)
{
    DEBUGMSG(debug_mathcards, "\nEntering MC_StartGameUsingWrongs()");

    /* Note: if not initialized, control will pass to       */
    /* MC_StartGame() via else clause so don't need to test */
    /* for initialization here                              */
    if (game->wrong_quests &&
            list_length(game->wrong_quests))
    {
        DEBUGMSG(debug_mathcards, "\nNon-zero length wrong_quests list found, will");
        DEBUGMSG(debug_mathcards, "\nuse for new game list:");

        /* initialize lists for new game: */
        delete_list(game->question_list);
        if(!randomize_list(&(game->wrong_quests)))
        {
            fprintf(stderr, "Error during randomization of wrong_quests!\n");
            /* Punt on trying wrong question list, just run normal game */
            return MC_StartGame(game);
        }
        game->question_list = game->wrong_quests;
        game->wrong_quests = 0;
        game->next_wrong_quest = 0;
        delete_list(game->active_quests);
        game->active_quests = 0;
        /* initialize counters for new game: */
        game->quest_list_length = list_length(game->question_list);
        game->unanswered = game->starting_length = game->quest_list_length;
        game->answered_correctly = 0;
        game->answered_wrong = 0;
        game->questions_pending = 0;

        if (debug_status & debug_mathcards) {
            print_counters(game);
            print_list(stdout, game->question_list);
            printf("\nLeaving MC_StartGameUsingWrongs()\n");
        }

        return 1;
    }
    else /* if no wrong_quests list, go to MC_StartGame()   */
        /* to set up list based on math_opts               */
    {
        DEBUGMSG(debug_mathcards, "\nNo wrong questions to review - generate list from math_opts\n");
        DEBUGMSG(debug_mathcards, "\nLeaving MC_StartGameUsingWrongs()\n");

        return MC_StartGame(game);
    }
}


/*  MC_NextQuestion() takes a pointer to an allocated      */
/*  MC_FlashCard struct and fills in the fields for     */
/*  use by the user interface program. It basically is     */
/*  like taking the next flashcard from the pile. The      */
/*  node containing the question is removed from the list. */
/*  Returns 1 if question found, 0 if list empty/invalid   */
/*  or if argument pointer is invalid.                     */
int MC_NextQuestion(MC_MathGame* game, MC_FlashCard* fc)
{
    DEBUGMSG(debug_mathcards, "\nEntering MC_NextQuestion()\n");

    /* (so we can move the node into active_quests:) */
    MC_MathQuestion* ptr;

    if (!fc )
    {
        fprintf(stderr, "\nNull MC_FlashCard* argument!\n");
        DEBUGMSG(debug_mathcards, "\nLeaving MC_NextQuestion()\n");
        return 0;
    }

    if (!game->question_list ||
            /*      !game->next_question || */
            !list_length(game->question_list) )
    {
        DEBUGMSG(debug_mathcards, "\nquestion_list invalid or empty");
        DEBUGMSG(debug_mathcards, "\nLeaving MC_NextQuestion()\n");

        return 0;
    }

    /* 'draw' - copy over the first question */
    MC_CopyCard(&(game->question_list->card), fc);

    /* take first question node out of list and move it into active_quests list: */
    ptr = game->question_list;
    game->question_list = remove_node(game->question_list, ptr);
    //  free_node(ptr);
    game->quest_list_length--;
    game->questions_pending++;
    game->active_quests = append_node(game->active_quests, ptr);

    if (debug_status & debug_mathcards) {
        printf("\nnext question is:");
        print_card(*fc);
        print_counters(game);
        printf("\n\nLeaving MC_NextQuestion()\n");
    }

    return 1;
}



/*  MC_AnsweredCorrectly() is how the user interface      */
/*  tells MathCards that the question has been answered   */
/*  correctly. Returns the number of points earned.       */
int MC_AnsweredCorrectly(MC_MathGame* game, int id, float t)
{
    DEBUGMSG(debug_mathcards, "\nEntering MC_AnsweredCorrectly()");

    MC_MathQuestion* quest = NULL;
    int points = 0;

    if(!game->active_quests) // No questions currently "in play" - something is wrong:
    {
        fprintf(stderr, "MC_AnsweredCorrectly() - active_quests empty\n");
        return 0;
    }

    DEBUGMSG(debug_mathcards, "\nQuestion id was: %d\n", id);

    //First take the question out of the active_quests list
    quest = game->active_quests;  
    // Loop until quest is NULL or we find card with same id:
    while(quest && (id != quest->card.question_id))
        quest = quest->next;
    if(!quest) // Means we didn't find matching card - something is wrong:
    {
        fprintf(stderr, "MC_AnsweredCorrectly() - matching question not found!\n");
        return 0;
    }

    /* Calculate how many points the player should receive, based on */
    /* difficulty and time required to answer it:                    */
    points = calc_score(quest->card.difficulty, t);

    DEBUGCODE(debug_mathcards)
    {
        printf("\nQuestion was:");
        print_card(quest->card);
        printf("Player recieves %d points\n", points);
    }


    //We found a matching question, now we take it out of the 
    //"active_quests" list and either put it back into the 
    //main question list in a random location, or delete it:
    game->active_quests = remove_node(game->active_quests, quest);
    game->questions_pending--;  //the length of the 'active_quests' list
    game->answered_correctly++;

    if (!game->math_opts->iopts[PLAY_THROUGH_LIST])
        /* reinsert question into question list at random location */
    {
        DEBUGMSG(debug_mathcards, "\nReinserting question into list");

        MC_MathQuestion* rand_spot;
        /* put it into list */
        rand_spot = pick_random(game->quest_list_length, game->question_list);
        game->question_list = insert_node(game->question_list, rand_spot, quest);
        game->quest_list_length++;
        /* unanswered does not change - was not decremented when */
        /* question allocated!                                   */
    }
    else
    {
        DEBUGMSG(debug_mathcards, "\nNot reinserting question into list");
        free_node(quest);
        /* not recycling questions so fewer questions remain:      */
        game->unanswered--;
    }

    DEBUGCODE(debug_mathcards)
    {
        print_counters(game);
        printf("\nLeaving MC_AnsweredCorrectly()\n");
    }

    /* Record the time it took to answer: */ 
    MC_AddTimeToList(game, t);

    return points;
}





/*  MC_NotAnsweredCorrectly() is how the user interface    */
/*  tells MathCards that the player failed to answer the  */
/*  question correctly. Returns 1 if no errors.           */
/*  Note: this gets triggered only if a player's igloo/city */
/*  gets hit by a question, not if they "miss".             */
int MC_NotAnsweredCorrectly(MC_MathGame* game, int id)
{
    DEBUGMSG(debug_mathcards, "\nEntering MC_NotAnsweredCorrectly()");

    MC_MathQuestion* quest = NULL;

    if(!game->active_quests) // No questions currently "in play" - something is wrong:
    {
        fprintf(stderr, "MC_NotAnsweredCorrectly() - active_quests empty\n");
        return 0;
    }

    DEBUGMSG(debug_mathcards, "\nQuestion id was: %d\n", id);

    //First take the question out of the active_quests list
    quest = game->active_quests;  
    // Loop until quest is NULL or we find card with same id:
    while(quest && (id != quest->card.question_id))
        quest = quest->next;
    if(!quest) // Means we didn't find matching card - something is wrong:
    {
        fprintf(stderr, "MC_NotAnsweredCorrectly() - matching question not found!\n");
        return 0;
    }

    DEBUGMSG(debug_mathcards, "\nMatching question is:");
    print_card(quest->card);


    /* if desired, put question back in list so student sees it again */
    if (game->math_opts->iopts[REPEAT_WRONGS])
    {
        int i;
        MC_MathQuestion* quest_copy;
        MC_MathQuestion* rand_loc;

        DEBUGMSG(debug_mathcards, "\nAdding %d copies to question_list:", game->math_opts->iopts[COPIES_REPEATED_WRONGS]);

        DEBUGCODE(debug_mathcards)
        {
            print_counters(game);
            printf("\nLeaving MC_AnsweredCorrectly()\n");
        }

        /* can put in more than one copy (to drive the point home!) */
        for (i = 0; i < game->math_opts->iopts[COPIES_REPEATED_WRONGS]; i++)
        {
            quest_copy = create_node_copy(quest);
            rand_loc = pick_random(game->quest_list_length, game->question_list);
            game->question_list = insert_node(game->question_list, rand_loc, quest_copy);
            game->quest_list_length++;
        }
        /* unanswered stays the same if a single copy recycled or */
        /* increases by 1 for each "extra" copy reinserted:       */
        game->unanswered += (game->math_opts->iopts[COPIES_REPEATED_WRONGS] - 1);
    }
    else
    {
        DEBUGMSG(debug_mathcards, "\nNot repeating wrong answers\n");
        /* not repeating questions so list gets shorter:      */
        game->unanswered--;
    }

    //Take the question out of the active_quests list and add it to
    //the wrong_quests list, unless an identical question is already
    //in the wrong_quests list:
    game->active_quests = remove_node(game->active_quests, quest);
    game->questions_pending--;  //the length of the 'active_quests' list
    game->answered_wrong++;

    /* add question to wrong_quests list: */
    if (!already_in_list(game->wrong_quests, quest)) /* avoid duplicates */
    {
        DEBUGMSG(debug_mathcards, "\nAdding to wrong_quests list");
        game->wrong_quests = append_node(game->wrong_quests, quest);
    }
    else /* avoid memory leak */
    {
        free_node(quest);
    }

    DEBUGCODE(debug_mathcards)
    {
        print_counters(game);
        printf("\nLeaving MC_NotAnswered_Correctly()\n");
    }

    return 1;
}






/* Tells user interface if all questions have been answered correctly! */
/* Requires that at list contained at least one question to start with */
/* and that wrongly answered questions have been recycled.             */
int MC_MissionAccomplished(MC_MathGame* game)
{
    if (game->starting_length
            && game->math_opts->iopts[REPEAT_WRONGS]
            && !game->unanswered)
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
int MC_TotalQuestionsLeft(MC_MathGame* game)
{
    return game->unanswered;
}

/*  Returns number of questions left in list, NOT       */
/*  including questions currently "in play".            */
int MC_ListQuestionsLeft(MC_MathGame* game)
{
    return game->quest_list_length;
}


/*  Store the amount of time a given flashcard was      */
/*  visible on the screen. Returns 1 if the request     */
/*  succeeds, 0 otherwise.                              */
int MC_AddTimeToList(MC_MathGame* game, float t)
{
    int newsize = 0;
    float *newlist;

    //Bail if time invalid:
    if(t < 0)
        return 0;

    /* This list will be allocated in an STL-like manner: when the       */
    /* list gets full, allocate an additional amount of storage equal    */
    /* to the current size of the list, so that only O(logN) allocations */
    /* will ever be needed. We therefore have to keep track of 2 sizes:  */
    /* the allocated size, and the actual number of items currently on   */
    /* the list.                                                         */
    if (game->length_time_per_question_list >= game->length_alloc_time_per_question_list) {
        /* The list is full, allocate more space */
        newsize = 2*game->length_time_per_question_list;
        if (newsize == 0)
            newsize = 100;
        newlist = realloc(game->time_per_question_list, newsize*sizeof(float));
        if (newlist == NULL)
        {
            DEBUGMSG(debug_mathcards,"\nError: allocation for time_per_question_list failed\n");
            return 0;
        }
        game->time_per_question_list = newlist;
        game->length_alloc_time_per_question_list = newsize;
    }

    /* Append the time to the list */
    game->time_per_question_list[game->length_time_per_question_list++] = t;
    return 1;
}

/* Frees heap memory used in program:                   */
void MC_EndGame(MC_MathGame* game)
{
    delete_list(game->question_list);
    game->question_list = 0;
    delete_list(game->wrong_quests);
    game->wrong_quests = 0;

    if (game->math_opts)
    {
        free(game->math_opts);
        game->math_opts = 0;
    }

    free(game->time_per_question_list);
    game->time_per_question_list = NULL;
    game->length_alloc_time_per_question_list = 0;
    game->length_time_per_question_list = 0;
}



/* prints struct to file */
void MC_PrintMathOptions(MC_MathGame* game, FILE* fp, int verbose)
{
    int i, vcommentsprimed = 0;
    //comments when writing out verbose...perhaps they can go somewhere less conspicuous
    static char* vcomments[NOPTS];
    if (!vcommentsprimed) //we only want to initialize these once
    {
        vcommentsprimed = 1;
        for (i = 0; i < NOPTS; ++i)
            vcomments[i] = NULL;
        vcomments[PLAY_THROUGH_LIST] =
            "\n############################################################\n"
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
            "############################################################\n\n";

        vcomments[FORMAT_ADD_ANSWER_LAST] =
            "\n############################################################\n"
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
            "############################################################\n\n";

        vcomments[ALLOW_NEGATIVES] =
            "\n############################################################\n"
            "# 'allow_negatives' allows or disallows use of negative    #\n"
            "# numbers as both operands and answers.  Default is 0      #\n"
            "# (no), which disallows questions like:                    #\n"
            "#          2 - 4 = ?                                       #\n"
            "# Note: this option must be enabled in order to set the    #\n"
            "# operand ranges to include negatives (see below). If it   #\n"
            "# is changed from 1 (yes) to 0 (no), any negative          #\n"
            "# operand limits will be reset to 0.                       #\n"
            "############################################################\n\n";

        vcomments[MAX_ANSWER] =
            "\n############################################################\n"
            "# 'max_answer' is the largest absolute value allowed in    #\n"
            "# any value in a question (not only the answer). Default   #\n"
            "# is 144. It can be set as high as 999.                    #\n"
            "############################################################\n\n";

        vcomments[MAX_QUESTIONS] =
            "\n############################################################\n"
            "# 'max_questions' is limit of the length of the question   #\n"
            "# list. Default is 5000 - only severe taskmasters will     #\n"
            "# need to raise it.                                        #\n"
            "############################################################\n\n";

        vcomments[RANDOMIZE] =
            "\n############################################################\n"
            "# If 'randomize' selected, the list will be shuffled       #\n"
            "# at the start of the game.  Default is 1 (yes).           #\n"
            "############################################################\n\n";

        vcomments[ADDITION_ALLOWED] =
            "\n############################################################\n"
            "#                                                          #\n"
            "#                 Math Operations Allowed                  #\n"
            "#                                                          #\n"
            "# These options enable questions for each of the four math #\n"
            "# operations.  All are 1 (yes) by default.                 #\n"
            "############################################################\n\n";

        vcomments[MIN_AUGEND] =
            "\n############################################################\n"
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
            "############################################################\n\n";

    }


    DEBUGMSG(debug_mathcards, "\nEntering MC_PrintMathOptions()\n");

    /* bail out if no struct */
    if (!game->math_opts)
    {
        fprintf(stderr, "\nMath Options struct does not exist!\n");
        return;
    }

    for (i = 0; i < NOPTS; ++i)
    {
        if (verbose && vcomments[i] != NULL)
            fprintf(fp, "%s", vcomments[i]);
        fprintf(fp, "%s = %d\n", MC_OPTION_TEXT[i], game->math_opts->iopts[i]);
    }
    DEBUGMSG(debug_mathcards, "\nLeaving MC_PrintMathOptions()\n");
}



int MC_PrintQuestionList(MC_MathGame* game, FILE* fp)
{
    if (fp && game->question_list)
    {
        print_list(fp, game->question_list);
        return 1;
    }
    else
    {
        fprintf(stderr, "\nFile pointer and/or question list invalid\n");
        return 0;
    }
}

int MC_PrintWrongList(MC_MathGame* game, FILE* fp)
{
    if (!fp)
    {
        fprintf(stderr, "File pointer invalid\n");
        return 0;
    }

    if (game->wrong_quests)
    {
        print_list(fp, game->wrong_quests);
    }
    else
    {
        fprintf(fp, "\nNo wrong questions!\n");
    }

    return 1;
}


int MC_StartingListLength(MC_MathGame* game)
{
    return game->starting_length;
}


int MC_WrongListLength(MC_MathGame* game)
{
    return list_length(game->wrong_quests);
}

int MC_NumAnsweredCorrectly(MC_MathGame* game)
{
    return game->answered_correctly;
}


int MC_NumNotAnsweredCorrectly(MC_MathGame* game)
{
    return game->answered_wrong;
}


int MC_MakeFlashcard(char* buf, MC_FlashCard* fc)
{
    int i = 0,tab = 0, s = 0;
    char formula[MC_FORMULA_LEN];
    sscanf (buf,"%*s%d%d%d%s",
            &fc->question_id,
            &fc->difficulty,
            &fc->answer,
            fc->answer_string); /* can't formula_string in sscanf in here cause it includes spaces*/

    /*doing all this cause sscanf will break on encountering space in formula_string*/
    /* NOTE changed to index notation so we keep within NET_BUF_LEN */
    while(buf[i]!='\n' && i < NET_BUF_LEN)
    {
        if(buf[i]=='\t')
            tab++; 
        i++;
        if(tab == 5)
            break;
    }

    while((buf[i] != '\n') 
            && (s < MC_FORMULA_LEN - 1)) //Must leave room for terminating null
    {
        formula[s] = buf[i] ;
        i++;
        s++;
    }
    formula[s]='\0';
    strcpy(fc->formula_string, formula); 

    DEBUGMSG(debug_lan, "In Make_Flashcard, new card is:\n");
    DEBUGCODE(debug_lan) print_card(*fc); 

    return 1;
} 


/* Report the median time per question */
float MC_MedianTimePerQuestion(MC_MathGame* game)
{
    if (game->length_time_per_question_list == 0)
        return 0;

    qsort(game->time_per_question_list, game->length_time_per_question_list, sizeof(float), floatCompare);
    return game->time_per_question_list[game->length_time_per_question_list/2];
}




/* Implementation of "private methods" - (cannot be called from outside
   of this file) */



/* Resets negative values to zero - used when allow_negatives deselected. */
void clear_negatives(MC_MathGame* game)
{
    int i;
    for (i = MIN_AUGEND; i <= MAX_TYPING_NUM; ++i)
        if (game->math_opts->iopts[i]< 0)
            game->math_opts->iopts[i]= 0;
}

// /* this is used by generate_list to see if a possible question */
// /* meets criteria to be added to the list or not:              */
// int validate_question(int n1, int n2, int n3)
// {
//   /* make sure none of values exceeds max_answer using absolute */
//   /* value comparison:                                          */
//   if (abs_value(n1) > abs_value(math_opts->iopts[MAX_ANSWER])
//    || abs_value(n2) > abs_value(math_opts->iopts[MAX_ANSWER])
//    || abs_value(n3) > abs_value(math_opts->iopts[MAX_ANSWER]))
//   {
//     return 0;
//   }
//   /* make sure none of values are negative if negatives not allowed: */
//   if (!math_opts->iopts[ALLOW_NEGATIVES])
//   {
//     if (n1 < 0 || n2 < 0 || n3 < 0)
//     {
//       return 0;
//     }
//   }
//   return 1;
// }



// MC_MathQuestion* create_node_from_card(const MC_FlashCard* flashcard)
// {
//   MC_MathQuestion* ret = allocate_node();
//   MC_CopyCard(flashcard, &(ret->card));
//   return ret;
// }

// /* FIXME take care of strings */
// /* this one copies the contents, including pointers; both nodes must be allocated */
// int copy_node(MC_MathQuestion* original, MC_MathQuestion* copy)
// {
//   if (!original)
//   {
//     fprintf(stderr, "\nIn copy_node(): invalid 'original' pointer arg.\n");
//     return 0;
//   }
//   if (!copy)
//   {
//     fprintf(stderr, "\nIn copy_node(): invalid 'copy' pointer arg.\n");
//     return 0;
//   }
// 
//   copy_card(&(original->card), &(copy->card) );
// 
//   copy->next = original->next;
//   copy->previous = original->previous;
//   copy->randomizer = original->randomizer;
//   return 1;
// }




/* this puts the node into the list AFTER the node pointed to by current */
/* and returns a pointer to the top of the modified list  */
MC_MathQuestion* insert_node(MC_MathQuestion* first,
        MC_MathQuestion* current,
        MC_MathQuestion* new_node)
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

    MC_MathQuestion* ptr = list;
    while (ptr)
    {
        fprintf(fp, "%s\n", ptr->card.formula_string);
        ptr = ptr->next;
    }
}


void print_vect_list(FILE* fp, MC_MathQuestion** vect, int length)
{
    if (!vect)
    {
        fprintf(fp, "\nprint_vect_list(): list empty or pointer invalid\n");
        return;
    }

    int i = 0;
    DEBUGMSG(debug_mathcards, "Entering print_vect_list()\n");
    for(i = 0; i < length; i++)
        fprintf(fp, "%s\n", vect[i]->card.formula_string);

    DEBUGMSG(debug_mathcards, "Leaving print_vect_list()\n");
}



void print_card(MC_FlashCard card)
{
    printf("\nprint_card():\n");
    printf("question_id: %d\nformula_string: %s\nanswer_string: %s\n"
            "answer: %d\ndifficulty: %d\n\n",
            card.question_id,
            card.formula_string,
            card.answer_string,
            card.answer,
            card.difficulty);
}

/* This sends the values of all "global" counters and the */
/* lengths of the question lists to stdout - for debugging */
void print_counters(MC_MathGame *game)
{
    printf("\nquest_list_length = \t%d", game->quest_list_length);
    printf("\nlist_length(question_list) = \t%d", list_length(game->question_list));
    printf("\nstarting_length = \t%d", game->starting_length);
    printf("\nunanswered = \t%d", game->unanswered);
    printf("\nanswered_correctly = \t%d", game->answered_correctly);
    printf("\nanswered_wrong = \t%d", game->answered_wrong);
    printf("\nlist_length(wrong_quests) = \t%d", list_length(game->wrong_quests));
    printf("\nquestions_pending = \t%d", game->questions_pending);
    printf("\nlist_length(active_quests) = \t%d", list_length(game->active_quests));
}






// 
// /* FIXME take care of strings */
// 
// MC_FlashCard create_card_from_node(MC_MathQuestion* node)
// {
//   MC_FlashCard fc;
//   if (!node)
//     return DEFAULT_CARD;
//   fc = MC_AllocateFlashcard();
//   copy_card(&(node->card), &fc);
//   return fc;
// }



/* a "copy constructor", so to speak */
/* FIXME perhaps should return newly allocated list if more than one node DSB */
MC_MathQuestion* create_node_copy(MC_MathQuestion* other)
{
    MC_MathQuestion* ret = allocate_node();
    if (ret)
        MC_CopyCard(&(other->card), &(ret->card) );
    return ret;
}


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
    if (!old_list || !*old_list) //invalid/empty list
        return 0;

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
    if (compare_card(&(first->card), &(first->card) ) ) //cards are equal
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

// /* to prevent option settings in math_opts from getting set to */
// /* values other than 0 or 1                                    */
// int int_to_bool(int i)
// {
//   if (i)
//     return 1;
//   else
//     return 0;
// }

// /* prevent values from getting into math_opts that are outside */
// /* the range that can be handled by the program (i.e. more     */
// /* than three digits; also disallow negatives if that has been */
// /* selected.                                                   */
// int sane_value(int i)
// {
//   if (i > MC_GLOBAL_MAX)
//     i = MC_GLOBAL_MAX;
//   else if (i < -MC_GLOBAL_MAX)
//     i = -MC_GLOBAL_MAX;
// 
//   if (i < 0
//    && math_opts
//    && !math_opts->iopts[ALLOW_NEGATIVES])
//   {
//     i = 0;
//   }
// 
//   return i;
// }

// int abs_value(int i)
// {
//   if (i > 0)
//     return i;
//   else
//     return -i;
// }


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



/****************************************************
  Functions for new mathcards architecture
 ****************************************************/

void MC_CopyCard(const MC_FlashCard* src, MC_FlashCard* dest)
{
    if (!src || !dest)
        return;
    DEBUGMSG(debug_mathcards, "Copying '%s' to '%s', ", src->formula_string,dest->formula_string);
    DEBUGMSG(debug_mathcards, "copying '%s' to '%s'\n", src->answer_string, dest->answer_string);
    strncpy(dest->formula_string, src->formula_string, MC_FORMULA_LEN);
    strncpy(dest->answer_string, src->answer_string, MC_ANSWER_LEN);
    DEBUGMSG(debug_mathcards, "Card is: '%s', '%s'\n", dest->formula_string, dest->answer_string);
    dest->answer = src->answer;
    dest->difficulty = src->difficulty;
    dest->question_id = src->question_id;
}

void free_node(MC_MathQuestion* mq) //no, not that freenode.
{
    if (!mq)
        return;
    MC_FreeFlashcard(&(mq->card) );
    free(mq);
}

MC_MathQuestion* allocate_node()
{
    MC_MathQuestion* ret = NULL;
    ret = malloc(sizeof(MC_MathQuestion) );
    if (!ret)
    {
        printf("Could not allocate space for a new node!\n");
        return NULL;
    }

    ret->card = MC_AllocateFlashcard();
    ret->next = ret->previous = NULL;

    return ret;
}

/*
   The function that does the central dirty work pertaining to flashcard
   creation. Extensible to just about any kind of math problem, perhaps
   with the exception of those with multiple answers, such as "8 + 2 > ?"
   Simply specify how the problem is presented to the user, and the
   answer the game should look for, as strings.
   */
MC_FlashCard generate_random_flashcard(MC_MathGame* game)
{
    int num;
    int length;
    MC_ProblemType pt;
    MC_FlashCard ret;
    static int generate_random_flashcard_id=0;

    generate_random_flashcard_id+=1;
    DEBUGMSG(debug_mathcards, "Entering generate_random_flashcard()\n");
    DEBUGMSG(debug_mathcards, "ID is %d\n",generate_random_flashcard_id);

    //choose a problem type
    do
        pt = rand() % MC_NUM_PTYPES;
    while ( (pt == MC_PT_TYPING && !MC_GetOpt(game, TYPING_PRACTICE_ALLOWED) ) ||
            (pt == MC_PT_ARITHMETIC && !MC_GetOpt(game, ADDITION_ALLOWED) &&
             !MC_GetOpt(game, SUBTRACTION_ALLOWED) &&
             !MC_GetOpt(game, MULTIPLICATION_ALLOWED) &&
             !MC_GetOpt(game, DIVISION_ALLOWED) ) ||
            (pt == MC_PT_COMPARISON && !MC_GetOpt(game, COMPARISON_ALLOWED) )
          );

    if (pt == MC_PT_TYPING) //typing practice
    {
        DEBUGMSG(debug_mathcards, "Generating typing question\n");
        ret = MC_AllocateFlashcard();
        num = rand() % (MC_GetOpt(game, MAX_TYPING_NUM)-MC_GetOpt(game, MIN_TYPING_NUM) + 1)
            + MC_GetOpt(game, MIN_TYPING_NUM);
        snprintf(ret.formula_string, MC_FORMULA_LEN, "%d", num);
        snprintf(ret.answer_string, MC_ANSWER_LEN, "%d", num);
        ret.answer = num;
        ret.difficulty = 10;
        ret.question_id=generate_random_flashcard_id;
    }
    else //if (pt == MC_PT_ARITHMETIC)
    {
        DEBUGMSG(debug_mathcards, "Generating arithmetic question");
        length = rand() % (MC_GetOpt(game, MAX_FORMULA_NUMS) -
                MC_GetOpt(game, MIN_FORMULA_NUMS) + 1) //avoid div by 0
            +  MC_GetOpt(game, MIN_FORMULA_NUMS);
        DEBUGMSG(debug_mathcards, " of length %d", length);
        ret = generate_random_ooo_card_of_length(game, length, 1);

        if (debug_status & debug_mathcards) {
            print_card(ret);
        }
    }
    //TODO comparison problems (e.g. "6 ? 9", "<")

    DEBUGMSG(debug_mathcards, "Exiting generate_random_flashcard()\n");

    return ret;
}

/*
   Recursively generate an order of operations problem. Hopefully this won't
   raise performance issues. Difficulty is calculated based on the length of
   the formula and on the operators used. Problems have a 'base' difficulty of
   1 for binary operations, 3 for 3 numbers, 6, 10, etc. Each operator adds to
   the score: 0, 1, 2, and 3 respectively for addition, subtraction,
   multiplication and division.If reformat is 0, FORMAT_ANS_LAST will be used,
   otherwise a format is chosen at random.
   */

/* FIXME we should consider rewriting this - it currently generates some
   questions with indeterminate answers (e.g. ? * 0 = 0) that seem to be 
   impossible to prevent using the current scheme with recursive string
   operations.
   */

MC_FlashCard generate_random_ooo_card_of_length(MC_MathGame* game, int length, int reformat)
{
    int format = 0;
    int r1 = 0;
    int r2 = 0;
    int ans = 0;
    char tempstr[MC_FORMULA_LEN];
    MC_FlashCard ret;
    MC_Operation op;

    id += 1;
    DEBUGMSG(debug_mathcards, ".");
    if (length > MAX_FORMULA_NUMS)
        return DEFAULT_CARD;
    if (length <= 2)
    {
        DEBUGMSG(debug_mathcards, "\n");
        ret = MC_AllocateFlashcard();
        for (op = rand() % MC_NUM_OPERS; //pick a random operation
                MC_GetOpt(game, op + ADDITION_ALLOWED) == 0; //make sure it's allowed
                op = rand() % MC_NUM_OPERS);

        DEBUGMSG(debug_mathcards, "Operation is %c\n", operchars[op]);
        /*
           if (op == MC_OPER_ADD)
           {
           r1 = rand() % (math_opts->iopts[MAX_AUGEND] - math_opts->iopts[MIN_AUGEND] + 1) + math_opts->iopts[MIN_AUGEND];
           r2 = rand() % (math_opts->iopts[MAX_ADDEND] - math_opts->iopts[MIN_ADDEND] + 1) + math_opts->iopts[MIN_ADDEND];
           ans = r1 + r2;
           }
           else if (op == MC_OPER_SUB)
           {
           r1 = rand() % (math_opts->iopts[MAX_MINUEND] - math_opts->iopts[MIN_MINUEND] + 1) + math_opts->iopts[MIN_MINUEND];
           r2 = rand() % (math_opts->iopts[MAX_SUBTRAHEND] - math_opts->iopts[MIN_SUBTRAHEND] + 1) + math_opts->iopts[MIN_SUBTRAHEND];
           ans = r1 - r2;
           }
           else if (op == MC_OPER_MULT)
           {
           r1 = rand() % (math_opts->iopts[MAX_MULTIPLIER] - math_opts->iopts[MIN_MULTIPLIER] + 1) + math_opts->iopts[MIN_MULTIPLIER];
           r2 = rand() % (math_opts->iopts[MAX_MULTIPLICAND] - math_opts->iopts[MIN_MULTIPLICAND] + 1) + math_opts->iopts[MIN_MULTIPLICAND];
           ans = r1 * r2;
           }
           else if (op == MC_OPER_DIV)
           {
           ans = rand() % (math_opts->iopts[MAX_QUOTIENT] - math_opts->iopts[MIN_QUOTIENT] + 1) + math_opts->iopts[MIN_QUOTIENT];
           r2 = rand() % (math_opts->iopts[MAX_DIVISOR] - math_opts->iopts[MIN_DIVISOR] + 1) + math_opts->iopts[MIN_DIVISOR];
           if (r2 == 0)
           r2 = 1;
           r1 = ans * r2;
           }
           */
        if (op > MC_OPER_DIV || op < MC_OPER_ADD)
        {
            DEBUGMSG(debug_mathcards, "Invalid operator: value %d\n", op);
            return DEFAULT_CARD;
        }
        //choose two numbers in the proper range and get their result

        else do
        {
            r1 = rand() % (game->math_opts->iopts[MAX_AUGEND+4*op] - game->math_opts->iopts[MIN_AUGEND+4*op] + 1) + game->math_opts->iopts[MIN_AUGEND+4*op];    
            r2 = rand() % (game->math_opts->iopts[MAX_ADDEND+4*op] - game->math_opts->iopts[MIN_ADDEND+4*op] + 1) + game->math_opts->iopts[MIN_ADDEND+4*op]; 

            if (op == MC_OPER_ADD)
                ans = r1 + r2;
            if (op == MC_OPER_SUB)
                ans = r1 - r2;
            if (op == MC_OPER_MULT)
                ans = r1 * r2;
            if (op == MC_OPER_DIV)  
            {
                if (r2 == 0)
                    r2 = 1;
                ret.difficulty = r1;
                r1 *= r2;
                ans = ret.difficulty;
            }
        } while ( (ans < 0 && !MC_GetOpt(game, ALLOW_NEGATIVES)) || ans > MC_GetOpt(game, MAX_ANSWER) );


        DEBUGMSG(debug_mathcards, "Constructing answer_string\n");
        snprintf(ret.answer_string, MC_ANSWER_LEN, "%d", ans);
        DEBUGMSG(debug_mathcards, "Constructing formula_string\n");
        snprintf(ret.formula_string, MC_FORMULA_LEN, "%d %c %d",
                r1, operchars[op], r2);
        ret.answer = ans;
        DEBUGMSG(debug_mathcards, "int answer is %d\n", ret.answer);
        ret.difficulty = op + 1;

    }
    else //recurse
    {
        ret = generate_random_ooo_card_of_length(game, length - 1, 0);

        if (strchr(ret.formula_string, '+') || strchr(ret.formula_string, '-') )
        {
            //if the expression has addition or subtraction, we can't assume that
            //introducing multiplication or division will produce a predictable
            //result, so we'll limit ourselves to more addition/subtraction
            for (op = rand() % 2 ? MC_OPER_ADD : MC_OPER_SUB;
                    MC_GetOpt(game, op + ADDITION_ALLOWED) == 0;
                    op = rand() % 2 ? MC_OPER_ADD : MC_OPER_SUB);

        }
        else
        {
            //the existing expression can be treated as a number in itself, so we
            //can do anything to it and be confident of the result.
            for (op = rand() % MC_NUM_OPERS; //pick a random operation
                    MC_GetOpt(game, op + ADDITION_ALLOWED) == 0; //make sure it's allowed
                    op = rand() % MC_NUM_OPERS);
        }
        DEBUGMSG(debug_mathcards, "Next operation is %c,",  operchars[op]);

        //pick the next operand
        if (op == MC_OPER_ADD)
        {
            r1 = rand() % (game->math_opts->iopts[MAX_AUGEND] - game->math_opts->iopts[MIN_AUGEND] + 1) + game->math_opts->iopts[MIN_AUGEND];
            ret.answer += r1;
        }
        else if (op == MC_OPER_SUB)
        {
            r1 = rand() % (game->math_opts->iopts[MAX_SUBTRAHEND] - game->math_opts->iopts[MIN_SUBTRAHEND] + 1) + game->math_opts->iopts[MIN_SUBTRAHEND];
            ret.answer -= r1;
        }
        else if (op == MC_OPER_MULT)
        {
            r1 = rand() % (game->math_opts->iopts[MAX_MULTIPLICAND] - game->math_opts->iopts[MIN_MULTIPLICAND] + 1) + game->math_opts->iopts[MIN_AUGEND];
            ret.answer *= r1;
        }
        else if (op == MC_OPER_DIV)
        {
            r1 = find_divisor(game, ret.answer);
            ret.answer /= r1;
        }
        else
        {
            ; //invalid operator
        }
        DEBUGMSG(debug_mathcards, " operand is %d\n", r1);
        DEBUGMSG(debug_mathcards, "Answer: %d\n", ret.answer);

        //next append or prepend the new number (might need optimization)
        if (op == MC_OPER_SUB || op == MC_OPER_DIV || //noncommutative, append only
                rand() % 2)
        {
            snprintf(tempstr, MC_FORMULA_LEN, "%s %c %d", //append
                    ret.formula_string, operchars[op], r1);
            strncpy(ret.formula_string, tempstr, MC_FORMULA_LEN);
        }
        else //we're prepending
        {
            snprintf(tempstr, MC_FORMULA_LEN, "%d %c %s", //append
                    r1, operchars[op], ret.formula_string);
            strncpy(ret.formula_string, tempstr, MC_FORMULA_LEN);
        }

        //finally update the answer and score
        snprintf(ret.answer_string, MC_ANSWER_LEN, "%d", ret.answer);
        ret.difficulty += (length - 1) + op;
    }

    if (reformat)
    {
        DEBUGMSG(debug_mathcards, "Reformatting...\n");
        do {
            format = rand() % MC_NUM_FORMATS;
        } while (!MC_GetOpt(game, FORMAT_ANSWER_LAST + format) && 
                !MC_GetOpt(game, FORMAT_ADD_ANSWER_LAST + op * 3 + format) );

        strncat(ret.formula_string, " = ?", MC_FORMULA_LEN - strlen(ret.formula_string) );
        DEBUGMSG(debug_mathcards, "Formula_string: %s\n", ret.formula_string);
        reformat_arithmetic(&ret, format );     
    }
    ret.question_id = id;

    DEBUGMSG(debug_mathcards, "At end of generate_rand_ooo_card_of_length():\n");
    print_card(ret);

    return ret;
}



MC_MathQuestion* generate_list(MC_MathGame* game)
{
    int i, j;
    int length = MC_GetOpt(game, AVG_LIST_LENGTH);
    int cl; //raw length
    double r1, r2, delta, var; //randomizers for list length
    MC_MathQuestion* list = NULL;
    MC_MathQuestion* end_of_list = NULL;
    MC_MathQuestion* tnode = NULL;

    if (debug_status & debug_mathcards)
        MC_PrintMathOptions(game, stdout, 0);

    if (!(MC_GetOpt(game, ARITHMETIC_ALLOWED) ||
                MC_GetOpt(game, TYPING_PRACTICE_ALLOWED) ||
                MC_GetOpt(game, COMPARISON_ALLOWED) ) )
        return NULL;

    //FIXME - remind me, why are we doing this??
    //randomize list length by a "bell curve" centered on average
    if (length && MC_GetOpt(game, VARY_LIST_LENGTH) )
    {
        r1 = (double)rand() / RAND_MAX / 2 + 0.5; //interval (0, 1)
        r2 = (double)rand() / RAND_MAX / 2 + 0.5; //interval (0, 1)
        DEBUGMSG(debug_mathcards, "Randoms chosen: %5f, %5f\n", r1, r2);
        delta = sqrt(-2 * log(r1) ) * cos(2 * PI_VAL * r2); //standard normal dist.
        var = length / 10.0; //variance
        delta = delta * var;
        DEBUGMSG(debug_mathcards, "Delta of average is %5f\n", delta);
        length += delta;
        if (length < 0)
            length = 1; //just in case...
    }

    if (MC_GetOpt(game, COMPREHENSIVE)) //generate all
    {
        int num_valid_questions; //How many questions the COMPREHENSIVE list specifies
        int cycles_needed;       //How many times we need to generate it to get enough

        num_valid_questions = calc_num_valid_questions(game);
        if(num_valid_questions == 0)
        {
            fprintf(stderr, "generate_list() - no valid questions\n");
            return NULL;
        }

        cycles_needed = length/num_valid_questions;

        if((cycles_needed * num_valid_questions) < length)
            cycles_needed++;

        DEBUGMSG(debug_mathcards, "In generate_list() - COMPREHENSIVE method requested\n");
        DEBUGMSG(debug_mathcards, "num_valid_questions = %d\t cycles_needed = %d\n",
                num_valid_questions, cycles_needed);

        for (i = MC_PT_TYPING; i < MC_NUM_PTYPES; ++i)
        {
            if (!MC_GetOpt(game, i + TYPING_PRACTICE_ALLOWED))
                continue;
            for (j = 0; j < cycles_needed; j++)
                list = add_all_valid(game, i, list, &end_of_list);
        }


        if (MC_GetOpt(game, RANDOMIZE) )
        {
            DEBUGMSG(debug_mathcards, "Randomizing list\n");
            randomize_list(&list);
        }

        if (length)
        {
            cl = list_length(list);
            // NOTE this should no longer happen - we run the COMPREHENSIVE
            // generation until we have enough questions.
            if (length > cl) //if not enough questions, pad out with randoms
            {
                DEBUGMSG(debug_mathcards, "Padding out list from %d to %d questions\n", cl, length);
                for (i = cl; i < length; ++i)
                {
                    tnode = malloc(sizeof(MC_MathQuestion) );
                    if(!tnode)
                    {
                        fprintf(stderr, "In generate_list() - allocation failed!\n");
                        delete_list(list);
                        return NULL;
                    }

                    tnode->card = generate_random_flashcard(game);
                    list = insert_node(list, end_of_list, tnode);
                    end_of_list = tnode;
                    //          DEBUGMSG(debug_mathcards, "%d.", list_length(list) );
                }
            }
            else if (length < cl) //if too many questions, chop off tail end of list
            {
                DEBUGMSG(debug_mathcards, "Cutting list to %d questions\n", length);
                end_of_list = find_node(list, length);
                delete_list(end_of_list->next);
                end_of_list->next = NULL;
            }
        }
    }

    /* Here we are just generating random questions, one at a */
    /* time until we have enough                              */
    /* NOTE generate_random_flashcard() has some bugs, so only */
    /* use this method if we need multiple operand questions   */
    else 
    {
        DEBUGMSG(debug_mathcards, "In generate_list() - COMPREHENSIVE method NOT requested\n");

        for (i = 0; i < length; ++i)
        {
            tnode = malloc(sizeof(MC_MathQuestion) );
            if(!tnode)
            {
                fprintf(stderr, "In generate_list() - allocation failed!\n");
                delete_list(list);
                return NULL;
            }

            tnode->card = generate_random_flashcard(game);
            list = insert_node(list, end_of_list, tnode);
            end_of_list = tnode;
        }
    }
    /* Now just put the question_id values in: */

    {
        int i = 1;
        MC_MathQuestion* ptr = list;
        while(ptr->next)
        {
            ptr->card.question_id = i;
            ptr = ptr->next;
            i++;
        }
    }

    return list;
}

/* NOTE - returns 0 (i.e. "false") if *identical*, and */
/* 1 (i.e. "true") if *different* - counterintuitive,  */
/* but same behavior as e.g. strcmp()                  */

static int compare_card(const MC_FlashCard* a, const MC_FlashCard* b)
{
    if (strncmp(a->formula_string, b->formula_string, MC_FORMULA_LEN) )
        return 1;
    if (strncmp(a->answer_string, b->answer_string, MC_ANSWER_LEN) )
        return 1;
    if (a->answer != b->answer);
    return 1;

    return 0; //the cards are identical
}

/* Public functions */

/* allocate space for an MC_Flashcard */
MC_FlashCard MC_AllocateFlashcard(void)
{
    MC_FlashCard ret;

    //NOTE strings now simply hard-coded to MC_FORMULA_LEN (= 40) and
    //MC_ANSWER_LEN (= 5) instead of tailoring them to save a few bytes - DSB
    //  DEBUGMSG(debug_mathcards, "Allocating %d + %d bytes for flashcard\n",
    //            max_formula_size + 1, max_answer_size + 1);
    //  ret.formula_string = malloc( (max_formula_size + 1) * sizeof(char));
    //  ret.answer_string = malloc( (max_answer_size + 1) * sizeof(char));
    //   if (!ret.formula_string || !ret.answer_string)
    //     {
    //     free(ret.formula_string);
    //     free(ret.answer_string);
    //     printf("Couldn't allocate space for a new flashcard!\n");
    //     ret = DEFAULT_CARD;
    //     }
    return ret;
}

//Now a no-op - MC_FlashCard no longer has dynamically allocated strings
void MC_FreeFlashcard(MC_FlashCard* fc)
{
    return;
    //   if (!fc)
    //     return;
    // //  DEBUGMSG(debug_mathcards, "Freeing formula_string\n");
    //   if (fc->formula_string)
    //   {
    //     free(fc->formula_string);
    //     fc->formula_string = NULL;
    //   }
    // //  DEBUGMSG(debug_mathcards, "Freeing answer_string\n");
    //   if (fc->answer_string)
    //   {
    //     free(fc->answer_string);
    //     fc->answer_string = NULL;
    //   }
}

unsigned int MC_MapTextToIndex(const char* text)
{
    int i;
    for (i = 0; i < NOPTS; ++i)
    {
        if (!strcasecmp(text, MC_OPTION_TEXT[i]) )
            return i;
    }
    DEBUGMSG(debug_mathcards, "'%s' isn't a math option\n", text);
    return NOT_VALID_OPTION;
}


//TODO more intuitive function names for access by index vs. by text
void MC_SetOpt(MC_MathGame* game, unsigned int index, int val)
{
    if (index >= NOPTS)
    {
        DEBUGMSG(debug_mathcards, "Invalid math option index: %d\n", index);
        return;
    }

    /* Do some sanity checks before we throw val into the struct: */
    switch(index)
    {
        /* All the booleans must be 0 or 1: */
        case PLAY_THROUGH_LIST:
        case REPEAT_WRONGS:
        case ALLOW_NEGATIVES:
        case FORMAT_ANSWER_LAST:
        case FORMAT_ANSWER_FIRST:
        case FORMAT_ANSWER_MIDDLE:
        case FORMAT_ADD_ANSWER_LAST:
        case FORMAT_ADD_ANSWER_FIRST:
        case FORMAT_ADD_ANSWER_MIDDLE:
        case FORMAT_SUB_ANSWER_LAST:
        case FORMAT_SUB_ANSWER_FIRST:
        case FORMAT_SUB_ANSWER_MIDDLE:
        case FORMAT_MULT_ANSWER_LAST:
        case FORMAT_MULT_ANSWER_FIRST:
        case FORMAT_MULT_ANSWER_MIDDLE:
        case FORMAT_DIV_ANSWER_LAST:
        case FORMAT_DIV_ANSWER_FIRST:
        case FORMAT_DIV_ANSWER_MIDDLE:
        case ADDITION_ALLOWED:
        case SUBTRACTION_ALLOWED:
        case MULTIPLICATION_ALLOWED:
        case DIVISION_ALLOWED:
        case TYPING_PRACTICE_ALLOWED:
        case ARITHMETIC_ALLOWED:
        case COMPARISON_ALLOWED:
        case RANDOMIZE:
        case COMPREHENSIVE:
        case VARY_LIST_LENGTH:
            {
                /* Reset all non-zero values to one: */
                if(val)
                {
                    if(val != 1)
                    {
                        fprintf(stderr, "Warning - parameter %s with invalid value %d, "
                                "resetting to 1\n", MC_OPTION_TEXT[index], val);
                        val = 1;
                    }
                }
                break;
            }

            /* Parameters concerning numbers of questions */
            /* must be greater than or equal to zero:     */
            /* TODO some additional checks would make sense */
        case QUESTION_COPIES:
        case COPIES_REPEATED_WRONGS:
        case MAX_QUESTIONS:
        case MAX_FORMULA_NUMS:
        case MIN_FORMULA_NUMS:
        case AVG_LIST_LENGTH:
            {
                /* Reset all negative values to zero: */
                if(val < 0)
                {
                    fprintf(stderr, "Warning - parameter %s with invalid value %d, "
                            "resetting to 0\n", MC_OPTION_TEXT[index], val);
                    val = 0;
                }
                break;
            }

            /* Operand values - make sure they are in displayable range */
            /* i.e. -999 to 999                                         */ 
        case MAX_ANSWER:
        case MIN_AUGEND:
        case MAX_AUGEND:
        case MIN_ADDEND:
        case MAX_ADDEND:
        case MIN_MINUEND:
        case MAX_MINUEND:
        case MIN_SUBTRAHEND:
        case MAX_SUBTRAHEND:
        case MIN_MULTIPLIER:
        case MAX_MULTIPLIER:
        case MIN_MULTIPLICAND:
        case MAX_MULTIPLICAND:
        case MIN_DIVISOR:
        case MAX_DIVISOR:
        case MIN_QUOTIENT:
        case MAX_QUOTIENT:
        case MIN_TYPING_NUM:
        case MAX_TYPING_NUM:
        case MIN_COMPARATOR:
        case MAX_COMPARATOR:
        case MIN_COMPARISAND:
        case MAX_COMPARISAND:
            {
                if(val > MC_GLOBAL_MAX)
                {
                    fprintf(stderr, "Warning - parameter %s with invalid value %d, "
                            "resetting to %d\n", MC_OPTION_TEXT[index],
                            val, MC_GLOBAL_MAX);
                    val = MC_GLOBAL_MAX;
                }

                if(val < (0 - MC_GLOBAL_MAX))
                {
                    fprintf(stderr, "Warning - parameter %s with invalid value %d, "
                            "resetting to %d\n", MC_OPTION_TEXT[index],
                            val, (0 - MC_GLOBAL_MAX));
                    val = (0 - MC_GLOBAL_MAX);
                }

                break;
            }

        default:
            fprintf(stderr, "Warning - in MC_SetOpt() - unrecognized index %d\n",
                    index);
    }
    /* Should now be safe to put "sanitized" value into struct: */
    game->math_opts->iopts[index] = val;
}

void MC_SetOp(MC_MathGame* game, const char* param, int val)
{
    MC_SetOpt(game, MC_MapTextToIndex(param), val);
}

int MC_GetOpt(MC_MathGame* game, unsigned int index)
{
    if (index >= NOPTS)
    {
        DEBUGMSG(debug_mathcards, "Invalid option index: %d\n", index);
        return MC_MATH_OPTS_INVALID;
    }
    if (!game->math_opts)
    {
        printf("Invalid options list!\n");
        return MC_MATH_OPTS_INVALID;
    }
    return game->math_opts->iopts[index];
}

int MC_GetOp(MC_MathGame* game, const char* param)
{
    return MC_GetOpt(game, MC_MapTextToIndex(param) );
}

int MC_VerifyOptionListSane(void)
{
    return strcmp(MC_OPTION_TEXT[NOPTS], "END_OF_OPTS") == 0;
}

int MC_MaxFormulaSize(void)
{
    return MC_FORMULA_LEN;
}

int MC_MaxAnswerSize(void)
{
    return MC_ANSWER_LEN;
}

void MC_ResetFlashCard(MC_FlashCard* fc)
{
    if (!fc || !fc->formula_string || !fc->answer_string)
        return;
    fc->formula_string[0] = '\0';
    fc->answer_string[0] = '\0';
    fc->answer = -9999;
    fc->difficulty = 0;
    fc->question_id = -1;
}

int MC_FlashCardGood(const MC_FlashCard* fc)
{
    return fc && fc->formula_string && fc->answer_string;
}

int find_divisor(MC_MathGame* game, int a)
{
    int div = 1; //the divisor to return
    int realisticpasses = 3; //reasonable time after which a minimum should be met
    int i;
    do
        for (i = 0; i < NPRIMES; ++i) //test each prime
            if (a % smallprimes[i] == 0)  //if it is a prime factor,
                if (rand() % (i + 1) == 0) //maybe we'll keep it
                    if (div * smallprimes[i] <= MC_GetOpt(game, MAX_DIVISOR) ) //if we can,
                        div *= smallprimes[i]; //update our real divisor
    //keep going if the divisor is too small
    while (div < MC_GetOpt(game, MIN_DIVISOR) && --realisticpasses); 

    return div;
}


//Computes (approximately) the number of questions that will be returned
//by add_all_valid() as specified by the current options. This does not 
//take into account screening out of invalid questions, such
//as divide-by-zero and questions like "0 x ? = 0".
static int calc_num_valid_questions(MC_MathGame* game)
{
    int total_questions = 0;
    int k = 0;
    //First add the number of typing questions
    if (MC_GetOpt(game, TYPING_PRACTICE_ALLOWED))
        total_questions += (MC_GetOpt(game, MAX_TYPING_NUM) - MC_GetOpt(game, MIN_TYPING_NUM));

    //Now add how many questions we will have for each operation:
    for (k = MC_OPER_ADD; k < MC_NUM_OPERS; ++k)
    {
        int num_this_oper = 0;
        int formats_this_oper = 0;

        if (!MC_GetOpt(game, k + ADDITION_ALLOWED) )
            continue;

        //calculate number of ordered pairs of first and second operands:
        //note the "+ 1" is due to the ranges being inclusive
        num_this_oper = (MC_GetOpt(game, MAX_AUGEND + 4 * k) - MC_GetOpt(game, MIN_AUGEND + 4 * k) + 1)
            *
            (MC_GetOpt(game, MAX_ADDEND + 4 * k) - MC_GetOpt(game, MIN_ADDEND + 4 * k) + 1);
        //check what formats are allowed
        if (MC_GetOpt(game, FORMAT_ANSWER_LAST) && MC_GetOpt(game, FORMAT_ADD_ANSWER_LAST + k * 3))
            formats_this_oper++;
        if (MC_GetOpt(game, FORMAT_ANSWER_FIRST) && MC_GetOpt(game, FORMAT_ADD_ANSWER_FIRST + k * 3))
            formats_this_oper++;
        if (MC_GetOpt(game, FORMAT_ANSWER_MIDDLE) && MC_GetOpt(game, FORMAT_ADD_ANSWER_MIDDLE + k * 3))
            formats_this_oper++;
        //Get total of e.g. addition questions:
        num_this_oper *= formats_this_oper;
        //add to overall total:
        total_questions += num_this_oper;
    }

    //TODO will also need to count up the COMPARISON questions once
    //they are implemented
    {
    }

    DEBUGMSG(debug_mathcards, "calc_num_valid_questions():\t%d\n", total_questions);
    return total_questions;
}




//NOTE end_of_list** needs to be doubly indirect because otherwise the end does not
//get updated in the calling code
//NOTE the difficulty is set as add = 1, sub = 2, mult = 3, div = 4, plus a 2 point
//bonus if the format is a "missing number".
MC_MathQuestion* add_all_valid(MC_MathGame* game,
        MC_ProblemType pt,
        MC_MathQuestion* list,
        MC_MathQuestion** end_of_list)
{
    int i, j;
    int ans = 0, tmp;
    MC_Operation k;
    MC_MathQuestion* tnode;

    DEBUGMSG(debug_mathcards, "Entering add_all_valid(%d)\n", pt);
    DEBUGMSG(debug_mathcards, "List already has %d questions\n", list_length(list));

    //make sure this problem type is actually allowed
    if (!MC_GetOpt(game, pt + TYPING_PRACTICE_ALLOWED) )
        return list;

    //add all typing questions in range
    if (pt == MC_PT_TYPING)
    {
        DEBUGMSG(debug_mathcards, "Adding typing...\n");
        for (i = MC_GetOpt(game, MIN_TYPING_NUM); i <= MC_GetOpt(game, MAX_TYPING_NUM); ++i)
        {
            DEBUGMSG(debug_mathcards, "(%d)\n", i);
            tnode = allocate_node();
            if(!tnode)
            {
                fprintf(stderr, "In add_all_valid() - allocate_node() failed!\n");
                delete_list(list);
                return NULL;
            }

            snprintf(tnode->card.formula_string, MC_FORMULA_LEN, "%d", i);
            snprintf(tnode->card.answer_string, MC_ANSWER_LEN, "%d", i);
            tnode->card.answer = i;
            tnode->card.difficulty = 1;
            list = insert_node(list, *end_of_list, tnode);
            *end_of_list = tnode;
        }
    }

    //add all allowed arithmetic questions
    else if (MC_PT_ARITHMETIC)
    {
        DEBUGMSG(debug_mathcards, "Adding arithmetic...\n");

        // The k loop iterates through the four arithmetic operations:
        // k = 0 means addition
        // k = 1 means subtraction
        // k = 2 means multiplication
        // k = 3 means division
        for (k = MC_OPER_ADD; k < MC_NUM_OPERS; ++k)
        {
            if (!MC_GetOpt(game, k + ADDITION_ALLOWED) )
                continue;
            DEBUGMSG(debug_mathcards, "\n*%d*\n", k);

            // The i loop iterates through the first value in the question:
            for (i = MC_GetOpt(game, MIN_AUGEND + 4 * k); i <= MC_GetOpt(game, MAX_AUGEND + 4 * k); ++i)
            {
                DEBUGMSG(debug_mathcards, "\n%d:\n", i);

                // The j loop iterates through the second value in the question:
                for (j = MC_GetOpt(game, MIN_ADDEND + 4 * k); j <= MC_GetOpt(game, MAX_ADDEND + 4 * k); ++j)
                {
                    // Generate the third number according to the operation.
                    // Although it is called "ans", it will not be the actual
                    // answer if it is a "missing number" type problem
                    // (e.g. "3 x ? = 12")
                    // We also filter out invalid questions here
                    switch (k)
                    {
                        case MC_OPER_ADD:
                            {
                                ans = i + j;
                                // throw anything over MAX_ANSWER
                                if (ans > MC_GetOpt(game, MAX_ANSWER))
                                    continue;
                                break;
                            }
                        case MC_OPER_SUB:
                            {
                                ans = i - j;
                                // throw out negatives if they aren't allowed:
                                if (ans < 0 && !MC_GetOpt(game, ALLOW_NEGATIVES))
                                    continue;
                                // throw anything over MAX_ANSWER
                                if (ans > MC_GetOpt(game, MAX_ANSWER))
                                    continue;
                                break;
                            }
                        case MC_OPER_MULT:
                            {
                                ans = i * j;
                                // throw anything over MAX_ANSWER
                                if (ans > MC_GetOpt(game, MAX_ANSWER))
                                    continue;
                                break;
                            }
                        case MC_OPER_DIV:
                            {
                                // throw anything over MAX_ANSWER
                                if (i * j > MC_GetOpt(game, MAX_ANSWER))
                                    continue;

                                tmp = i;
                                i *= j;
                                ans = j;
                                j = tmp;
                                break;
                            }
                        default:
                            fprintf(stderr, "Unrecognized operation type: %d\n", k);
                            continue;
                    }

                    DEBUGMSG(debug_mathcards, "Generating: %d %c %d = %d\n", i, operchars[k], j, ans);

                    //add each format, provided it's allowed in general and for this op

                    // Questions like "a + b = ?"
                    if (MC_GetOpt(game, FORMAT_ANSWER_LAST) && MC_GetOpt(game, FORMAT_ADD_ANSWER_LAST + k * 3))
                    {
                        // Avoid division by zero:
                        if (k == MC_OPER_DIV && j == 0)
                        {
                            // need to restore i and j to original values so loop works:
                            j = ans;
                            i = tmp;
                            continue;
                        }

                        tnode = allocate_node();
                        if(!tnode)
                        {
                            fprintf(stderr, "In add_all_valid() - allocate_node() failed!\n");
                            delete_list(list);
                            return NULL;
                        }

                        snprintf(tnode->card.answer_string, MC_ANSWER_LEN, "%d", ans);
                        //snprintf(tnode->card.formula_string, MC_FORMULA_LEN,
                        //         "%d %c %d = ?", i, operchars[k], j);
                        create_formula_str(tnode->card.formula_string, i, j, k, MC_FORMAT_ANS_LAST);
                        tnode->card.difficulty = k + 1;
                        tnode->card.answer = ans;
                        list = insert_node(list, *end_of_list, tnode);
                        *end_of_list = tnode;
                    }


                    // Questions like "? + b = c"
                    if (MC_GetOpt(game, FORMAT_ANSWER_FIRST) && MC_GetOpt(game, FORMAT_ADD_ANSWER_FIRST + k * 3) )
                    {
                        // Avoid questions with indeterminate answer:
                        // e.g. "? x 0 = 0"
                        if (k == MC_OPER_MULT && j == 0)
                        {
                            continue;
                        }
                        // Avoid division by zero:
                        if (k == MC_OPER_DIV && j == 0)
                        {
                            // need to restore i and j to original values so loop works:
                            j = ans;
                            i = tmp;
                            continue;
                        }

                        tnode = allocate_node();
                        if(!tnode)
                        {
                            fprintf(stderr, "In add_all_valid() - allocate_node() failed!\n");
                            delete_list(list);
                            return NULL;
                        }

                        snprintf(tnode->card.answer_string, MC_ANSWER_LEN, "%d", i);
                        create_formula_str(tnode->card.formula_string, j, ans, k, MC_FORMAT_ANS_FIRST);
                        //snprintf(tnode->card.formula_string, MC_FORMULA_LEN,
                        //         "? %c %d = %d", operchars[k], j, ans);
                        tnode->card.answer = ans;
                        tnode->card.difficulty = k + 3;
                        list = insert_node(list, *end_of_list, tnode);
                        *end_of_list = tnode;
                    }


                    // Questions like "a + ? = c"
                    if (MC_GetOpt(game, FORMAT_ANSWER_MIDDLE) && MC_GetOpt(game, FORMAT_ADD_ANSWER_MIDDLE + k * 3))
                    {
                        // Avoid questions with indeterminate answer:
                        // e.g. "0 x ? = 0"
                        if (k == MC_OPER_MULT && i == 0)
                            continue;

                        // e.g. "0 / ? = 0"
                        if (k == MC_OPER_DIV && i == 0)
                        {
                            // need to restore i and j to original values so loop works:
                            j = ans;
                            i = tmp;
                            continue;
                        }

                        tnode = allocate_node();
                        if(!tnode)
                        {
                            fprintf(stderr, "In add_all_valid() - allocate_node() failed!\n");
                            delete_list(list);
                            return NULL;
                        }

                        snprintf(tnode->card.answer_string, MC_ANSWER_LEN, "%d", j);
                        create_formula_str(tnode->card.formula_string, i, ans, k, MC_FORMAT_ANS_MIDDLE);
                        //snprintf(tnode->card.formula_string, MC_FORMULA_LEN,
                        //         "%d %c ? = %d", i, operchars[k], ans);
                        tnode->card.answer = ans;
                        tnode->card.difficulty = k + 3;
                        list = insert_node(list, *end_of_list, tnode);
                        *end_of_list = tnode;
                    }
                    //If we divided, reset i and j so loop works correctly
                    if (k == MC_OPER_DIV)
                    {
                        j = ans;
                        i = tmp;
                        DEBUGMSG(debug_mathcards, "resetting to %d %d\n", j, i);
                    }
                }
            }
        }
    }
    //add all comparison questions (TODO implement them!)
    else if (pt == MC_PT_COMPARISON)
    {
        for (i = MC_GetOpt(game, MIN_COMPARATOR); i < MC_GetOpt(game, MAX_COMPARATOR); ++i)
        {
            for (j = MC_GetOpt(game, MIN_COMPARISAND); j < MC_GetOpt(game, MAX_COMPARISAND); ++j)
            {
                tnode = allocate_node();
                if(!tnode)
                {
                    fprintf(stderr, "In add_all_valid() - allocate_node() failed!\n");
                    delete_list(list);
                    return NULL;
                }

                snprintf(tnode->card.formula_string, MC_FORMULA_LEN, "%d ? %d", i,j);
                snprintf(tnode->card.answer_string, MC_ANSWER_LEN,
                        i < j ? "<" : 
                        i > j ? ">" : 
                        "=");
                tnode->card.difficulty = 1;
                list = insert_node(list, *end_of_list, tnode);
                *end_of_list = tnode;
            }
        }
    }
    DEBUGMSG(debug_mathcards, "Exiting add_all_valid()\n");  
    DEBUGMSG(debug_mathcards, "List now has %d questions\n\n", list_length(list));

    return list;
}

MC_MathQuestion* find_node(MC_MathQuestion* list, int num)
{
    while (--num > 0 && list)
        list = list->next;
    return list;
}

void reformat_arithmetic(MC_FlashCard* card, MC_Format f)
{
    int i, j;
    char* beg = 0;
    char* end = 0;
    char nans[MC_ANSWER_LEN];
    char nformula[MC_FORMULA_LEN + MC_ANSWER_LEN + 1]; //gets a bit larger than usual in the meantime

    {
        DEBUGMSG(debug_mathcards, "Starting formula: %s\n", card->formula_string);
        //insert old answer where question mark was
        for (i = 0, j = 0; card->formula_string[j] != '?'; ++i, ++j)
            nformula[i] = card->formula_string[j];
        i += snprintf(nformula + i, MC_ANSWER_LEN, "%s", card->answer_string);
        DEBUGMSG(debug_mathcards, "interim formula: %s\n", nformula);
        snprintf(nformula + i, MC_FORMULA_LEN - i, "%s", card->formula_string + j + 1);

        //replace the new answer with a question mark
        if (f == MC_FORMAT_ANS_LAST)
            beg = strrchr(nformula, ' ') + 1;
        if (f == MC_FORMAT_ANS_FIRST)
            beg = nformula;
        if (f == MC_FORMAT_ANS_MIDDLE)
            beg = strchr(nformula, ' ') + 3;
        end = strchr(beg + 1, ' ');
        if (!end)
            end = "";

        //we now have beg = first digit of number to replace, end = the char after
        sscanf(beg, "%s", nans);
        *beg = 0; //sequester the first half of the string
        snprintf(card->formula_string, MC_FORMULA_LEN, "%s?%s", nformula, end);
        DEBUGMSG(debug_mathcards, "New formula: %s\n", card->formula_string);
        snprintf(card->answer_string, MC_ANSWER_LEN, "%s", nans);
        card->answer = atoi(card->answer_string);
    }
}

static int calc_score(int difficulty, float t)
{
    if (t < 0 || difficulty < 1)
        return 0;
    return (difficulty * SCORE_COEFFICIENT)/t;
}

static int create_formula_str(char* formula_str, int n1, int n2, int op, int format)
{
    const char format_strings[MC_NUM_OPERS][MC_NUM_FORMATS][32] = {
        {N_("%d + %d = ?"), N_("? + %d = %d"), N_("%d + ? = %d")},
        {N_("%d - %d = ?"), N_("? - %d = %d"), N_("%d - ? = %d")},
        {N_("%d x %d = ?"), N_("? x %d = %d"), N_("%d x ? = %d")},
        {N_("%d ÷ %d = ?"), N_("? ÷ %d = %d"), N_("%d ÷ ? = %d")}
    };

    if(!formula_str)
        return 0;

    snprintf(formula_str, MC_FORMULA_LEN,
            _(format_strings[op][format]),
            n1, n2);

    DEBUGMSG(debug_mathcards, "n1 = %d\tn2 = %d\top = %d\tformat = "
            "%dformat_strings[op][format] = %s\tformula: %s\n",
            n1, n2, op, format, format_strings[op][format], formula_str);

    return 1;
}

