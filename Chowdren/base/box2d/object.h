#ifndef CHOWDREN_BOX2D_OBJECT_H
#define CHOWDREN_BOX2D_OBJECT_H

#include "../frameobject.h"
#include "Box2D.h"
/*#include "callback.h"
#include "listener.h"
#include "userdata.h"*/

#define round(x) floor(x+0.5)

#define RAD_TO_DEG -57.295779513082320876798154814105f
#define DEG_TO_RAD -0.017453292519943295769236907684886f
#define M_PI2 6.283185307179586476925286766559f
#define MAX_OBJECTS 2000

bool AssertFail(const wchar_t* expression, const wchar_t* file, int line)
{
	char buffer[256];
	sprintf(buffer,"Assertion Failed at: %ws\n\nFile %ws\nLine %d\n\nPress OK to continue or Cancel to Halt the Program.",expression,file,line);
	std::cout << buffer << std::endl;
	return true;
}

bool AssertFail2(const char* message)
{
	char buffer[256];
	sprintf(buffer,"%s\n\nPress OK to continue or Cancel to Halt the Program.",message);
	std::cout << buffer << std::endl;
	return true;
}

void Debug(const char* str, ...)
{
	char buffer[256];
	if(!str) return;

	va_list ptr;
	va_start(ptr,str);
	vsnprintf(buffer,256,str,ptr);
	va_end(ptr);

	std::cout << buffer << std::endl;
}

void Debug(int d)
{
	Debug("%d",d);
}

void Debug(float d)
{
	Debug("%f",d);
}

/*struct RayResult
{
	void Init(int n)
	{
		if(shapes) delete [] shapes;
		shapes = new b2Shape*[n];
		numShapes = n;
		hits = 0;
		normal.SetZero();
		point.SetZero();
	}
	~RayResult()
	{
		if(shapes) delete [] shapes;
	}
	b2Shape** shapes;
	int numShapes;
	b2Vec2 point;
	b2Vec2 normal;
	rayUserData ud;
	int hits;
};*/

class Box2D : public FrameObject
{
public:
	int *AttachedObjectIDs;
	int maxBodies;
	int maxJoints;
	int maxBodyDefs;
	int maxShapeDefs;
	int maxJointDefs;
	int maxControllers;
/*	Callback* callbacks;
	Callback* lastcall;*/
/*	RayResult ray;*/
	int lastBody;
	int lastJoint;
	int lastController;
	b2Body** bodies;
	b2Joint** joints;
	b2BodyDef** bDefs;
	b2JointDef** jDefs;
	b2ShapeDef** sDefs;
	b2Controller** controllers;
/*	BoundaryListener* BL;
	ContactListener* CL;
	DestructionListener* DL;
	ContactFilter* CF;*/
	b2World* world;
	int eventBody;
	int eventJoint;
	int eventShape;
	int enumBody;
	int enumJoint;
	int enumShape;
	int enumController;
	// CollData collData;
	b2Vec2 gravity;
	b2Vec2 lastConv;
	b2AABB bounds;
	float scale;
	b2BodyDef* curBodyDef;
	b2JointDef* curJointDef;
	b2ShapeDef* curShapeDef;
	bool allowSleep;
	int posIterations;
	int velIterations;
	float timestep;
	bool WarmStart;
	bool PosCorrection;
	bool CCD;
	bool floatAngles;
	bool autoUpdate;
	b2Settings settings;
	char collReg[32][32];
	char collMode;
	char* tempdata;

    Box2D(const std::string & name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id)
	{
	}

	~Box2D()
	{
/*		for(int i = 0; i < maxBodyDefs; i++)
		{
			if(!bDefs[i]) continue;
			delete bDefs[i];
		}
		for(int i = 0; i < maxJointDefs; i++)
		{
			if(!jDefs[i]) continue;
			delete jDefs[i];
		}
		for(int i = 0; i < maxShapeDefs; i++)
		{
			if(!sDefs[i]) continue;
			delete sDefs[i];
		}

		if(world) delete world;
		if(bodies) delete [] bodies;
		if(joints) delete [] joints;
		if(bDefs) delete [] bDefs;
		if(jDefs) delete [] jDefs;
		if(sDefs) delete [] sDefs;
		if(controllers) delete [] controllers;
		if(AttachedObjectIDs) delete [] AttachedObjectIDs;
		if(BL) delete BL;
		if(CL) delete CL;
		if(DL) delete DL;
		if(CF) delete CF;
		if(DD) delete DD;
		if(tempdata) delete [] tempdata;

		if(callbacks)
		{
			while(callbacks)
			{
				callbacks->Do(rdPtr);
				Callback* c = callbacks->Next;
				delete callbacks;
				callbacks = c;
			}
			lastcall = NULL;
		}*/
	}

	void initialize_box2d()
	{
/*		tempdata = NULL;
		ray.ud.collType = -1;
		ray.ud.mask = -1;

		callbacks = NULL;
		lastcall = NULL;

		bodies = new b2Body*[maxBodies];
		joints = new b2Joint*[maxJoints];
		bDefs = new b2BodyDef*[maxBodyDefs];
		jDefs = new b2JointDef*[maxJointDefs];
		sDefs = new b2ShapeDef*[maxShapeDefs];
		controllers = new b2Controller*[maxControllers];

		memset(bodies, 0, maxBodies*4);
		memset(joints, 0, maxJoints*4);
		memset(bDefs, 0, maxBodyDefs*4);
		memset(jDefs, 0, maxJointDefs*4);
		memset(sDefs, 0, maxShapeDefs*4);
		memset(controllers, 0, maxControllers*4);
		
		AttachedObjectIDs = new int[MAX_OBJECTS];
		for(int i = 0; i < MAX_OBJECTS; i++) AttachedObjectIDs[i] = -1;

		BL = new BoundaryListener;
		CL = new ContactListener;
		DL = new DestructionListener;
		CF = new ContactFilter;
		BL->rdPtr = rdPtr;
		CL->rdPtr = rdPtr;
		DL->rdPtr = rdPtr;
		CF->rdPtr = rdPtr;
		DD->rdPtr = rdPtr;

		memset(collReg,0,sizeof(char)*16*16);
		
		eventBody = -2;
		eventJoint = -2;
		eventShape = -2;
		memset(&collData,0,sizeof(CollData));

		world = new b2World(bounds,gravity,allowSleep, &settings);
		world->SetBoundaryListener(BL);
		world->SetContactListener(CL);
		world->SetDestructionListener(DL);
		world->SetContactFilter(CF);

		world->GetGroundBody()->GetUserData()->ID = -2;

		curBodyDef = NULL;
		curJointDef = NULL;
		curShapeDef = NULL;

		lastBody = -2;
		lastJoint = -2;
		lastController = -2;
		enumBody = -2;
		enumJoint = -2;
		enumShape = -2;
		enumController = -2;*/
	}
};

#include "userdata.cpp"

#endif // CHOWDREN_BOX2D_OBJECT_H