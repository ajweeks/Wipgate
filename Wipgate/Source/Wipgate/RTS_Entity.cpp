// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Entity.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DataTable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(RTS_ENTITY_LOG);

// Sets default values
ARTS_Entity::ARTS_Entity()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = root;

	SelectionStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionEffect"));
	SelectionStaticMeshComponent->SetupAttachment(RootComponent);

	TestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TestMesh"));
	TestMesh->SetupAttachment(RootComponent);
	FRotator rot;
	rot.Yaw = -90;
	TestMesh->SetRelativeRotation(rot);

	BarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Bars"));
	BarWidget->SetupAttachment(RootComponent);

	MinimapIcon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Minimap Icon"));
	MinimapIcon->SetupAttachment(RootComponent);

	AbilityIcons.SetNumZeroed(NUM_ABILITIES);

	//TODO: Update for outer and middle range
	UStaticMeshComponent* innerVision = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerVisionRange"));
	innerVision->SetupAttachment(RootComponent);
	DebugMeshes.Push(innerVision);
}

// Called when the game starts
void ARTS_Entity::BeginPlay()
{
	Super::BeginPlay();

	if (!TeamRow.IsNull())
	{
		FTeamRow tr;
		tr = *TeamRow.GetRow<FTeamRow>("ARTS_Entity::ARTS_Entity > Row not found!");
		Team.Name = TeamRow.RowName;
		Team.Color = tr.Color;
		Team.Alignment = tr.Alignment;
	}

	CurrentMovementStats = BaseMovementStats;
	CurrentAttackStats = BaseAttackStats;
	CurrentDefenceStats = BaseDefenceStats;
	CurrentVisionStats = BaseVisionStats;

	//TODO: No hardcoding
	MinimapIcon->SetRelativeLocation(FVector(0, 0, 5000));

	SetSelected(false);

	for (auto debugMesh : DebugMeshes)
	{
		debugMesh->SetVisibility(ShowRange);
	}

	if (ShowRange)
		CreateRangeDebug();
}

void ARTS_Entity::SetSelected(bool selected)
{
	Selected = selected;
	SelectionStaticMeshComponent->SetVisibility(selected, true);
}

void ARTS_Entity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Apply effects */
	for (auto e : UnitEffects)
	{
		e->Elapsed += DeltaTime;

		// only apply effect after delay
		if (e->Elapsed < 0)
			continue;

		switch (e->Type)
		{
		case EUnitEffectType::OVER_TIME:
			ApplyEffectLinear(e);
			break;
		case EUnitEffectType::INSTANT:
			ApplyEffectOnce(e);
			break;
		default:
			break;
		}

		if (e->Ticks >= e->Duration)
			e->IsFinished = true;
	}

	/* Clean up effects */
	for (size_t i = UnitEffects.Num() - 1; i < UnitEffects.Num(); i--)
	{
		if (UnitEffects[i]->IsFinished)
			RemoveUnitEffect(UnitEffects[i]);
	}
}

bool ARTS_Entity::IsSelected() const
{
	return Selected;
}

TArray<UUnitEffect*> ARTS_Entity::GetUnitEffects() const
{
	return UnitEffects;
}

void ARTS_Entity::AddUnitEffect(UUnitEffect * effect)
{
	UnitEffects.Add(effect);

	// start particle systems
	if (effect->StartParticles)
		UGameplayStatics::SpawnEmitterAttached(effect->StartParticles, RootComponent);

	if (effect->ConstantParticles)
		effect->StartParticleConstant(RootComponent);
}

void ARTS_Entity::RemoveUnitEffect(UUnitEffect * effect)
{
	if (!UnitEffects.Contains(effect))
		return;

	switch (effect->AffectedStat)
	{
	case EUnitEffectStat::ARMOR:
		if (effect->Type == EUnitEffectType::OVER_TIME)
			CurrentDefenceStats.Armor -= (effect->Magnitude / effect->Duration) * effect->Ticks;
		else
			CurrentDefenceStats.Armor -= effect->Magnitude;
		break;
	case EUnitEffectStat::MOVEMENT_SPEED:
		break;
	default:
		break;
	}
	UnitEffects.Remove(effect);

	// stop particle systems
	if (effect->EndParticles)
		UGameplayStatics::SpawnEmitterAttached(effect->EndParticles, RootComponent);

	if (effect->ConstantParticles)
		effect->StopParticleConstant();
}

void ARTS_Entity::DisableDebug()
{
	for (UStaticMeshComponent* debugMesh : DebugMeshes)
	{
		UActorComponent* actor = Cast<UActorComponent>(debugMesh);
		actor->DestroyComponent();
	}
	DebugMeshes.Empty();
}

//TODO: Update for outer and middle range, not hardcoding debug meshes
void ARTS_Entity::CreateRangeDebug()
{
	UStaticMeshComponent* innerVision = DebugMeshes[0];
	if (RangeMesh)
		innerVision->SetStaticMesh(RangeMesh);
	else
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("ARTS_Entity::CreateRangeDebug > No valid mesh found!"));

	if (RangeMaterial)
		innerVision->SetMaterial(0, RangeMaterial);
	else
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("ARTS_Entity::CreateRangeDebug > No valid material found!"));
}

void ARTS_Entity::ApplyEffectLinear(UUnitEffect * effect)
{
	if (effect->Elapsed > EFFECT_INTERVAL)
	{
		// increment ticks each interval
		effect->Ticks++;
		if (effect->IsFinished)
			return;

		// only apply effect if it was not finished yet
		switch (effect->AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			CurrentDefenceStats.Armor += effect->Magnitude / (effect->Duration / EFFECT_INTERVAL);
			break;
		case EUnitEffectStat::DAMAGE:
			//UnitCoreComponent->ApplyDamage_CPP(effect->Magnitude / (effect->Duration / EFFECT_INTERVAL), false);
			break;
		case EUnitEffectStat::HEALING:
			//UnitCoreComponent->ApplyHealing(effect->Magnitude / (effect->Duration / EFFECT_INTERVAL));
			break;
		case EUnitEffectStat::MOVEMENT_SPEED:
			break;
		default:
			break;
		}

		// spawn particles and reset time
		if (effect->TickParticles)
			UGameplayStatics::SpawnEmitterAttached(effect->TickParticles, RootComponent);
		effect->Elapsed = 0;
	}
}

void ARTS_Entity::ApplyEffectOnce(UUnitEffect * effect)
{
	if (effect->Ticks == 0)
	{
		effect->Ticks++;
		if (effect->TickParticles)
			UGameplayStatics::SpawnEmitterAttached(effect->TickParticles, RootComponent);

		switch (effect->AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			CurrentDefenceStats.Armor += effect->Magnitude;
			break;

		case EUnitEffectStat::DAMAGE:
			//UnitCoreComponent->ApplyDamage_CPP(effect->Magnitude, false);
			break;

		case EUnitEffectStat::HEALING:
			//UnitCoreComponent->ApplyHealing(effect->Magnitude);
			break;

		case EUnitEffectStat::MOVEMENT_SPEED:
			break;

		default:
			break;
		}
	}

	if (effect->Elapsed > effect->Duration)
		effect->IsFinished = true;
}

