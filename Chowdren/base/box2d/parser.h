#ifndef _parser_h_
#define _parser_h_

#include	"Include\Box2D.h"

namespace Parser
{
	enum types
	{
		type_invalid = 0,
		type_space = 1,
		type_char = 2,
		type_delim = 3,
		type_start = 4,
		type_end = 5,
		type_number = 6,
	};

	static char parseType[128] = 
	{   
	//  0 1 2 3 4 5 6 7 8 9 a b c d e f
/* 0 */	0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,
/* 1 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 2 */ 1,0,0,0,0,0,0,0,4,5,0,6,3,6,6,0,
/* 3 */ 6,6,6,6,6,6,6,6,6,6,3,0,4,3,5,0,
/* 4 */ 0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
/* 5 */ 2,2,2,2,2,2,2,2,2,2,2,4,0,5,0,2,
/* 6 */ 0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
/* 7 */ 2,2,2,2,2,2,2,2,2,2,2,4,0,5,0,0,
	};

	bool nextElement(char* &buf);
	bool compare(char* buf, char* cmp);

	bool parseVertices(char* &buf, b2Vec2* &dest, int &count, int minVerts);
	bool parseShape(char* &buf, b2ShapeDef* &def, LPRDATA rdPtr);
	bool parseBody(char* &buf, b2BodyDef* &def, LPRDATA rdPtr);

	bool parseShapeCircle(char* &buf, b2CircleDef* def, LPRDATA rdPtr);
	bool parseShapePoly(char* &buf, b2PolygonDef* def, LPRDATA rdPtr);
	bool parseShapeEdge(char* &buf, b2EdgeChainDef* def, LPRDATA rdPtr);
	bool parseShapeDefault(char* &buf, b2ShapeDef* def, LPRDATA rdPtr);
	bool parseBodySub(char* &buf, b2BodyDef* def, LPRDATA rdPtr);

	bool parseFloat(char* &buf, float &flt);
	bool parseInteger(char* &buf, int &val);
}

#endif