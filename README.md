# CDR_File_Inventory_Controller

CDR File Transfer Module from Telco Switches to Telco Billing Systems Locations.

The functionality of the CDR File Inventory Controller is to transfer the CDR files from the Telco Switches (MSC, SMSC etc where the CDR files are generated) to the Billing 
System locations to be processed my the Switch Mediation Engines. The Token process will make sure that only one instance of this will be up and running at a perticular time.
ie. it prevents the multiple deamons of this, running. Aditionally CDR file info is logged to the database so these can be processed by the relevant Mediation Engines.
