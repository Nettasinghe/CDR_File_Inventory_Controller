

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CDR File Inventory Controller -
// The functionality of the CDR File Inventory Controller is to transfer the CDR files from the 
// Telco Switches (MSC, SMSC etc where the CDR files are generated) to the Billing System locations
// to be processed my the Switch Mediation Engines. The Token process will make sure that only one 
// instance of this will be up and running at a perticular time. ie. it prevents the multiple deamons
// of this, running. Aditionally CDR file info is logged to the database so these can be processed by 
// the relevant Mediation Engines.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// main.cpp
// CDR File Inventory Controller
// main method contains here
// developer - Chandika Nettasinghe
// code start - 2002-Sept
//
//////////////////////////////////////////////////////////////////////


//newFICtro for one destination
// version 1.0.0

#include <iostream.h>
#include <stdio.h>
#include <errno.h>
#include <fstream.h>
#include <stdlib.h>
#include <time.h>

#include "db.h"
#include "Token.h"

ofstream f_Error("./logs/FICtro_ERROR.log", ios::out | ios::app);
token run_ticket("./bin/FICtro.ticket");

char zTime[30];



int main(int argc, char *argv[])
{
	/*------- get time -------*/
	time_t t;
	time(&t);
	/*------- get time -------*/

	char zFileName[50];
	strftime(zTime, 30, "%Y%B%d", localtime(&t));
	sprintf(zFileName, "./logs/FICtro_%s.log", zTime);
	ofstream f_Fictro(zFileName, ios::out | ios::app);

	strftime(zTime, 30, "%Y%B%d_%H%M", localtime(&t));

	// tokens to prevent multiple instances running
	if(run_ticket.is_runable())
	{
		run_ticket.set_token(T_ON);
	}
	else
	{
		f_Error << " No ticket to run" << endl;
		exit(0);
	}

	FILE* fp_LOG;
	FILE* fp_DumpFile;
	FILE* fp_LastFile;
	FILE* fp_TestMount;

	char zBuffer[100];
	char zLastFile[100];
	char zCmd[500];
	
	int iFileIdNumber;
	int iPathLength;
	
	bool bDB_Updated = false;
	
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//  Creates a log consists of recently added file names.
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	//system("mkdir COPIED_FILES");

	if (argc == 6)
	{
		char zSourcePath[100];
		char zDestinationPath[100];
		char zMountTestPath[100];
		char zTableName[30];

		//------------------- check mount points start ----------
		sprintf(zMountTestPath, "%s.mount_test_file", argv[1]);
		if ((fp_TestMount = fopen(zMountTestPath, "r")) == NULL)
		{
			f_Error << "Mount point failure. Can't access \".mount_test_file\"\n";	
			run_ticket.set_token(T_OFF);	// release lock
			exit(0);
		}
		fclose(fp_TestMount);


		//------------------- check mount points end ------------

		sprintf(zSourcePath, "%s", argv[1]);
		sprintf(zDestinationPath, "%s", argv[2]);
		sprintf(zTableName, "%s", argv[4]);

		int i = 0;
		while (zSourcePath[i] != '\0')
			i++;
		if (zSourcePath[i-1] != '/')
		{	
			zSourcePath[i]   = '/';
			zSourcePath[i+1] = '\0';
			iPathLength = i+1;
		}
		else
			iPathLength = i;
		
		//sprintf(zCmd, "ls -t1 %s*.BGwOK|head -1", zSourcePath);
		sprintf(zCmd, "ls -t1 %s%s|head -1", zSourcePath, argv[5]);
		if ((fp_LastFile = popen(zCmd, "r")) == NULL)
		{
			f_Error << "Error in opening the pipe for read." << endl;
			run_ticket.set_token(T_OFF);    // release lock
			return -1;
		}
		if (fscanf(fp_LastFile, "%s", zBuffer) == EOF)
		{
			f_Error << "There are no files to copy." << endl;
			run_ticket.set_token(T_OFF);    // release lock
			return -1;
		}
		strcpy(zLastFile, zBuffer);
		f_Fictro << "\nLast Dump File at: " << zTime << " : " << zLastFile << endl;
			
		sprintf(zCmd, "ls %s%s", zSourcePath, argv[5]);
		if ((fp_LOG = popen(zCmd, "r")) == NULL)
		{
			f_Error << "Error in opening the \"files.log\" file for read." << endl;
			run_ticket.set_token(T_OFF);    // release lock
			return -1;
		}

		// //  Connects to the database to updates the info.
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++

		db_connect(argv[3]);
		iFileIdNumber = db_get_seq_no(argv[4]);
		db_disconnect();


		if (iFileIdNumber<0) 
			iFileIdNumber = 0;
		iFileIdNumber++;


		while (fscanf(fp_LOG, "%s", zBuffer) != EOF)
		{
			if ((fp_DumpFile = fopen(zBuffer, "a+")) == NULL)
				f_Error << "The file " << zBuffer << " accessing problem" << endl;
			else if(strcmp(zBuffer, zLastFile) != 0)
			{
                                // ++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                //  Filter the filename from the full path
                                // ++++++++++++++++++++++++++++++++++++++++++++++++++++++
				i = 0;
				while (zBuffer[iPathLength+i] != '\0')
				{	
					zFileName[i] = zBuffer[iPathLength+i];
					i++;
				}
				zFileName[i] = '\0';
				sprintf(zFileName, "%s_%s", zFileName, zTime);

				// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//  Copies those files to the destination folder.
				//  Renames the source file names which have copied.
				// ++++++++++++++++++++++++++++++++++++++++++++++++++++++

				sprintf(zCmd, "cp %s %s%s", zBuffer, argv[2], zFileName);
				i = system(zCmd);
				//f_Fictro << zBuffer << " successfully copied" << endl;

				db_connect(argv[3]);
				db_insert_file(argv[4], zFileName, iFileIdNumber);
				//f_Fictro << zBuffer << " successfully inserted to DB" << endl;
				db_disconnect();

				sprintf(zCmd, "mv %s %s_%s", zBuffer, zBuffer, zTime);
				i = system(zCmd);

				f_Fictro << zBuffer << " successfully transfered." << endl;

				bDB_Updated = true;
				iFileIdNumber++;
			}
		}
	}
	else
	{
		f_Error << "Incompatible number of arguments!!!" << endl;
		f_Error << "Usage: newFICtro <source_path> <destination_path>  <db_connect_string> 
			<< <table> <file_format>" << endl;
		run_ticket.set_token(T_OFF);    // release lock
		return -1;
	}

	run_ticket.set_token(T_OFF);    // release lock
	return 0;
}
