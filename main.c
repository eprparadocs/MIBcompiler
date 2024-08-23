
#include

#include

#include



extern char getopt(),*optarg,*getenv(),*Version;

extern int debug,optind,bNoStandardOIDs,bDoingTraps;

extern int nCheckOnly,nErrorCount;



#define PATH_MAX 512


/*

 * InitDB

 *

 * Do any necessary database initialization.

 */

void InitDB(void)

 {

 }





/*

 * SaveMainOIDFiles

 *

 * Save the main database files just in case we need to restore

 * them later. This happens when the MIB has an error in it, and

 * the underlying database system isn't capable of dealing with

 * multiple definitions.

 */

static int SaveMainOIDFiles()

 {

 int nRc = 0;

 /*

 * Return the results.

 */

 return(nRc);

 }





/*

 * RecoverMainOIDFiles

 *

 * Restore the main database files. This routine is called whenever one

 * or more "fatal" errors are uncovered in the MIB.

 */

static void RecoverMainOIDFiles()

 {

 }





/*

 * CreateOIDFiles

 *

 * Create any necessary database files needed to hold the compiled MIB.

 */

static int CreateOIDFiles(char *pName)

 {

 int nRc = 0;

 /*

 * Return the results.

 */

 return(nRc);

 }





/*

 * CloseOIDFiles

 *

 * Close any necessary database files.

 */

static void CloseOIDFiles(void)

 {

 }





/*

 * ProcessRequest

 *

 * Compile a MIB definition.

 */

static void ProcessRequest(char *pName)

 {

 int nRc = 0;

 static char filename[PATH_MAX];

 char *pStr;

 /*

 * Reset the error count for this mib. This error count is the fatal

 * error count; if it is non-zero it will force a call to restore

 * the database files.

 */

 nErrorCount = 0;



 /*

 * Create a filename for the database. Here we strip off the extension

 * and provide a suffix O. For example, a mib called mib2.my, would

 * have a database file name of Omib2. We also handle the path

 * name part, as well.

 */

 pStr = strrchr(pName,'/');

 if( pStr )

 {

 /*

 * It has a path part...

 */

 strncpy(filename,pName,(pStr - pName + 1));

 strcat(filename,"O");

 strcat(filename,pStr+1);

 pStr = strchr(filename,'.');

 if( pStr ) *pStr = '\0';

 }

 else {

 /*

 * It has no path!

 */

 strcpy(filename,"O");

 strcat(filename,pName);

 pStr = strchr(filename,'.');

 if( pStr ) *pStr = '\0';

 }

 pName = filename;



 /*

 * Create an appropriate private MIB database file, but only if this

 * isn't a check.

 */

 if( nCheckOnly == 0 )

 nRc = CreateOIDFiles(pName);

 if( nRc )

 {

 ErrorMsg("Couldn't create MIB db file %s\n",pName);

 }

 else {

 /*

 * Save the main OID files, just in case.

 */

 if( !nCheckOnly && SaveMainOIDFiles() )

 {

 ErrorMsg("Couldn't save main database files\n");

 }



 /*

 * Process the input file. If it failed, delete the private

 * MIB file.

 */

 if( yyparse() || nErrorCount )

 {

 /*

 * It failed. So close the file out and delete it.

 */

 if( !nCheckOnly )

 {

 CloseOIDFiles();

 unlink(pName);



 /*

 * We should also recover the main

 * oid files as well.

 */

 RecoverMainOIDFiles();

 printf("Database recovered.\n");

 }

 }

 else {

 /*

 * It worked!

 */

 if( !nCheckOnly ) CloseOIDFiles();

 }

 }

 }





main(argc,argv)

 int argc;

 char *argv[];

 {

 char c,*pStr;

 int nRc;

 /*

 * If the name of this is MIBcheck, we will only check the mib.

 */

 pStr = strrchr(argv[0],'/');

 if( pStr ) pStr++;

 else pStr = argv[0];

 if( strcmp(pStr,"MIBcheck") == 0 )

 {

 nCheckOnly = 1;

 }



 /*

 * Collect any run time flags...

 */

 while( (c = getopt(argc,argv,"cd:lv")) != -1 )

 switch(c)

 {

 case 'c' :

 /*

 * Check only.

 */

 nCheckOnly = 1;

 break;



 case 'd' :

 /*

 * Set debug flag.

 */

 debug = atoi(optarg);

 break;



 case 'v' :

 /*

 * Display the current version number.

 */

 printf(Version,pStr);

 break;



 default :

 ErrorMsg("Unknown option -- %c\n",c);

 exit(1);

 }



 /*

 * Do any onetime initialization necessary.

 */

 if( nCheckOnly ==0 )

 {

 /*

 * Init the database system, as needed.

 */

 InitDB();

 }



 /*

 * Process all the file names that follow.

 */

 for( ; optind < argc; optind++ )

 {

 /*

 * This is easy - we take the file name, and open it as

 * standard input.

 */

 if( freopen(argv[optind],"r",stdin) )

 {

 /*

 * Okay. Process the request.

 */

 ProcessRequest(argv[optind]);

 }

 else {

 ErrorMsg("Couldn't open file %s\n",argv[optind]);

 }

 }



 /*

 * And exit.

 */

 exit(0);

 }



