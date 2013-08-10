#ifndef _userdata_h_
#define _userdata_h_

class Box2D;

struct Attachment
{
	Attachment();
	~Attachment();

	Box2D* obj;
	int objectNum;
	float rotOff;
	b2Vec2 offset;
	char rotation;
	char dest;
	Attachment* Next;
	Attachment* Prev;
};

struct bodyUserData
{
	bodyUserData();

	void AddObject(int id, b2Vec2 offset, int rot, int dest, float roff);
	void RemObject(int id);
	Attachment* GetAttachment(int id);
	void RemAttachment(Attachment* &a);
	void BodyDie();

	Attachment* attachment;

	Box2D* obj;
	int numJoints;
	int numShapes;
	int ID;
	bool customMass;
	bool sleepflag;
};

struct jointUserData
{
	jointUserData();

	int body1;
	int body2;
	int ID;
};

struct shapeUserData
{
	shapeUserData();

	Box2D* obj;
	char collType;
	int body;
	int ID;
};

struct rayUserData
{
	rayUserData();

	bool solidShapes;
	int collType;
	short mask;
};

#endif
