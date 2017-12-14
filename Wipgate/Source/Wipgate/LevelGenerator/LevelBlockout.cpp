#include "LevelBlockout.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"

ALevelBlockout::ALevelBlockout()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ALevelBlockout::Initialize(LevelGrid * grid)
{
	FVector origin(-grid->GetWidth() / 2 * BLOCK_SIZE, -grid->GetHeight() / 2 * BLOCK_SIZE, 0);

	for (size_t col = 0; col < grid->GetWidth(); col++)
	{
		for (size_t row = 0; row < grid->GetHeight(); row++)
		{
			FVector pos(col * BLOCK_SIZE, row * BLOCK_SIZE, 0);
			UStaticMeshComponent * meshComp = NewObject<UStaticMeshComponent>(this);
			meshComp->SetStaticMesh(m_Block);
			meshComp->SetWorldLocation(origin + pos);

			m_BlockOutMap.Add((*grid)[col][row], meshComp);

			FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
			meshComp->AttachToComponent(RootComponent, rules);
			meshComp->RegisterComponentWithWorld(GetWorld());
		}
	}
}

void ALevelBlockout::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALevelBlockout::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

