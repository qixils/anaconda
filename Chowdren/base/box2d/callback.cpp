#include "common.h"

void Callback::Do(tagRDATA* rdPtr)
{

}

void BoundaryCallback::Do(tagRDATA* rdPtr)
{
	rdPtr->eventBody = bodyID;
	rdPtr->rRd->GenerateEvent(3);
	rdPtr->eventBody = -1;
}

void LostAttachmentCallback::Do(tagRDATA* rdPtr)
{
	rdPtr->eventBody = bodyID;
	rdPtr->rRd->GenerateEvent(71);
	rdPtr->eventBody = -1;
}


void SleepCallback::Do(tagRDATA* rdPtr)
{
	rdPtr->eventBody = bodyID;
	rdPtr->rRd->GenerateEvent(40);
	rdPtr->eventBody = -1;
}

void WakeCallback::Do(tagRDATA* rdPtr)
{
	rdPtr->eventBody = bodyID;
	rdPtr->rRd->GenerateEvent(41);
	rdPtr->eventBody = -1;
}


void JointDieCallback::Do(tagRDATA* rdPtr)
{
	rdPtr->eventJoint = jointID;
	rdPtr->rRd->GenerateEvent(4);
	rdPtr->eventJoint = -1;
}

void CollideCallback::Do(tagRDATA* rdPtr)
{
	rdPtr->collData = data;

	rdPtr->rRd->GenerateEvent(cndOffset);
	rdPtr->rRd->GenerateEvent(cndOffset+1);
	rdPtr->rRd->GenerateEvent(cndOffset+2);
	
	rdPtr->collData.body1 = data.body2;
	rdPtr->collData.body2 = data.body1;
	rdPtr->collData.shape1 = data.shape2;
	rdPtr->collData.shape2 = data.shape1;
	rdPtr->collData.type1 = data.type2;
	rdPtr->collData.type2 = data.type1;
	rdPtr->collData.angle = data.angle + 180;
	rdPtr->collData.impulse.x = data.impulse.x*-1;
	
	rdPtr->rRd->GenerateEvent(cndOffset);
	rdPtr->rRd->GenerateEvent(cndOffset+1);
	rdPtr->rRd->GenerateEvent(cndOffset+2);
}

void addCallback(Callback* c, tagRDATA* rdPtr)
{
	if(rdPtr->lastcall)
		rdPtr->lastcall->Next = c;
	else
		rdPtr->callbacks = c;
	rdPtr->lastcall = c;
}
