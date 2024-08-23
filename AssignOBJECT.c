
#include

#include

#include



#include "oid.h"



#include "MibCompiler.h"

#include "AssignOID.h"

#include "AssignOBJECT.h"

#include "symbol.h"





extern int debug,bResolveOIDsNeeded,yyErrorCnt,nCheckOnly;

extern int errno;





QUEUEOBJ *HeadObj = {0},*TailObj = {0};





/*

 * SaveOID

 *

 * Save the compiled OID somewhere...You might want to write things

 * out to a database, etc....

 *

 */

int SaveOID(OIDINFO *pOID)

 {

 int nRc = 0;

 /*

 * Return the outcome.

 */

 return(nRc);

 }





static void RegisterAs(char *pName,char *pOID,int type)

 {

 SYMBOL *pSym;

 /*

 * Allocate space for the symbol.

 */

 pSym = (SYMBOL *) malloc(sizeof(SYMBOL));

 if( pSym )

 {

 /*

 * Fill it in.

 */

 pSym->DefinedLineNo = 0;

 pSym->SymbolName = pName;

 if( pOID ) pSym->SymbolValue = (LIST *) strdup(pOID);

 else pSym->SymbolValue = 0;



 /*

 * Add it to the symbol table.

 */

 AddToSymbolTable(pSym,type);

 }

 else {

 /*

 * Out of memory!

 */

 RestoreErrorMsg("Out of memory\n");

 }

 }





static int IsArray(char *pName)

 {

 SYMBOL *pSym;

 /*

 * See if we know about the symbol already.

 */

 for( pSym = 0; pSym = GetNextSymbol(pSym,ARRAYTABLE); )

 {

 if( strcmp(pName,pSym->SymbolName) == 0 ) return(1);

 }

 return(0);

 }





static char *CheckForColumnEntry(char *pOID)

 {

 SYMBOL *pSym;

 int n;

 /*

 * See if we have this OID as a subset of any array.

 */

 for( pSym = 0; pSym = GetNextSymbol(pSym,ROWTABLE); )

 {

 if( strstr(pOID,(char *)pSym->SymbolValue) ) return(pSym->SymbolName);

 }

 return((char *)0);

 }





static OIDTYPE ProcessType(char *pName,TYPE **pT,int nLineNo)

 {

 TYPE *pNewType;

 SYMBOL *pSym;

 OIDTYPE oidtype,BasicType(char *,TYPE **,int);

 /*

 * Look up the type in the symbol table -- it must be present.

 */

 pSym = FindSymbol((*pT)->Data.TypeName,NEWTYPES);

 if( pSym )

 {

 /*

 * We have something, so resolve it into something we can

 * use.

 */

 pNewType = (TYPE * )pSym->SymbolValue;

 oidtype = BasicType(pSym->SymbolName,&pNewType,pSym->DefinedLineNo);

 if( oidtype != NOTYPE ) *pT = pNewType;

 }

 else {

 /*

 * Not present, so kick it out with an error...

 */

 oidtype = NOTYPE;

 RestoreErrorMsg("Line %d : Illegal SYNTAX type of \"%s\" in OBJECT-TYPE \"%s\"\n",nLineNo,(*pT)->Data.TypeName,pName);

 yyErrorCnt++;

 }



 /*

 * Return the outcome...

 */

 return(oidtype);

 }





static OIDTYPE BasicType(char *pName,TYPE **pT,int nLineNo)

 {

 OIDTYPE type;

 /*

 * Do normal checking of type...

 */

 switch((*pT)->Type)

 {

 case NULLTYPE:

 type = NULLOBJ;

 break;

 case TICKTYPE:

 type = TIMETICKS;

 break;

 case COUNTERTYPE:

 type = COUNTER;

 break;

 case GAUGETYPE:

 type = GAUGE;

 break;

 case INTEGERTYPE:

 case RANGEINTEGER:

 case ENUMINTEGER:

 type = INTEGER;

 break;

 case OBJIDTYPE:

 type = OBJECTID;

 break;

 case IPADDRTYPE:

 type = IPADDR;

 break;

 case NETWORKTYPE:

 type = NETADDR;

 break;

 case OCTETTYPE:

 type = OCTETSTRING;

 break;

 case STRINGTYPE:

 type = DISPLAYSTRING;

 break;

 case SEQOFTYPE:

 case ITEMTYPE:

 case SEQTYPE:

 type = NOTYPE;

 RestoreErrorMsg("Line %d : Illegal SYNTAX type in OBJECT-TYPE \"%s\"\n",nLineNo,pName);

 yyErrorCnt++;

 break;

 case TYPENAME:

 /*

 * We need to resolve this type, to the true thing behind

 * it. Only if we can't resolve things any further do we

 * actually display an error.

 */

 type = ProcessType(pName,pT,nLineNo);

 break;

 }

 return(type);

 }





static void FillInDetails(OIDINFO *pOI,TYPE *pT)

 {

 LIST *pL;

 int i;

 /*

 * Set the default conditions... no specific sub information!

 */

 pOI->OIDsubtype = NOSUBTYPE;



 /*

 * Fill in the other details.

 */

 switch(pT->Type)

 {

 case RANGEINTEGER:

 case OCTETTYPE:

 case STRINGTYPE:

 /*

 * These types can have ranges associated with them.

 * See if the MIB defined them.

 */

 if( pT->Data.Range.Lower || pT->Data.Range.Upper )

 {

 /*

 * We need to store a range in the array.

 */

 i = pOI->NoElems;

 pOI->OIDsubtype = RANGESET;

 pOI->extra[i].Pair.Lower = pT->Data.Range.Lower;

 pOI->extra[i].Pair.Upper = pT->Data.Range.Upper;

 pOI->NoElems = i+1;

 }

 break;

 case ENUMINTEGER:

 /*

 * This one has a lot of data!

 */

 i = pOI->NoElems;

 for( pL = pT->Data.EnumList; pL; pL = (LIST *)pL->Next,i++ )

 {

 /*

 * Store the entry in the 'extra' array.

 */

 pOI->extra[i].EnumData.EnumName = pL->Name;

 pOI->extra[i].EnumData.EnumValue = pL->Number;

 }

 pOI->NoElems = i;

 pOI->OIDsubtype = ENUMSET;

 break;

 }

 }





static void FillInArrayDetails(OIDINFO *pOI,LIST *pIndex)

 {

 int i;

 /*

 * This one has a lot of data!

 */

 for( i = pOI->NoElems ; pIndex; pIndex = (LIST *)pIndex->Next,i++ )

 {

 /*

 * Store the entry in the 'extra' array.

 */

 pOI->extra[i].IndexName = pIndex->Name;

 }

 pOI->NoElems = i;

 pOI->OIDsubtype = ENUMSET;

 }





/*

 * Process the object. We take all the input, process it and place it

 * in a data structure called OIDINFO.

 */

static void SaveObject(char *pName,TYPE *pSyntax,int Access,int Status,

 char *pDesc,char *pRef,LIST *pIndex,LIST *pDefault,

 LIST *pOID,int nLineNo)

 {

 OIDINFO oid;

 char *pRowName;

 /*

 * Do the simple cleanup to initialize the oid structure.

 */

 memset((char *)&oid,0,sizeof(oid));



 /*

 * Make sure that the access and status are okay.

 */

 if( (Access == (int)UNKNOWN) || (Status == (int)UNKNOWN) )

 {

 RestoreErrorMsg("Line : %d Illegal ACCESS or STATUS in definition \"%s\"\n",nLineNo,pName);

 yyErrorCnt++;

 return;

 }

 /*

 * Convert the OID to a string for later reference.

 */

 oid.OIDoid = OIDListToString(pOID);



 /*

 * Assign the easy things...

 */

 oid.OIDname = pName;

 oid.OIDaccess = (OIDACCESS) Access;

 oid.OIDstatus = (OIDSTATUS) Status;

 oid.OIDcomments.Comments = pDesc;



 /*

 * See if this is the "base" of a SEQUENCE OF syntax. If so, then

 * this is an array. We should also have a INDEX list as well. If

 * not, complain.

 */

 if( pSyntax->Type == SEQOFTYPE )

 {

 /*

 * Register this object id as a array for later reference.

 * Remember that the thing on the end of the SEQUENCE OF is

 * an ID!

 */

 RegisterAs(((TYPE *)(pSyntax->Data.TypeList))->Data.TypeName,oid.OIDoid,ARRAYTABLE);



 /*

 * Assign this as the ARRAYTYPE - it is the base of the array.

 */

 oid.OIDflag = ARRAYTYPE;

 oid.OIDtype = NOTYPE;

 oid.OIDsubtype = NOSUBTYPE;



 /*

 * Make sure we don't have an INDEX statement.

 */

 if( pIndex )

 {

 RestoreErrorMsg("Line %d : \"%s\" is an OBJECT-TYPE with an INDEX clause; it is probably misplaced\n",nLineNo,pName);

 yyErrorCnt++;

 }

 }

 else {

 /*

 * We have something that isn't an array definition. So

 * see if it is part of a column, a slice or a leaf.

 */

 if( (pSyntax->Type == TYPENAME) &&

 IsArray(pSyntax->Data.TypeName) )

 {

 /*

 * This is the "slice" of the array; it has no

 * real type - it is an aggregate for all purposes.

 */

 oid.OIDflag = ROWTYPE;

 oid.OIDtype = AGGREGATE;

 oid.OIDsubtype = NOSUBTYPE;



 /*

 * We need to have an INDEX clause for this definition.

 * Complain if we don't.

 */

 if( pIndex == (LIST *)0 )

 {

 ErrorMsg("Line %d : \"%s\" is an OBJECT-TYPE without an INDEX clause; it should have one\n",nLineNo,pName);

 yyErrorCnt++;

 }



 /*

 * Fill in the details we need.

 */

 FillInArrayDetails(&oid,pIndex);



 /*

 * Register this thing for later access.

 */

 RegisterAs(pName,oid.OIDoid,ROWTABLE);

 }

 else {

 /*

 * For these types, we shouldn't have an INDEX

 * field. If so it is probably a misplacement.

 */

 if( pIndex )

 {

 RestoreErrorMsg("Line %d : \"%s\" is an OBJECT-TYPE with an INDEX clause; it is probably misplaced\n",nLineNo,pName);

 yyErrorCnt++;

 }



 /*

 * Assign the type necessary.

 */

 oid.OIDtype = BasicType(pName,&pSyntax,nLineNo);

 if( oid.OIDtype == NOTYPE ) return;



 /*

 * We might have a column entry or leaf. Figure

 * out which by looking at the OID numbers.

 */

 if( pRowName = CheckForColumnEntry(oid.OIDoid) )

 {

 /*

 * For a column type, we store the

 * name of the row in the first

 * entry in the extra[] array.

 */

 oid.extra[0].IndexName = pRowName;

 oid.NoElems = 1;

 oid.OIDflag = COLTYPE;

 }

 else {

 oid.OIDflag = LEAFTYPE;



 /*

 * A leaf always has a 0 at the end

 * of the OID.

 */

 strcat(oid.OIDoid,".0");

 }



 /*

 * This is the hard part... fill in the

 * oid.NoElems value and the array extra!

 */

 FillInDetails(&oid,pSyntax);



 }

 }



 /*

 * We now have assembled everything, and it has now to be written to

 * the database!

 */

 if( (nCheckOnly == 0) && SaveOID(&oid) )

 {

 RestoreErrorMsg("Problem with database : couldn't write record for OBJECT-TYPE \"%s\" (errno = %d uerr_cod = %d)\n",pName,errno);

 }

 }





static void QueueObject(char *pName,TYPE *pSyntax,int Access,int Status,

 char *pDesc,char *pRef,LIST *pIndex,LIST *pDefault,

 LIST *pOID,int nLineNo)

{

 QUEUEOBJ *pQ;

 /*

 * Allocate a queue structure to store everything...

 */

 pQ = (QUEUEOBJ *) malloc(sizeof(QUEUEOBJ));

 if( pQ )

 {

 /*

 * Okay, fill in the details.

 */

 pQ->Name = pName;

 pQ->Syntax = pSyntax;

 pQ->Access = Access;

 pQ->Status = Status;

 pQ->Description = pDesc;

 pQ->Reference = pRef;

 pQ->Index = pIndex;

 pQ->Default = pDefault;

 pQ->OID = pOID;

 pQ->LineNo = nLineNo;



 /*

 * Queue up the thing for later processing. Note that they

 * must be queued in the order they are seen!

 */

 if( HeadObj )

 {

 TailObj->Next = (struct qobject *) pQ;

 }

 else {

 HeadObj = pQ;

 }

 TailObj = pQ;

 pQ->Next = 0;

 }

 else {

 /*

 * Too bad, we couldn't fit everything.

 */

 RestoreErrorMsg("Out of memory\n");

 }

}





/*

 * pName name of the object being defined.

 * pSyntax what type of data is involved.

 * Access what type of access is allowed.

 * Status what is the status of the object.

 * pDesc pointer to description of the object.

 * pRef pointer to reference list.

 * pIndex pointer to table index list.

 * pDefault default value.

 * pOID OID of this object.

 * nLineNo where, in the MIB file it is defined.

 * bSaveOID whether or not the OID and name is to be added

 * to the symbol table.

 */

void ProcessObject(char *pName,TYPE *pSyntax,int Access,int Status,

 char *pDesc,char *pRef,LIST *pIndex,LIST *pDefault,

 LIST *pOID,int nLineNo,int bSaveOID)

 {

 LIST *pNew;

 /*

 * See if the OID supplied can be converted to only numbers, such as

 * 1.3.6

 */

 pNew = ResolveSpecificOID(pOID,pName,0);



 /*

 * We add the passed in OID to the symbol table list. We will use

 * the resolved OID if it is available. Otherwise we use the original!

 */

 if( pNew )

 pOID = pNew;

 else bResolveOIDsNeeded++;

 if( bSaveOID ) AddOIDSymbol(pName,pOID,nLineNo);



 /*

 * Determine what to do... If we were able to resolve the OID at this

 * time we will add the entry to the database. Otherwise, we create

 * a "in-memory" copy and link them altogether for later access.

 */

 if( pNew == pOID )

 {

 /*

 * Save it now.

 */

 SaveObject(pName,pSyntax,Access,Status,pDesc,pRef,pIndex,

 pDefault,pOID,nLineNo);

 }

 else {

 /*

 * Save it for later use -- FinishObjectProcessing

 * will deal with it.

 */

 QueueObject(pName,pSyntax,Access,Status,pDesc,pRef,pIndex,

 pDefault,pOID,nLineNo);

 }

 }





/*

 * Do the second pass of the object definitions, just in case there

 * are some that need to be processed.

 */

void FinishObjectProcessing()

 {

 QUEUEOBJ *pQ;

 /*

 * See if there is any deferred processing to do...

 */

 for( pQ = HeadObj; pQ; pQ = (QUEUEOBJ *)pQ->Next )

 {

 /*

 * Process this object definition.

 */

 ProcessObject(pQ->Name,pQ->Syntax,pQ->Access,pQ->Status,

 pQ->Description,pQ->Reference,pQ->Index,

 pQ->Default,pQ->OID,pQ->LineNo,0);

 }

 }

