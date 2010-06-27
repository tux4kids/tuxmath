/*
  parse_xmlLesson.c

  For TuxMath
  Parse XML lesson files.

  by Vikas Singh
  vikassingh008@gmail.com

  Part of "Tux4Kids" Project
  http://www.tux4kids.com
*/


#include <stdio.h>
#include <stdlib.h>
//#include <gtk/gtk.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include"parse_xmlLesson.h"
#include"schoolmode.h"
#include"factoroids.h"
//#include"parse_xmlLesson.h"
int serial_number,i;
int total_no_menus;
 xmlChar *wave;
void parse_fractions(xmlNode *);
void parse_factors(xmlNode *);
  char menu_names[MAX_MENU_ITEMS][MENU_ITEM_LENGTH] = {{'\0'}};
char wave_string[1][5]={{'\0'}}; //used in sprintf to convert finally to integer corrresponding to a particular wave no

int waves_parsed[MAX_WAVES+1]={-1};
int parse_xmlLesson()
{
xmlNode *cur_node;
  char fn[4096];
char *lesson_path = "schoolmode/lessonData.xml";
  snprintf(fn, 4096, "%s/images/%s", DATA_PREFIX, lesson_path);

  //xmlChar *Num_asteroids;
//int serial_number;

  // --------------------------------------------------------------------------
  // Open XML document
  // --------------------------------------------------------------------------

  xmlDocPtr doc;
  doc = xmlParseFile(fn);

  if (doc == NULL) 
        printf("error: could not parse file lessonData.xml\n");
  else 
        printf("parsed file lessonData.xml\n");
  // --------------------------------------------------------------------------
  // XML root.
  // --------------------------------------------------------------------------

  /*Get the root element node */
  xmlNode *root = NULL;
  root = xmlDocGetRootElement(doc);
  
  // --------------------------------------------------------------------------
  // Must have root element, a name and the name must be "lessonData"
  // --------------------------------------------------------------------------
  
  if( !root || 
      !root->name ||
      xmlStrcmp(root->name,(const xmlChar *)"lessonData") ) 
  {
     xmlFreeDoc(doc);
     return 0;
  }

  // --------------------------------------------------------------------------
  // lessonData children: For each factors
  // --------------------------------------------------------------------------


  for(  i=0 , cur_node = root->children    ;   cur_node != NULL   ;    cur_node = cur_node->next)
  {
     if ( cur_node->type == XML_ELEMENT_NODE  &&
          !xmlStrcmp(cur_node->name, (const xmlChar *) "factors" ) )
     {  
            sprintf(menu_names[i], "%s", cur_node->name); 
            i++;
               //menu_names[i]=(char *)cur_node->name;
     }

   else if ( cur_node->type == XML_ELEMENT_NODE  &&
          !xmlStrcmp(cur_node->name, (const xmlChar *) "fractions" ) )
     {  
                sprintf(menu_names[i], "%s", cur_node->name); 
                i++;
     }

  }

total_no_menus=i;
printf("\nno of levels in parse == %d",total_no_menus);

for(i=0;i<3;i++)
 printf("\n i : %s",menu_names[i]);


if (cur_node)
{
cur_node=NULL;
}

//  root = xmlDocGetRootElement(doc);

  for(i=0 , cur_node = root->children    ; cur_node != NULL   ;      cur_node = cur_node->next)
  {
     if ( cur_node->type == XML_ELEMENT_NODE  &&
          !xmlStrcmp(cur_node->name, (const xmlChar *) "factors" ) )
     {  
        display_screen(i);  // i decides the next game to be played
       i++;   
       parse_factors(cur_node);
       factoroids_schoolmode(0);
 
     }

   else if ( cur_node->type == XML_ELEMENT_NODE  &&
          !xmlStrcmp(cur_node->name, (const xmlChar *) "fractions" ) )
     {  
        display_screen(i);  // i decides the next game to be played
        i++;          
        parse_fractions(cur_node);
        //factoroids_schoolmode(1);
     }

  }
  // --------------------------------------------------------------------------

  /*free the document */
  xmlFreeDoc(doc);

  /*
   *Free the global variables that may
   *have been allocated by the parser.
   */
  xmlCleanupParser();

  return 0;
}



void parse_factors(xmlNode *cur_node)
{
 xmlNode *child_node;
int i=0;
       printf("Element: %s \n", cur_node->name); 
        serial_number=0;

        // For each child of factors: i.e. wave
        for(child_node = cur_node->children; child_node != NULL; child_node = child_node->next)
        {
           if ( cur_node->type == XML_ELEMENT_NODE  &&
                !xmlStrcmp(child_node->name, (const xmlChar *)"wave_factors") )
           {
              serial_number++; 
              printf("   Child=%s\n", child_node->name);
              printf("         Serial number=%d\n", serial_number);   
              
            
              wave= xmlNodeGetContent(child_node);
              if(wave)
               {
                 printf("         Wave: %s\n", wave);
                 sprintf(wave_string[0], "%s", wave); 
                 waves_parsed[i++]=atoi(wave_string[0]);
               }
              xmlFree(wave);
           }          
         }
}



void parse_fractions(xmlNode *cur_node)
{
 xmlNode *child_node;


 printf("Element: %s \n", cur_node->name); 
        serial_number=0;

        // For each child of fractions: i.e. wave
        for(child_node = cur_node->children; child_node != NULL; child_node = child_node->next)
        {
           if ( cur_node->type == XML_ELEMENT_NODE  &&
                !xmlStrcmp(child_node->name, (const xmlChar *)"wave_fractions") )
           {
              serial_number++; 
              printf("   Child=%s\n", child_node->name);
              printf("         Serial number=%d\n", serial_number);   
              
            
              wave= xmlNodeGetContent(child_node);
              if(wave) printf("         Wave: %s\n", wave);
              xmlFree(wave);
           }          
         }
}
