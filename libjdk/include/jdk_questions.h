#ifndef _JDK_QUESTIONS_H
#define _JDK_QUESTIONS_H

#include "jdk_settings.h"

struct jdk_question
{
	enum qtype
	{
		type_none,
		type_yesno,
		type_text,
		type_password,
		type_multiline,
		type_multiplechoice,
		type_path,
		type_number,
		type_hidden,
		type_text_readonly,
		type_multiline_text_file
	};
	
	char *title;
	qtype type;
	char *question;
	char *field;
	char *multichoices;
	char *enabled_on_field;
	bool chained_forward;
	int lower_limit,upper_limit;	
};

class jdk_questions_asker
{
    jdk_questions_asker( const jdk_questions_asker & );
    const jdk_questions_asker & operator = ( const jdk_questions_asker & );
    
public:
	enum user_action
	{
		action_ok,
		action_done,
		action_cancel,
		action_next,
		action_previous
	};	
	
	jdk_questions_asker(
						const jdk_question *question_list_,
						const jdk_settings &defaults_,
						jdk_settings &result_
						)
	  : 
	question_list( question_list_ ),
	defaults( defaults_ ),
	result( result_ )
	{
		
	}
	
	virtual ~jdk_questions_asker()
	{
	}
	
	virtual user_action ask_questions();
	

protected:

	virtual user_action ask_none(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_yesno(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_text(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_password(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_multiline(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_path(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_number(const jdk_question *q,bool first,bool last) = 0;
	virtual user_action ask_multiplechoice(const jdk_question *q,bool first,bool last) = 0;
	
	
	const jdk_question *question_list;
	const jdk_settings &defaults;
	jdk_settings &result;
};


class jdk_questions_asker_text : public jdk_questions_asker
{
public:
	jdk_questions_asker_text(
						const jdk_question *question_list_,
						const jdk_settings &defaults_,
						jdk_settings &result_
						);
	
	~jdk_questions_asker_text();
	
protected:
	user_action ask_none(const jdk_question *q,bool first,bool last);	
	user_action ask_yesno(const jdk_question *q,bool first,bool last);
	user_action ask_text(const jdk_question *q,bool first,bool last);
	user_action ask_password(const jdk_question *q,bool first,bool last);
	user_action ask_multiline(const jdk_question *q,bool first,bool last);
	user_action ask_path(const jdk_question *q,bool first,bool last);
	user_action ask_number(const jdk_question *q,bool first,bool last);
	user_action ask_multiplechoice(const jdk_question *q,bool first,bool last);
	
};

#endif
