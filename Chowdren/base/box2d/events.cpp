ACTION(
	/* ID */			1,
	/* Name */			"Quick Create Body at (%0), offset (%1,%2), rotation (%3), destruction (%4)",
	/* Flags */			0,
	/* Params */		(5,PARAM_OBJECT,"Object",PARAM_NUMBER,"X Offset",PARAM_NUMBER,"Y Offset",PARAM_NUMBER,"Rotation (-1: Disable, 0: Not Object, 1: Fast, 2: Quality)",PARAM_NUMBER,"Link Destruction")
) 


{
	LPRO obj = oParam();
	float x = fParam();
	float y = fParam();
	int rotation = lParam();
	int dest = lParam();

	if(!obj) return;
	
	if(hasAttachment(obj,rdPtr)) removeAttachment(obj,rdPtr);

	int n = getNullBody(rdPtr);
	if(n == -1) return;

	b2BodyDef def;
	def.userData.ID = n;
	def.userData.rdPtr = rdPtr;
	def.userData.AddObject(obj->roHo.hoNumber,b2Vec2(-x/rdPtr->scale,-y/rdPtr->scale),rotation,dest,0);
	if(rdPtr->floatAngles)
		def.angle = obj->roc.rcAngleF * DEG_TO_RAD;
	else
		def.angle = obj->roc.rcAngleI * DEG_TO_RAD;
	def.position.Set(((float)obj->roHo.hoX + x)/rdPtr->scale, ((float)obj->roHo.hoY + y)/rdPtr->scale);
	def.fixedRotation = rotation == -1;
	

	if((rdPtr->bodies[n] = rdPtr->world->CreateBody(&def)) == NULL)
	{
		//Failed to create body
		def.userData.BodyDie();
		rdPtr->lastBody = -2;
		return;
	}

	rdPtr->lastBody = n;
}