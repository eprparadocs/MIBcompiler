
%{

/*

 * Copyright 1992 C. Wegrzyn

 *

 */

#include



#include "oid.h"



#include "MibCompiler.h"

#include "AssignOID.h"

#include "AssignTRAP.h"

#include "AssignOBJECT.h"

#include "AssignTYPE.h"





extern char *ShouldBeKeyword(char *);



extern KEYTYPES eNoKeyWords;

extern char yytext[];

extern int yylineno,yyErrorCnt;



int bNoStandardOIDs = {0},bDoingTraps = {0};

int bResolveOIDsNeeded = {0},nLineNoOfDef;

int debug = {0},nCheckOnly = {0};

%}



%token ACCESS BGIN COUNTER DEFINITIONS DEFVAL DEPRECATEDOBJECT

 DESCRIPTION DOTDOT END ENTERPRISE EXPORTS FROM GAUGE ID

 IDENTIFIER IMPORTS INDEX INTEGER IPADDRESS MANDATORYOBJECT

 NETWORKADDRESS NOTACCESSIBLEOBJECT NULLID OCTET OCTSTR OBJECT

 OBJECTTYPE OBSOLETEOBJECT OF OPAQUE OPTIONALOBJECT

 READONLYOBJECT READWRITEOBJECT REFERENCE SEQUENCE SIZE

 STATUS STRING SYNTAX TIMETICKS TRAPTYPE VARIABLES

 WRITEONLYOBJECT NAME CCE SEMICOLON COMMA LITSTRING LBRACE

 RBRACE LITNUMBER LPAREN RPAREN ERROR



%union

 {

 PAIR yy_pair;

 LIST *yy_list;

 char *yy_string;

 long yy_number;

 TYPE *yy_type;

 int yy_int;

 }



%type RangeOfValues OctetSize

%type OIDSubId OIDList ObjectId OptObjectId Data Default

 IndexList Index IndexElement VarList Variables

 NamedNumber NamedNumberList ModuleName

%type ReferenceLine DescriptionLine LITSTRING NAME ID

%type Access AccessLine Status StatusLine LITNUMBER

%type NullType TimeTickType CounterType ObjectIdType

 OptionalType GaugeType IpAddressType BuiltInType

 NetworkAddressType IntegerType OctetType

 OptionalTypeList SeqList SequenceType SequenceOfType

 Type DefinedType SyntaxLine SyntaxType

%type TrapType TypeDefinition ObjectIdAssignment ObjectType

 $MARK ImportItem



%%



ModuleDefinition

 :

 {

 /*

 * Add in the standard stuff from the SMI RFC; we won't

 * do it if told not to.

 */

 if( bNoStandardOIDs == 0 )

 {

 AddStandardOIDS();

 AddStandardTypes();

 }

 }

 ModuleName DEFINITIONS CCE

 BGIN

 Exports

 Imports

 Statements

 END

 {

 /*

 * See if we have any syntax errors. If so, we abort

 * the operation to update the database files. Other-

 * wise we continue on.

 */

 if( 0 == yyErrorCnt )

 {

 /*

 * Resolve the OID numbers; any unresolved OID

 * assignments will abort the operation. We do

 * this only if we have found unresolved refs

 * in the course of the operation.

 */

 if( debug ) DumpAllOIDs();

 if( bResolveOIDsNeeded && !AllOIDsResolved() )

 {

 /*

 * Too bad, we have things that couldn't

 * be resolved. So exit now.

 */

 exit(1);

 }



 /*

 * Do any final object type output.

 */

 FinishObjectProcessing();

 }

 }

 ;

ModuleName

 : ID OptObjectId

 {

 $$ = newListElement(NAMEANDOID,$1,0);

 $$->OID = (struct list *)$2;

 }

 ;



Imports

 : IMPORTS ImportFromModuleList SEMICOLON

 | IMPORTS SEMICOLON

 | empty

 ;

ImportFromModuleList

 : ImportFromModuleList ImportFromModule

 | ImportFromModule

 ;

ImportFromModule

 : ImportList FROM ID

 ;

ImportList

 : ImportItem

 | ImportList COMMA ImportItem

 {

 yyerrok;

 }

 | error

 | ImportList error

 | ImportList error ImportItem

 {

 yyerrok;

 }

 | ImportList COMMA error

 ;

ImportItem

 : NAME

 {

 }

 | ID

 {

 }

 | COUNTER

 {

 }

 | GAUGE

 {

 }

 | IPADDRESS

 {

 }

 | NETWORKADDRESS

 {

 }

 | OBJECTTYPE

 {

 }

 | TIMETICKS

 {

 }

 | TRAPTYPE

 {

 }

 | ERROR

 {

 fprintf(stderr,"Line %d : Invalid TYPE \"%s\" -- should be \"%s\"\n",yylineno,yytext,ShouldBeKeyword(yytext));

 }

 ;



Exports

 : EXPORTS

 {

 /*

 * Indicate we don't want keywords at this point.

 */

 eNoKeyWords = NOKEYWORDS;

 }

 ExportList

 {

 /*

 * At this point turn keyword searching on again.

 */

 eNoKeyWords = ALLKEYWORDS;

 }

 SEMICOLON

 | EXPORTS SEMICOLON

 | empty

 ;

ExportList

 : ExportItem

 | ExportList COMMA ExportItem

 {

 yyerrok;

 }

 | error

 | ExportList error

 | ExportList error ExportItem

 {

 yyerrok;

 }

 | ExportList COMMA error

 ;

ExportItem

 : ID

 | NAME

 ;



Statements

 :

 | Statements Statement

 {

 yyerrok;

 }

 | Statements error

 ;

Statement

 : ObjectIdAssignment

 | ObjectType

 | TrapType

 | TypeDefinition

 ;



$MARK :

 {

 /*

 * This is performed only to get save the correct

 * line number for an ASN.1 definition.

 */

 nLineNoOfDef = yylineno;

 }

 ;



ObjectIdAssignment

 : NAME OBJECT $MARK IDENTIFIER CCE ObjectId

 {

 /*

 * This assigns a name to an object id; we store it

 * in our database.

 */

 ProcessOID($1,$6,nLineNoOfDef);

 }

 ;



ObjectType

 : NAME OBJECTTYPE $MARK

 SyntaxLine

 AccessLine

 StatusLine

 DescriptionLine

 ReferenceLine

 Index

 Default

 CCE ObjectId

 {

 /*

 * Process the object definition.

 */

 ProcessObject($1,$4,$5,$6,$7,$8,$9,$10,$12,nLineNoOfDef,1);



 }

 ;



TrapType

 : NAME TRAPTYPE $MARK

 ENTERPRISE ObjectId

 Variables

 DescriptionLine

 ReferenceLine

 CCE LITNUMBER

 {

 /*

 * If we are doing stuff with traps, emit all the

 * things necessary now.

 */

 ProcessTrap($1,$5,$6,$7,$8,$10,nLineNoOfDef);

 }

 ;



TypeDefinition

 : ID CCE $MARK Type

 {

 /*

 * Process any type definition stuff.

 */

 ProcessType($1,$4,nLineNoOfDef);

 }

 ;



Type

 : BuiltInType

 {

 $$ = $1;

 }

 | DefinedType

 {

 $$ = $1;

 }

 ;



DefinedType

 : ID

 {

 /*

 * Notice that we don't worry about checking to see if

 * 'ID' is really defined now. We will check it out

 * later on. This will allow a type to be used before

 * it is defined; it means a little more work later on.

 */

 $$ = newTypeElement(TYPENAME,$1);

 }

 | ERROR

 {

 fprintf(stderr,"Line %d : Invalid TYPE \"%s\" -- should be \"%s\"\n",yylineno,yytext,ShouldBeKeyword(yytext));

 $$ = (TYPE *)0;

 }

 ;



BuiltInType

 : IntegerType

 {

 $$ = $1;

 }

 | OctetType

 {

 $$ = $1;

 }

 | NullType

 {

 $$ = $1;

 }

 | SequenceType

 {

 $$ = $1;

 }

 | SequenceOfType

 {

 $$ = $1;

 }

 | ObjectIdType

 {

 $$ = $1;

 }

 | CounterType

 {

 $$ = $1;

 }

 | TimeTickType

 {

 $$ = $1;

 }

 | GaugeType

 {

 $$ = $1;

 }

 | IpAddressType

 {

 $$ = $1;

 }

 | NetworkAddressType

 {

 $$ = $1;

 }

 ;



NetworkAddressType

 : NETWORKADDRESS

 {

 $$ = newTypeElement(NETWORKTYPE,(char *)0);

 }

 ;



IpAddressType

 : IPADDRESS

 {

 $$ = newTypeElement(IPADDRTYPE,(char *)0);

 }

 ;



GaugeType

 : GAUGE

 {

 $$ = newTypeElement(GAUGETYPE,(char *)0);

 }

 ;



TimeTickType

 : TIMETICKS

 {

 $$ = newTypeElement(TICKTYPE,(char *)0);

 }

 ;



CounterType

 : COUNTER

 {

 $$ = newTypeElement(COUNTERTYPE,(char *)0);

 }

 ;



OctetType

 : OCTET STRING OctetSize

 {

 $$ = newTypeElement(OCTETTYPE,(char *)&($3));

 }

 | OCTSTR OctetSize

 {

 $$ = newTypeElement(OCTETTYPE,(char *)&($2));

 }

 ;



IntegerType

 : INTEGER

 {

 $$ = newTypeElement(INTEGERTYPE,(char *)0);

 }

 | INTEGER LBRACE NamedNumberList RBRACE

 {

 $$ = newTypeElement(ENUMINTEGER,(char *)$3);

 }

 | INTEGER LPAREN RangeOfValues RPAREN

 {

 $$ = newTypeElement(RANGEINTEGER,(char *)&($3));

 }

 ;



ObjectIdType

 : OBJECT IDENTIFIER

 {

 $$ = newTypeElement(OBJIDTYPE,(char *)0);

 }

 ;



NullType

 : NULLID

 {

 $$ = newTypeElement(NULLTYPE,(char *)0);

 }

 ;



SequenceOfType

 : SEQUENCE OF Type

 {

 $$ = newTypeElement(SEQOFTYPE,(char *)$3);

 }

 ;



SequenceType

 : SEQUENCE LBRACE SeqList RBRACE

 {

 $$ = newTypeElement(SEQTYPE,(char *)$3);

 }

 ;

SeqList

 : OptionalTypeList

 {

 $$ = $1;

 }

 | empty

 {

 $$ = 0;

 }

 ;

OptionalTypeList

 : OptionalType

 {

 $$ = $1;

 }

 | OptionalTypeList COMMA OptionalType

 {

 yyerrok;

 $$ = LinkToType($1,$3);

 }

 | error

 {

 $$ = 0;

 }

 | OptionalTypeList error

 {

 $$ = $1;

 }

 | OptionalTypeList error OptionalType

 {

 yyerrok;

 $$ = LinkToType($1,$3);

 }

 | OptionalTypeList COMMA error

 {

 $$ = $1;

 }

 ;

OptionalType

 : NAME Type

 {

 $$ = newTypeElement(ITEMTYPE,(char *)0);

 $$->Data.Names.ItemName = $1;

 $$->Data.Names.ItemType = $2;

 }

 ;



OctetSize

 : LPAREN SIZE LPAREN RangeOfValues RPAREN RPAREN

 {

 $$ = $4;

 }

 | empty

 {

 memset(&($$),0,sizeof(PAIR));

 }

 ;

RangeOfValues

 : LITNUMBER DOTDOT LITNUMBER

 {

 /*

 * Make sure that the first integer is less than the

 * second.

 */

 if( $1 > $3 )

 {

 fprintf(stderr,"Line %d : Range of %d..%d is incorrect\n",yylineno,$1,$3);

 $$.Lower = $3;

 $$.Upper = $1;

 }

 else {

 $$.Lower = $1;

 $$.Upper = $3;

 }

 }

 | LITNUMBER

 {

 $$.Lower = $$.Upper = $1;

 }

 ;



NamedNumberList

 : NamedNumber

 {

 $$ = $1;

 }

 | NamedNumberList COMMA NamedNumber

 {

 $$ = LinkToList($1,$3);

 yyerrok;

 }

 | error

 {

 $$ = 0;

 }

 | NamedNumberList error

 {

 $$ = $1;

 }

 | NamedNumberList error NamedNumber

 {

 $$ = LinkToList($1,$3);

 yyerrok;

 }

 | NamedNumberList COMMA error

 {

 $$ = $1;

 }

 ;

NamedNumber

 : NAME LPAREN LITNUMBER RPAREN

 {

 /*

 * Allocate a new element for the list.

 */

 $$ = newListElement(NAMEANDNUMBER,$1,$3);

 }

 | NAME LPAREN error RPAREN

 {

 /*

 * Invalid thing in the middle.

 */

 fprintf(stderr,"Line %d : Incorrect NAME - NUMBER pair for name \"%s\"\n",yylineno,$1);

 $$ = newListElement(NAMEANDNUMBER,(char *)0,0);

 yyerrok;

 }

 ;







Variables

 : VARIABLES LBRACE VarList RBRACE

 {

 $$ = $3;

 }

 | empty

 {

 $$ = 0;

 }

 ;

VarList

 : NAME

 {

 $$ = newListElement(NAMEONLY,$1,0);

 }

 | VarList COMMA NAME

 {

 $$ = LinkToList($1,newListElement(NAMEONLY,$3,0));

 yyerrok;

 }

 | error

 {

 $$ = 0;

 }

 | VarList error

 {

 $$ = $1;

 }

 | VarList error NAME

 {

 $$ = LinkToList($1,newListElement(NAMEONLY,$3,0));

 yyerrok;

 }

 | VarList COMMA error

 {

 $$ = $1;

 }

 ;



StatusLine

 : STATUS Status

 {

 $$ = $2;

 yyerrok;

 }

 | STATUS error

 {

 $$ = UNKNOWN;

 }

 | error

 {

 $$ = UNKNOWN;

 }

 ;



Status

 : MANDATORYOBJECT

 {

 $$ = (int) MANDATORY;

 }

 | OPTIONALOBJECT

 {

 $$ = (int) OPTIONAL;

 }

 | OBSOLETEOBJECT

 {

 $$ = (int) OBSOLETE;

 }

 | DEPRECATEDOBJECT

 {

 $$ = (int) DEPRECATED;

 }

 | error

 {

 $$ = (int) UNKNOWN;

 }

 ;



AccessLine

 : ACCESS Access

 {

 $$ = $2;

 yyerrok;

 }

 | ACCESS error

 {

 $$ = (int) UNKNOWN;

 }

 | error

 {

 $$ = (int) UNKNOWN;

 }

 ;

Access

 : READONLYOBJECT

 {

 $$ = (int) RONLY;

 }

 | READWRITEOBJECT

 {

 $$ = (int) RWRITE;

 }

 | WRITEONLYOBJECT

 {

 $$ = (int) WRITEONLY;

 }

 | NOTACCESSIBLEOBJECT

 {

 $$ = (int) NOACCESS;

 }

 ;



SyntaxLine

 : SYNTAX SyntaxType

 {

 yyerrok;

 $$ = $2;

 }

 | SYNTAX error

 {

 $$ = 0;

 }

 ;

SyntaxType

 : IntegerType

 | OctetType

 | NullType

 | ObjectIdType

 | CounterType

 | TimeTickType

 | GaugeType

 | IpAddressType

 | NetworkAddressType

 | SEQUENCE OF ID

 {

 $$ = newTypeElement(SEQOFTYPE,

 (char *)newTypeElement(TYPENAME,$3));

 }

 | ID

 {

 $$ = newTypeElement(TYPENAME,$1);

 }

 | ERROR

 {

 fprintf(stderr,"Line %d : Invalid TYPE \"%s\" -- should be \"%s\"\n",yylineno,yytext,ShouldBeKeyword(yytext));

 $$ = (TYPE *)0;

 }

 ;



DescriptionLine

 : DESCRIPTION LITSTRING

 {

 $$ = $2;

 }

 | DESCRIPTION error

 {

 fprintf(stderr,"Line %d : Invalid DESCRIPTION statement\n",yylineno);

 yyclearin;

 yyerrok;

 $$ = 0;

 }

 | empty

 {

 $$ = 0;

 }

 ;



ReferenceLine

 : REFERENCE LITSTRING

 {

 $$ = $2;

 }

 | REFERENCE error

 {

 fprintf(stderr,"Line %d : Invalid REFERENCE statement\n",yylineno);

 yyclearin;

 yyerrok;

 $$ = 0;

 }

 | empty

 {

 $$ = 0;

 }

 ;



Index

 : INDEX LBRACE IndexList RBRACE

 {

 $$ = $3;

 }

 | empty

 {

 $$ = 0;

 }

 ;

IndexList

 : IndexElement

 {

 $$ = $1;

 }

 | IndexList COMMA IndexElement

 {

 $$ = LinkToList($1,$3);

 yyerrok;

 }

 | error

 {

 $$ = 0;

 }

 | IndexList error

 {

 $$ = $1;

 }

 | IndexList error IndexElement

 {

 $$ = LinkToList($1,$3);

 yyerrok;

 }

 | IndexList COMMA error

 {

 $$ = $1;

 }

 ;

IndexElement

 : NAME

 {

 $$ = newListElement(NAMEONLY,$1,0);

 }

 | INTEGER

 {

 $$ = newListElement(NAMEONLY,"INTEGER",0);

 }

 | GAUGE

 {

 $$ = newListElement(NAMEONLY,"GAUGE",0);

 }

 | COUNTER

 {

 $$ = newListElement(NAMEONLY,"COUNTER",0);

 }

 | TIMETICKS

 {

 $$ = newListElement(NAMEONLY,"TIMETICKS",0);

 }

 | NETWORKADDRESS

 {

 $$ = newListElement(NAMEONLY,"NETWORKADDRESS",0);

 }

 | IPADDRESS

 {

 $$ = newListElement(NAMEONLY,"IPADDRESS",0);

 }

 | OBJECT IDENTIFIER

 {

 $$ = newListElement(NAMEONLY,"OID",0);

 }

 | OCTET STRING

 {

 $$ = newListElement(NAMEONLY,"STRING",0);

 }

 ;



Default

 : DEFVAL LBRACE Data RBRACE

 {

 $$ = $3;

 }

 | DEFVAL error RBRACE

 {

 fprintf(stderr,"Line %d : Invalid DEFVAL statement; expected an integer or string type\n",yylineno);

 yyclearin;

 yyerrok;

 $$ = 0;

 }

 | empty

 {

 $$ = 0;

 }

 ;

Data

 : LITNUMBER

 {

 $$ = newListElement(NUMBERONLY,(char *)0,$1);

 }

 | LITSTRING

 {

 $$ = newListElement(STRINGONLY,$1,0);

 }

 | NAME

 {

 $$ = newListElement(NAMEONLY,$1,0);

 }

 ;



OptObjectId

 : ObjectId

 {

 $$ = $1;

 }

 | empty

 {

 $$ = 0;

 }

 ;

ObjectId

 : NAME

 {

 $$ = newListElement(NAMEONLY,$1,0);

 }

 | LBRACE OIDList RBRACE

 {

 $$ = $2;

 }

 ;

OIDList

 : OIDSubId

 {

 $$ = $1;

 }

 | OIDList OIDSubId

 {

 $$ = LinkToList($1,$2);

 yyerrok;

 }

 | OIDList error

 {

 $$ = $1;

 }

 ;

OIDSubId

 : LITNUMBER

 {

 $$ = newListElement(NUMBERONLY,(char *)0,$1);

 }

 | NAME

 {

 $$ = newListElement(NAMEONLY,$1,0);

 }

 | NAME LPAREN LITNUMBER RPAREN

 {

 $$ = newListElement(NAMEANDNUMBER,$1,$3);

 }

 ;



empty

 :

 ;

%%



