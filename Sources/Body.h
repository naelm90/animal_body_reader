#pragma once

#include "Sticker.h"

enum AnimalLeg { LEG_FRONT_LEFT, LEG_FRONT_RIGHT, LEG_BACK_LEFT, LEG_BACK_RIGHT, LEG_COUNT };

struct BodyPart
{
	BodyPart() { strcpy(name, ""); pSticker = NULL;  pObject = NULL; }
	bool IsVisible() const { return pObject != NULL;  }
	char name[20];
	Sticker *pSticker;
	Object *pObject;
};


struct Body
{
	BodyPart head;
	BodyPart neck;
	BodyPart back;
	BodyPart tail;
	BodyPart legs[LEG_COUNT];
};

