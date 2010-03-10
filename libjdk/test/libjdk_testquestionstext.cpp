#include "jdk_world.h"
#include "jdk_questions.h"

jdk_setting_description question_defaults[] =
{
  { "name", "", "" },
  { "age", "", "" },
  { "like_chicken", "0", "" },
  { "like_beef", "0", "" },
  { "like_chicken_fried", "0", "" },
  { "like_beef_roasted", "0", "" },
  { 0,0,0 }
};

jdk_question questions[] =
{
  { "Welcome to the question tester", jdk_question::type_none, "", "", "", "", false },
  { "First step", jdk_question::type_text, "What is your name?", "name", "", "", false },
  { "Second step", jdk_question::type_number, "How old are you?", "age", "", "", false, 1,200 },
  { "Third step", jdk_question::type_yesno, "Do you like chicken?", "like_chicken", "", "", false },
  { "Fourth step", jdk_question::type_yesno, "Do you like beef?", "like_beef", "", "", false },
  { "Fifth step", jdk_question::type_yesno, "Do you like fried chicken?", "like_chicken_fried", "", "like_chicken", false },	
  { "Sixth step", jdk_question::type_yesno, "Do you like roast beef?", "like_beef_roasted", "", "like_beef", false },		
  { 0 }
};


int main( int argc, char **argv )
{
  jdk_settings_text result;
  jdk_settings_text defaults( question_defaults );
  
  jdk_questions_asker_text asker( questions, defaults, result );
  
  jdk_questions_asker::user_action action=asker.ask_questions();
  
  if( action==jdk_questions_asker::action_done )
  {
    result.save_file( stdout );
    return 0;
  }
  
  return 1;
}
