

//////////////////////////////////////////////////////////////////////
//
// db.h - methods for database interaction
// CDR File Inventory Controller
// developer - Chandika Nettasinghe
// code start - 2002-Sept
//
//////////////////////////////////////////////////////////////////////


#ifndef DB_H
#define DB_H

	void SqlError(char*);
	void db_connect(char*);
	void db_disconnect();
	int  db_get_seq_no(char*);
	void db_insert_file(char*, char*,int);

#endif
