

//////////////////////////////////////////////////////////////////////
//
// Token.cpp - implementation of the token class
// CDR File Inventory Controller
// developer - Chandika Nettasinghe
// code start - 2002-Sept
//
//////////////////////////////////////////////////////////////////////


#include <fstream.h>
#include "Token.h"

extern ofstream f_Error;

token::token(string f_name)
{
	token_state = T_UNDEF;
	file_name = f_name;
}	

bool token::is_runable(void)
{
	if( (token_file  = fopen( file_name.c_str(), "r" )) == NULL )
	{
		f_Error << "The file does not exsist : " << file_name.c_str() << endl;
		return false;
	}
	char buf[80];
	fscanf(token_file, "%s", buf );
	
	if( fclose(token_file) )
		f_Error << "The file was not closed. : " << file_name.c_str() << endl;

	token_state = code_token(buf);

	return(token_state == T_OFF);
}


bool token::set_token(e_token_state a)
{
	if( (token_file  = fopen( file_name.c_str(), "w" )) == NULL )
	{
		f_Error << "ERROR: file could not be opened to set state : " << file_name.c_str() << endl;
		return false;
	}
	
	fprintf(token_file, "%s", decode_token(a).c_str() );
	
	if( fclose(token_file) )
	{
		f_Error << "file was not closed after set state : " << file_name.c_str() << endl;
		return false;
	}

	return(true);
}

e_token_state token::code_token(string a)
{
	if("ON"==a)
		token_state = T_ON;
	else if ("OFF"==a)
		token_state = T_OFF;
	else 
		token_state = T_UNDEF;

	return token_state;
}

string token::decode_token(e_token_state a)
{
	switch(a)
	{
	case T_ON:
		token_text="ON";
		break;
	case T_OFF:
		token_text="OFF";
		break;
	case T_UNDEF:
		token_text="UNDEF";
		break;
	default:
		token_text="UNDEF";
	}
	return token_text;
}

