#ifndef _func_h_
#define _func_h_

struct tagRDATA;
typedef tagRDATA* LPRDATA;

#define oParam() (LPRO)Param(-1);
#define uParam() Param(-1);
#define lParam() Param(TYPE_INT);
inline float gfParam(LPRDATA rdPtr, int param1) {int f = CNC_GetFloatParameter(rdPtr); return *(float*)&f;}
#define fParam() gfParam(rdPtr,param1);
#define sParam() (LPCSTR)Param(TYPE_STRING);

#define xlParam() ExParam(TYPE_INT);
#define xsParam() (LPCSTR)ExParam(TYPE_STRING);
inline float xgfParam(LPRDATA rdPtr, int param1) {int f = ExParam(TYPE_FLOAT); return *(float*)&f;}
#define xfParam() xgfParam(rdPtr,param1);

bool hasAttachment(LPRO obj, LPRDATA rdPtr);
void removeAttachment(LPRO obj, LPRDATA rdPtr);

int getNullBody(LPRDATA rdPtr);
int getNullJoint(LPRDATA rdPtr);
int getNullController(LPRDATA rdPtr);

b2Body* getBody(int i, LPRDATA rdPtr);
b2Joint* getJoint(int i, LPRDATA rdPtr);
b2BodyDef* getBodyDef(int i, LPRDATA rdPtr);
b2JointDef* getJointDef(int i, LPRDATA rdPtr);
b2ShapeDef* getShapeDef(int i, LPRDATA rdPtr);
b2Controller* getController(int i, LPRDATA rdPtr);

bool isBody(int i, LPRDATA rdPtr);
bool isJoint(int i, LPRDATA rdPtr);
bool isBodyDef(int i, LPRDATA rdPtr);
bool isJointDef(int i, LPRDATA rdPtr);
bool isShapeDef(int i, LPRDATA rdPtr);
bool isController(int i, LPRDATA rdPtr);

b2Shape* getShape(b2Body* b, int n);
b2Joint* getJoint(b2Body* b, int n);
b2Controller* getController(b2Body* b, int n);
b2Body* getBody(b2Controller* c, int n);

bool setJointDefAnchor(b2JointDef* d, LPRDATA rdPtr);

void updateShapes(b2Body* b);

void copyDef(b2ShapeDef* src, b2ShapeDef* &dest);
void copyDef(b2BodyDef* src, b2BodyDef* &dest);
void copyDef(b2JointDef* src, b2JointDef* &dest);

static bool parseDelim[128] = 
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define delim parseDelim

float* parseString(LPCSTR string, int &num);

struct extendedParam
{
	union
	{
		b2Body* p1_body;
		b2Shape* p1_shape;
		long p1_long;
	};
	union
	{
		b2Body* p2_body;
		b2Shape* p2_shape;
		long p2_long;
	};
	union
	{
		b2Body* p3_body;
		b2Shape* p3_shape;
		long p3_long;
	};
	union
	{
		b2Body* p4_body;
		b2Shape* p4_shape;
		long p4_long;
	};
};

long ProcessCondition(tagRDATA* rdPtr, long param1, long param2, long (*myFunc)(tagRDATA*, LPHO, long));

#endif