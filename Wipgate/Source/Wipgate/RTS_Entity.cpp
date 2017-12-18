// Fill out your copyright notice in the Description page of Project Settings.

#include "RTS_Entity.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "UnitEffect.h"
#include "AbilityIcon.h"
#include "UI_Bar.h"
#include "RTS_Entity.h"
#include "RTS_GameState.h"
#include "RTS_PlayerController.h"

DEFINE_LOG_CATEGORY(RTS_ENTITY_LOG);

// Sets default values
ARTS_Entity::ARTS_Entity()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Selection component
	{
		SelectionStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionEffect"));
		SelectionStaticMeshComponent->SetupAttachment(RootComponent);
		SelectionStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SelectionStaticMeshComponent->SetCanEverAffectNavigation(false);
		SelectionStaticMeshComponent->SetReceivesDecals(false);
	}

	// UI components
	{
		BarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Bars"));
		BarWidget->SetupAttachment(RootComponent);
		BarWidget->SetReceivesDecals(false);
		BarWidget->SetCanEverAffectNavigation(false);

		MinimapIcon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Minimap Icon"));
		MinimapIcon->SetupAttachment(RootComponent);
		MinimapIcon->SetReceivesDecals(false);
		MinimapIcon->SetCanEverAffectNavigation(false);
		MinimapIcon->SetCastShadow(false);
		MinimapIcon->CastShadow = 1;
	}

	// Vision debugging meshes
	{
		RangeInnerVisionColor = FLinearColor(0.0f, 0.9f, 0.0f, 1.0f);
		RangeAttackColor = FLinearColor(0.9f, 0.0f, 0.0f, 1.0f);
		RangeOuterVisionColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);

		UStaticMeshComponent* innerVision = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerVisionRange"));
		innerVision->SetupAttachment(RootComponent);
		innerVision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		innerVision->SetReceivesDecals(false);
		innerVision->SetVisibility(false);
		DebugMeshes.Push(innerVision);

		UStaticMeshComponent* attackRange = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttackRange"));
		attackRange->SetupAttachment(RootComponent);
		attackRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		attackRange->SetReceivesDecals(false);
		attackRange->SetVisibility(false);
		DebugMeshes.Push(attackRange);

		UStaticMeshComponent* outerVision = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterVisionRange"));
		outerVision->SetupAttachment(RootComponent);
		outerVision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		outerVision->SetReceivesDecals(false);
		outerVision->SetVisibility(false);
		DebugMeshes.Push(outerVision);

		ConstructorHelpers::FObjectFinder<UStaticMesh> cylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		if (cylinderMesh.Succeeded())
		{
			innerVision->SetStaticMesh(cylinderMesh.Object);
			attackRange->SetStaticMesh(cylinderMesh.Object);
			outerVision->SetStaticMesh(cylinderMesh.Object);
		}
		else
		{
			UE_LOG(RTS_ENTITY_LOG, Error, TEXT("Failed to find mesh at \"/Engine/BasicShapes/Cylinder.Cylinder\""));
		}
	}

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

	if (BarWidget)
	{
		UUserWidget* barUserWidget = BarWidget->GetUserWidgetObject();
		if (barUserWidget)
		{
			UUI_Bar* bar = Cast<UUI_Bar>(barUserWidget);
			if (bar)
			{
				bar->Initialize(this);
			}
			else
			{
				UE_LOG(RTS_ENTITY_LOG, Error, TEXT("Entity's bar's type is not derived from UI_Bar!"));
			}
		}
		else
		{
			UE_LOG(RTS_ENTITY_LOG, Error, TEXT("Entity's bar's user widget object was not set!"));
		}
	}

	CurrentMovementStats = BaseMovementStats;
	CurrentAttackStats = BaseAttackStats;
	CurrentDefenceStats = BaseDefenceStats;
	CurrentVisionStats = BaseVisionStats;

	//TODO: Remove hardcoding
	MinimapIcon->SetRelativeLocation(FVector(0, 0, 5000));

	SetSelected(false);

	for (auto debugMesh : DebugMeshes)
	{
		debugMesh->SetVisibility(ShowRange);
	}

	if (ShowRange)
	{
		SetRangeDebug();
	}

	// Bars
	APawn* playerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (playerPawn)
	{
		TArray<UCameraComponent*> cameracomponents;
		playerPawn->GetComponents(cameracomponents);
		if (cameracomponents.Num() > 0)
		{
			UCameraComponent* camera = cameracomponents[0];
			FRotator rot = camera->GetComponentRotation();

			BarRotation.Roll = rot.Roll;
			BarRotation.Pitch = rot.Pitch + 90;
			BarRotation.Yaw = rot.Yaw + 180;
		}
		else
		{
			UE_LOG(RTS_ENTITY_LOG, Error, TEXT("Camera pawn doesn't contain a camera component!"));
		}
	}

	if (BarWidget)
	{
		BarWidget->SetWorldRotation(BarRotation);
	}

	SetTeamMaterial();
}

void ARTS_Entity::SetSelected(bool selected)
{
	Selected = selected;
	SelectionStaticMeshComponent->SetVisibility(selected, true);
}

void ARTS_Entity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Rate of fire */
	if (TimerRateOfFire > 0)
	{
		TimerRateOfFire -= DeltaTime;
	}

	/* Update movement stats */
	UCharacterMovementComponent* movement = GetCharacterMovement(); 
	movement->MaxWalkSpeed = CurrentMovementStats.Speed;

	/* Apply effects */
	for (auto e : UnitEffects)
	{
		if (!e) continue;

		e->Elapsed += DeltaTime;

		// only apply effect after delay
		if (e->Elapsed < 0)
		{
			continue;
		}

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
		{
			e->IsFinished = true;
		}
	}

	/* Clean up effects */
	for (size_t i = UnitEffects.Num() - 1; i < UnitEffects.Num(); i--)
	{
		if (UnitEffects[i]->IsFinished)
		{
			RemoveUnitEffect(UnitEffects[i]);
		}
	}
}

bool ARTS_Entity::IsSelected() const
{
	return Selected;
}

void ARTS_Entity::SetTeamMaterial()
{
	FLinearColor selectionColorHSV = Team.Color.LinearRGBToHSV();
	selectionColorHSV.B = SelectionBrightness;
	FLinearColor selectionColorRGB = selectionColorHSV.HSVToLinearRGB();
	if (SelectionStaticMeshComponent->GetMaterials().Num() > 0)
	{
		UMaterialInstanceDynamic* selectionMatInst = SelectionStaticMeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, SelectionStaticMeshComponent->GetMaterial(0));
		selectionMatInst->SetVectorParameterValue("EmissiveColor", selectionColorRGB);
		SelectionStaticMeshComponent->SetVisibility(false);
	}
	else
	{
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("No selection mesh material found!"));
	}

	//Set minimap icon color
	if (MinimapIcon)
	{
		if (MinimapIcon->GetMaterials().Num() > 0)
		{
			UMaterialInstanceDynamic* mMaterial = MinimapIcon->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MinimapIcon->GetMaterial(0));
			mMaterial->SetVectorParameterValue(MinimapColorParameterName, Team.Color);
		}
		else
		{
			UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("Entity's minimap mesh's material is not set!"));
		}
	}
	else
	{
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("Entitiy's minimap mesh icon was not created!"));
	}
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
	{
		UGameplayStatics::SpawnEmitterAttached(effect->StartParticles, RootComponent);
	}

	if (effect->ConstantParticles)
	{
		//if (effect->SocketName != "None")
		effect->StartParticleConstant(RootComponent);
		effect->AttachParticleToSocket(GetMesh());
	}
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
		else if (effect->Ticks > 0)
			CurrentDefenceStats.Armor -= effect->Magnitude;
		break;

	case EUnitEffectStat::DAMAGE:
		if (effect->Type == EUnitEffectType::OVER_TIME)
			CurrentAttackStats.Damage -= (effect->Magnitude / effect->Duration) * effect->Ticks;
		else if(effect->Ticks > 0)
			CurrentAttackStats.Damage -= effect->Magnitude;
		break;

	case EUnitEffectStat::MOVEMENT_SPEED:
		if (effect->Type == EUnitEffectType::OVER_TIME)
			CurrentMovementStats.Speed -= (effect->Magnitude / effect->Duration) * effect->Ticks;
		else if(effect->Ticks > 0)
			CurrentMovementStats.Speed -= effect->Magnitude;
		break;

	default:
		break;
	}

	UnitEffects.Remove(effect);

	// stop particle systems
	if (effect->EndParticles)
	{
		UGameplayStatics::SpawnEmitterAttached(effect->EndParticles, RootComponent);
	}

	if (effect->ConstantParticles)
	{
		effect->StopParticleConstant();
	}
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

void ARTS_Entity::SetRangeDebug()
{
	if (DebugMeshes.Num() < 3)
	{
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("ARTS_Entity::SetRangeDebug > Not all debug meshes were properly initialized!"));
		return;
	}

	UStaticMeshComponent* innerVision = DebugMeshes[0];
	UStaticMeshComponent* attackRange = DebugMeshes[1];
	UStaticMeshComponent* outerVision = DebugMeshes[2];

	if (RangeMesh)
	{
		innerVision->SetStaticMesh(RangeMesh);
		attackRange->SetStaticMesh(RangeMesh);
		outerVision->SetStaticMesh(RangeMesh);

		FVector size;

		size.X = CurrentVisionStats.InnerRange / 50.f;
		size.Y = size.X;
		size.Z = RangeHeight;
		innerVision->SetWorldScale3D(size);

		size.X = CurrentAttackStats.Range / 50.f;
		size.Y = size.X;
		attackRange->SetWorldScale3D(size);

		size.X = CurrentVisionStats.OuterRange / 50.f;
		size.Y = size.X;
		outerVision->SetWorldScale3D(size);
	}
	else
	{
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("ARTS_Entity::SetRangeDebug > No valid mesh found!"));
	}

	if (RangeMaterial)
	{
		UMaterialInstanceDynamic* iMaterial = innerVision->CreateAndSetMaterialInstanceDynamicFromMaterial(0, RangeMaterial);
		iMaterial->SetVectorParameterValue(RangeColorParameterName, RangeInnerVisionColor);
		UMaterialInstanceDynamic* aMaterial = attackRange->CreateAndSetMaterialInstanceDynamicFromMaterial(0, RangeMaterial);
		aMaterial->SetVectorParameterValue(RangeColorParameterName, RangeAttackColor);
		UMaterialInstanceDynamic* oMaterial = outerVision->CreateAndSetMaterialInstanceDynamicFromMaterial(0, RangeMaterial);
		oMaterial->SetVectorParameterValue(RangeColorParameterName, RangeOuterVisionColor);
	}
	else
	{
		UE_LOG(RTS_ENTITY_LOG, Warning, TEXT("ARTS_Entity::SetRangeDebug > No valid material found!"));
	}
}

bool ARTS_Entity::ApplyDamage(int damage, bool armor)
{
	//Apply the damage
	if (armor)
	{
		int d = damage;
		d -= CurrentDefenceStats.Armor;
		CurrentDefenceStats.Health -= FMath::Clamp(d, 1, damage);
	}
	else
	{
		CurrentDefenceStats.Health -= FMath::Clamp(damage, 1, damage);
	}

	//Check if character is dead
	if (CurrentDefenceStats.Health <= 0)
	{
		CurrentDefenceStats.Health = 0;
		Kill();
		return true;
	}
	return false;
}

void ARTS_Entity::ApplyHealing(int healing)
{
	CurrentDefenceStats.Health += healing;
	if (CurrentDefenceStats.Health > BaseDefenceStats.Health)
	{
		CurrentDefenceStats.Health = BaseDefenceStats.Health;
	}
}

void ARTS_Entity::Kill()
{
	SetSelected(false);

	UCapsuleComponent* capsule = GetCapsuleComponent();
	if (capsule)
	{
		capsule->DestroyComponent();
		//capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//SetActorTickEnabled(false);
		//DisableDebug();
	}

	if (Controller)
	{
		Controller->SetActorTickEnabled(false);
	}

	if (BarWidget)
	{
		BarWidget->DestroyComponent();
	}

	UWorld* world = GetWorld();
	if (world)
	{
		AGameStateBase* baseGameState = world->GetGameState();
		ARTS_GameState* castedGameState = Cast<ARTS_GameState>(baseGameState);
		if (baseGameState && castedGameState)
		{
			castedGameState->SelectedEntities.Remove(this);
			castedGameState->Entities.Remove(this);

			APlayerController* playerController = UGameplayStatics::GetPlayerController(world, 0);
			ARTS_PlayerController* rtsPlayerController = Cast<ARTS_PlayerController>(playerController);

			if (playerController && rtsPlayerController)
			{
				rtsPlayerController->UpdateAbilityButtons();
				URTS_HUDBase* hud = rtsPlayerController->GetHUD();
				if (hud)
				{
					hud->UpdateSelectedEntities(castedGameState->SelectedEntities);
				}
			}
		}
	}
}


void ARTS_Entity::ApplyEffectLinear(UUnitEffect * effect)
{
	if (effect->Elapsed > EFFECT_INTERVAL)
	{
		// increment ticks each interval
		effect->Ticks++;
		if (effect->IsFinished)
		{
			return;
		}

		// only apply effect if it was not finished yet
		switch (effect->AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			CurrentDefenceStats.Armor += effect->Magnitude / (effect->Duration / EFFECT_INTERVAL);
			break;
		case EUnitEffectStat::DAMAGE:
			ApplyDamage(effect->Magnitude / (effect->Duration / EFFECT_INTERVAL), false);
			break;
		case EUnitEffectStat::HEALING:
			ApplyHealing(effect->Magnitude / (effect->Duration / EFFECT_INTERVAL));
			break;
		case EUnitEffectStat::MOVEMENT_SPEED:
			CurrentMovementStats.Speed += effect->Magnitude / (effect->Duration / EFFECT_INTERVAL);
			break;
		default:
			break;
		}

		// spawn particles and reset time
		if (effect->TickParticles)
		{
			UGameplayStatics::SpawnEmitterAttached(effect->TickParticles, RootComponent);
		}
		effect->Elapsed = 0;
	}
}

void ARTS_Entity::ApplyEffectOnce(UUnitEffect * effect)
{
	if (effect->Ticks == 0)
	{
		effect->Ticks++;
		if (effect->TickParticles)
		{
			UGameplayStatics::SpawnEmitterAttached(effect->TickParticles, RootComponent);
		}

		switch (effect->AffectedStat)
		{
		case EUnitEffectStat::ARMOR:
			CurrentDefenceStats.Armor += effect->Magnitude;
			break;

		case EUnitEffectStat::DAMAGE:
			ApplyDamage(effect->Magnitude, false);
			break;

		case EUnitEffectStat::HEALING:
			ApplyHealing(effect->Magnitude);
			break;

		case EUnitEffectStat::MOVEMENT_SPEED:
			CurrentMovementStats.Speed += effect->Magnitude;
			break;

		default:
			break;
		}
	}

	if (effect->Elapsed > effect->Duration)
	{
		effect->IsFinished = true;
	}
}

