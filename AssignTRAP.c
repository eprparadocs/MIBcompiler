
#include



#include "MibCompiler.h"

#include "AssignTRAP.h"





extern int debug,nCheckOnly;





/*

 * pName name of the trap

 * pOID pointer to linked list that is the enterprise OID

 * pVar pointer to linked list of variables that are part

 * of the trap message

 * pDescr pointer to character string that is a description of

 * the trap

 * pRef pointer to character string that is the reference of

 * the trap

 * TrapNo trap number

 * nLineNo start of definition of trap in .my file.

 *

 */

void ProcessTrap(char *pName,LIST *pOID,LIST *pVar,

 char *pDescr,char *pRef,int TrapNo,int nLineNo)

 {

 /*

 * First, see if we are going to display the trap info.

 */

 if( debug )

 {

 printf("Trap name %s (number %d)\n",pName,TrapNo);

 printf("\tEnterprise ");PrintList(pOID,0);printf("\n");

 printf("\tReference %s\n",pRef);

 printf("\tDescription %s\n",pDescr);

 printf("\tVariables ");PrintList(pVar,0);printf("\n\n");

 }

 }



