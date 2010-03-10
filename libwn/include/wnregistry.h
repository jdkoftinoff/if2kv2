#ifndef __WNREGISTRY_H
#define __WNREGISTRY_H

enum WNREGISTRY_AREA
{
	WNREGISTRY_MACHINE = 0,
    WNREGISTRY_USER,
};



class WNRegistry
{
public:

	WNRegistry(
               bool writable,
			   const char *machine_keyname,
			   const char *user_keyname ="",
			   HKEY user_key_base = HKEY_CURRENT_USER
			   );
	~WNRegistry();

	void SelectUser( const char *user_keyname );
	
	long GetInt( 
				const char *section,
				const char *field,
				const char *descriptive_field,
				long default_val
				);

	bool GetString(
				   const char *section,
				   const char *field,
				   const char *descriptive_field,							   
				   const char *default_val,
				   char *buf,
				   int buf_size
				   );
	

	bool WriteInt(
				  WNREGISTRY_AREA area,
				  const char *field,
				  const char *descriptive_field,							  
				  long val
				  );

	bool WriteString(
					 WNREGISTRY_AREA area,
					 const char *field,
					 const char *descriptive_field,								 
					 const char *val
					 );

	bool WriteMachineInt(
						 const char *section,
						 const char *field,
						 const char *descriptive_field,									 
						 long val
						 );

	bool WriteMachineString(
							   const char *section,
							   const char *field,
	 						   const char *descriptive_field,										
							   const char *val
							   );


	bool WriteUserInt(
					  const char *section,
					  const char *field,
					  const char *descriptive_field,								  
					  long val
					  );

	bool WriteUserString(
							   const char *section,
							   const char *field,
							   const char *descriptive_field,									 
							   const char *val
							   );
	
	bool DeleteValue(
					 const char *field,
					 const char *descriptive_field = 0
					);					 

	
	
private:
	
	 HKEY wnregistry_machine_key;
	 HKEY wnregistry_user_key;
	 HKEY wnregistry_curuser_key;
	 HKEY wnregistry_user_key_base;		
};


#endif
