#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "tuxmath.h"
#include "credits.h"
#include "titlescreen.h"
#include "SDL_extras.h"
#include "game.h"
#include "fileops.h"

#define TESTING_CAMPAIGN

#define NUM_STAGES 5 
#define NUM_ROUNDS 3


static char* briefings[NUM_STAGES][50] = {
  //cadet
  {
    "-[Esc] to skip",
    "Mission One: Careful Cadet",
    "--------------------------",
    "Welcome, Tux!",
    "",
    "Congratulations on your graduation from the ",
    "Math Command Training Acedemy. ",
    "",
    "Your arrival to the Galactic Math Command Fleet",
    "comes just in time. The distant star Mathematica",
    "has gone supernova, and parts of its solar",
    "system are now traveling toward the planet FOSS.",
    "",
    "Mathematican asteroids are made of a material",
    "called Undotrium, a mysterious metal that is",
    "known to be very hard to destroy. But Galactic",
    "scientists think they have found a way to do",
    "so. Powerful computers connected to a Lambda",
    "Laser can use numbers to locate Undotrium comets,",
    "aim and shoot a perfect beam that will turn it",
    "into harmless snow. But first, you need to look",
    "closely at the comet and tell the Lambda Laser",
    "Computer what number it needs to use. There are",
    "many different types of comets, and it is up to",
    "up to you to figure out these numbers!",
    "",
    "Tux, your first mission as a Cadet will be to",
    "help the peaceful penguins of FOSS. The penguins",
    "are afraid to leave their igloos, and they need",
    "Math Command help to keep them safe. Igloos can",
    "protect penguins from Undotrium, but they will",
    "melt if they're hit more than once, and then",
    "the penguin will be without a home. Do not let",
    "that happen!",
    "",
    "-IMPORANT",
    "There is one more thing you should know. Certain",
    "comets are made of a more powerful type of",
    "Undotrium. You will know these comets when you",
    "see them, by their red color. If you can shoot",
    "a red comet, you may be able to use it to build",
    "additional igloos."
    "",
    "",
    "Good luck, Cadet.",
    NULL
  },
  //scout
  {
    "-[Esc] to skip",
    "Mission Two: Smart Scout",
    "------------------------",
    "Great job, Tux. Your performance on FOSS was",
    "brilliant and the penguins give you their thanks.",
    "After such a show of skill and smarts, we feel",
    "that your training as a Cadet must be complete.",
    "We are pleased to promote you to the rank of",
    "Galactic Math Scout. As a gift for your",
    "accomplishment, you are getting your own Lambda",
    "Laser Computer. And it will come in handy...",
    "You need to go on another mission right away!",
    "",
    "Already, another wave of comets is headed toward",
    "FOSS. This time, things will not be so easy.",
    "Radar scans of the new comets show signs of",
    "subtractive Undotrium. Before, you have seen",
    "comets such as \"2+3=5\". Now you will begin to",
    "see \"5-2=3\" and \"5-2=3\". Subtractive",
    "Undotrium is the opposite of normal Undotrium,",
    "just like subtraction is the opposite of",
    "addition. Don't let it trick you!",
    "",
    "Sometimes, you may need to figure out a very",
    "large subtractive comet. The best strategy for",
    "doing so is to start at the lower number and",
    "count upward until you reach the higher number.",
    "The number you count will be the answer, or as",
    "we call it in the Fleet, the difference.",
    "",
    "You can do it, Tux! Show us how good you are.",
    NULL
  },
  //ranger
  {
    "-[Esc] to skip",
    "Mission Three: Royal Ranger",
    "---------------------------",
    "You've done it again. The Penguin Emperor, his",
    "majesty the Great Auk, has heard of your math",
    "skills and wants to congratulate you himself.",
    "You have been invited to the Royal Igloo for a",
    "celebration, where the Emperor will personally",
    "offer a token of gratitude. He tells us it's a",
    "surprise."
    "",
    "-........",
    "-@**>##;7^^PLEaSe StaND bY",
    "**EMERGENCY ROYAL TRANSMISSION**",
    "-Help! Save us! We....comets.....the penguins...",
    "-...must come to...if....don't....go",
    "**END OF TRANSMISSION**",
    "",
    "It sounds like the Royal Igloo is in trouble.",
    "We weren't able to decode the whole transmission,",
    "but the Emperor definitely said that there were",
    "more comets. We are sending you in right away.",
    "",
    "Intel shows a new strain of Undotrium comets that",
    "are many times larger than the ones you have seen",
    "already. These multiplicative comets are made of",
    "huge amounts of additive Undotrium. To solve them",
    "you will need to add over and over again. Just",
    "one comet can have a question like 3*6, which is",
    "the same as 3+3+3+3+3+3!"
    "",
    "Be careful, Tux. We have also picked up signs of",
    "comets that are very different from the ones",
    "we've seen before. They have numbers that are",
    "less than zero. These negative numbers do",
    "strange things to addition and subtraction. If",
    "a negative number is added, it is really",
    "subtracted, and if it is subtracted, it's really",
    "added."
    //TODO better explanation of negatives
    "Also, if two numbers are multiplied and one of",
    "them is negative, the answer will be negative",
    "also. But if both numbers are negative, the",
    "answer is positive! Be careful."
    "",
    "We are making you a Ranger right away, and we",
    "hope that you will give the Emperor and his",
    "citizens extra care. We know you will do so.",
    NULL
  },
  //ace
  {
    "-[Esc] to skip",
    "Ace",
    "",
    NULL
  },
  //commando
  {
    "-[Esc] to skip",
    "Commando",
    "",
    NULL
  },
};

void start_campaign();

#endif // CAMPAIGN_H
