#include "common.h"
#include "func.h"

void DestructionListener::SayGoodbye(b2Joint *joint)
{
	JointDieCallback* c = new JointDieCallback;
	
	c->jointID = joint->GetUserData()->ID;
	addCallback(c,rdPtr);

	b2Body* b = getBody(joint->GetUserData()->body1,rdPtr);
	b2Body* b2 = getBody(joint->GetUserData()->body2,rdPtr);

	if(b) b->GetUserData()->numJoints--;
	if(b2) b2->GetUserData()->numJoints--;

	rdPtr->joints[joint->GetUserData()->ID] = NULL;
}

void DestructionListener::SayGoodbye(b2Shape *shape)
{

}

void BoundaryListener::Violation(b2Body *body)
{
	BoundaryCallback* c = new BoundaryCallback;

	c->bodyID = body->GetUserData()->ID;
	addCallback(c,rdPtr);
}


bool ContactListener::ContactStart(b2Contact* contact)
{
	shapeUserData* u1 = contact->GetShape1()->GetUserData();
	shapeUserData* u2 = contact->GetShape2()->GetUserData();
	
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	rdPtr->collMode = 0;

	if(contact->GetShape1()->IsSensor() || contact->GetShape2()->IsSensor()) rdPtr->collMode = 1;

	if(cr == 1)
	{
		CollideCallback* c = new CollideCallback(12);
		
		c->data.body1 = u1->body;
		c->data.body2 = u2->body;
		c->data.collPoint.SetZero(); 
		c->data.shape1 = u1->ID;
		c->data.shape2 = u2->ID;
		c->data.type1 = u1->collType;
		c->data.type2 = u2->collType;

		addCallback(c,rdPtr);

		rdPtr->collData = c->data;
		rdPtr->rRd->GenerateEvent(18);
		rdPtr->rRd->GenerateEvent(19);
		rdPtr->rRd->GenerateEvent(20);

		rdPtr->collData.body1 = c->data.body2;
		rdPtr->collData.body2 = c->data.body1;
		rdPtr->collData.shape1 = c->data.shape2;
		rdPtr->collData.shape2 = c->data.shape1;
		rdPtr->collData.type1 = c->data.type2;
		rdPtr->collData.type2 = c->data.type1;
		
		rdPtr->rRd->GenerateEvent(19);
		rdPtr->rRd->GenerateEvent(20);
	}
	return rdPtr->collMode == 0;
}
void ContactListener::ContactStop(b2Contact* contact)
{
	shapeUserData* u1 = contact->GetShape1()->GetUserData();
	shapeUserData* u2 = contact->GetShape2()->GetUserData();
	
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	if(cr == 1)
	{
		CollideCallback* c = new CollideCallback(15);
		
		c->data.body1 = u1->body;
		c->data.body2 = u2->body;
		c->data.collPoint.SetZero(); 
		c->data.shape1 = u1->ID;
		c->data.shape2 = u2->ID;
		c->data.type1 = u1->collType;
		c->data.type2 = u2->collType;

		addCallback(c,rdPtr);
	}
}

void ContactListener::ContactPointAdd(b2Contact* contact, const b2ContactPoint* point)
{
	contact;

	shapeUserData* u1 = point->shape1->GetUserData();
	shapeUserData* u2 = point->shape2->GetUserData();
	
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	if(cr == 1)
	{
		CollideCallback* c = new CollideCallback(21);
		
		c->data.body1 = u1->body;
		c->data.body2 = u2->body;
		c->data.collPoint = point->position; 
		c->data.collPoint *= rdPtr->scale;
		c->data.shape1 = u1->ID;
		c->data.shape2 = u2->ID;
		c->data.type1 = u1->collType;
		c->data.type2 = u2->collType;
		c->data.friction = point->friction;
		c->data.restitution = point->restitution;
		c->data.separation = point->separation;
		c->data.velocity = point->velocity;

		addCallback(c,rdPtr);
	}
}
void ContactListener::ContactPointPersist(b2Contact* contact, const b2ContactPoint* point)
{
	contact;

	shapeUserData* u1 = point->shape1->GetUserData();
	shapeUserData* u2 = point->shape2->GetUserData();
	
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	if(cr == 1)
	{
		CollideCallback* c = new CollideCallback(24);
		
		c->data.body1 = u1->body;
		c->data.body2 = u2->body;
		c->data.collPoint = point->position; 
		c->data.collPoint *= rdPtr->scale;
		c->data.shape1 = u1->ID;
		c->data.shape2 = u2->ID;
		c->data.type1 = u1->collType;
		c->data.type2 = u2->collType;
		c->data.friction = point->friction;
		c->data.restitution = point->restitution;
		c->data.separation = point->separation;
		c->data.velocity = point->velocity;

		addCallback(c,rdPtr);
	}
}
void ContactListener::ContactPointRemove(b2Contact* contact, const b2ContactPoint* point)
{
	contact;

	shapeUserData* u1 = point->shape1->GetUserData();
	shapeUserData* u2 = point->shape2->GetUserData();
	
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	if(cr == 1)
	{
		CollideCallback* c = new CollideCallback(27);
		
		c->data.body1 = u1->body;
		c->data.body2 = u2->body;
		c->data.collPoint = point->position; 
		c->data.collPoint *= rdPtr->scale;
		c->data.shape1 = u1->ID;
		c->data.shape2 = u2->ID;
		c->data.type1 = u1->collType;
		c->data.type2 = u2->collType;
		c->data.friction = point->friction;
		c->data.restitution = point->restitution;
		c->data.separation = point->separation;
		c->data.velocity = point->velocity;

		addCallback(c,rdPtr);
	}
}
void ContactListener::ContactPointResponse(b2Contact* contact, const b2ContactResult* point)
{
	contact;
	shapeUserData* u1 = point->shape1->GetUserData();
	shapeUserData* u2 = point->shape2->GetUserData();
	
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	if(cr == 1)
	{
		CollideCallback* c = new CollideCallback(0);
		
		c->data.body1 = u1->body;
		c->data.body2 = u2->body;
		c->data.collPoint = point->position; 
		c->data.collPoint *= rdPtr->scale;
		c->data.shape1 = u1->ID;
		c->data.shape2 = u2->ID;
		c->data.type1 = u1->collType;
		c->data.type2 = u2->collType;
		c->data.impulse.Set(point->tangentImpulse,point->normalImpulse);
		c->data.angle = atan2(point->normal.y,point->normal.x) * RAD_TO_DEG;

		addCallback(c,rdPtr);
	}
}

bool ContactFilter::ShouldCollide(b2Shape* shape1, b2Shape* shape2)
{
	shapeUserData* u1 = shape1->GetUserData();
	shapeUserData* u2 = shape2->GetUserData();
	const b2FilterData& filter1 = shape1->GetFilterData();
	const b2FilterData& filter2 = shape2->GetFilterData();

	//Handle Group Index
	if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
	{
		return filter1.groupIndex > 0;
	}

	//Handle Mask/Category
	bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;

	//Handle Collision Reg
	char cr = rdPtr->collReg[u1->collType][u2->collType] | rdPtr->collReg[u2->collType][u1->collType];

	if(cr == -1) //No Collision
		collide = false;
	return collide;

}

bool ContactFilter::RayCollide(rayUserData* userData, b2Shape* shape)
{
	shapeUserData* us = (shapeUserData*)shape->GetUserData();
	if(userData->collType >= 0)
	{
		char cr = rdPtr->collReg[shape->GetUserData()->collType][userData->collType] | rdPtr->collReg[userData->collType][shape->GetUserData()->collType];
		if(cr <= 0)
			return false;
	}
	if((userData->mask & shape->GetFilterData().categoryBits) == 0)
		return false;
	return true;
}

/// Draw a closed polygon provided in CCW order.
void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
}

/// Draw a solid closed polygon provided in CCW order.
void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
}

/// Draw a circle.
void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
}

/// Draw a solid circle.
void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
}

/// Draw a line segment.
void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void DebugDraw::DrawXForm(const b2XForm& xf)
{
}