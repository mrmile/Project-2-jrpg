#include "App.h"
#include "Entity.h"
#include "EntityManager.h"
#include "ModuleCollisions.h"
#include "Collider.h"
#include "Render.h"
#include "Textures.h"
#include "ModulePlayer.h"
#include "ModulePhysics.h"
#include "GameManager.h"
#include "InventoryMenu.h"
#include "Audio.h"
#include "QuestManager.h"

Entity::Entity(int x,int y) : position(x,y)
{

}

Entity::~Entity()
{
	if (collider != nullptr)
		collider->pendingToDelete = true;
}
const Collider* Entity::GetColldier() const
{
	return collider;
}
void Entity::Update(float dt)
{
	if (currentAnim != nullptr)
	{
		currentAnim->Update();
	}

	if (collider != nullptr)
	{
		collider->SetPos(position.x, position.y);
	}

}
void Entity::Draw()
{
	if (currentAnim != nullptr)
	{
		app->render->DrawTexture(texture, position.x + offsetX, position.y + offsetY, &(currentAnim->GetCurrentFrame()));
	}
}

void Entity::SetToDelete()
{
	LOG("DELETING COLLIDER");

	if (collider != nullptr)
	{
		collider->pendingToDelete = true;
	}

}

void Entity::ReduceAP(int AP_used)
{
	EntityAP = EntityAP - AP_used;
}


void Entity::OnCollision(Collider* c2)
{

	if ((c2->type == Collider::Type::GRENADE_DAMAGE) && (EntityKillable == true))
	{
		EntityHP -=	20;
	}

	if ((c2->type == Collider::Type::PLAYER_ATTACK) && (EntityKillable == true))
	{
		EntityHP -= app->player->MeleeDamage;
	}
	
}
