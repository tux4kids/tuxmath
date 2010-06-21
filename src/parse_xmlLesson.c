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
int serial_number;
 xmlChar *wave;


int parse_xmlLesson()
{

xmlNode *cur_node;
  //xmlChar *Num_asteroids;
//int serial_number;

  // --------------------------------------------------------------------------
  // Open XML document
  // --------------------------------------------------------------------------

  xmlDocPtr doc;
  doc = xmlParseFile("lessonData.xml");

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

  for(cur_node = root->children; cur_node != NULL; cur_node = cur_node->next)
  {
     if ( cur_node->type == XML_ELEMENT_NODE  &&
          !xmlStrcmp(cur_node->name, (const xmlChar *) "factors" ) )
     {  
        parse_factors(cur_node);
     }

   else if ( cur_node->type == XML_ELEMENT_NODE  &&
          !xmlStrcmp(cur_node->name, (const xmlChar *) "fractions" ) )
     {  
               parse_fractions(cur_node);
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
              if(wave) printf("         Wave: %s\n", wave);
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

