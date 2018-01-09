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
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/CollisionProfile.h"
#include "UObject/ConstructorHelpers.h"

#include "UnitEffect.h"
#include "UI_Bar.h"
#include "RTS_Entity.h"
#include "RTS_GameState.h"
#include "RTS_PlayerController.h"
#include "RTS_Team.h"
#include "RTS_AIController.h"
#include "GeneralFunctionLibrary_CPP.h"

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
		BarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		MinimapIcon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Minimap Icon"));
		MinimapIcon->SetupAttachment(RootComponent);
		MinimapIcon->SetReceivesDecals(false);
		MinimapIcon->SetCanEverAffectNavigation(false);
		MinimapIcon->SetCastShadow(false);
		MinimapIcon->CastShadow = 1;
	}
}

// Called when the game starts
void ARTS_Entity::BeginPlay()
{
	Super::BeginPlay();
}

void ARTS_Entity::SetSelected(bool selected)
{
	Selected = selected;

	if (SelectionStaticMeshComponent)
	{
		SelectionStaticMeshComponent->SetVisibility(selected, true);
	}
}

void ARTS_Entity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Rate of fire */
	if (TimerRateOfFire > 0)
	{
		TimerRateOfFire -= DeltaTime;
		if (TimerRateOfFire <= 0)
		{
			m_IsAttackOnCooldown = false;
		}
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

FVector ARTS_Entity::GetGroundLocation()
{
	return GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

void ARTS_Entity::SetTeamMaterial()
{
	FLinearColor selectionColorHSV = Team->Color.LinearRGBToHSV();
	selectionColorHSV.B = SelectionBrightness;
	FLinearColor selectionColorRGB = selectionColorHSV.HSVToLinearRGB();

	if (SelectionStaticMeshComponent && SelectionStaticMeshComponent->GetMaterials().Num() > 0)
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
			mMaterial->SetVectorParameterValue(MinimapColorParameterName, Team->Color);
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

void ARTS_Entity::PostInitialize()
{
	if (BarWidget)
	{
		UUserWidget* barUserWidget = BarWidget->GetUserWidgetObject();
		if (barUserWidget)
		{
			UUI_Bar* bar = Cast<UUI_Bar>(barUserWidget);
			if (bar)
			{
				bar->InitializeFromOwner(this);
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
	Health = BaseDefenceStats.MaxHealth;
	CurrentVisionStats = BaseVisionStats;

	MinimapIcon->SetRelativeLocation(FVector(0, 0, 5000));

	SetSelected(false);

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

TArray<UUnitEffect*> ARTS_Entity::GetUnitEffects() const
{
	return UnitEffects;
}

bool ARTS_Entity::HasEffectWithTag(FName tag)
{
	for (auto e : UnitEffects)
	{
		if (e->Tag == tag)
			return true;
	}
	return false;
}

void ARTS_Entity::RemoveUnitEffectWithTag(FName tag)
{
	for (auto e : UnitEffects)
	{
		if (e->Tag == tag)
			RemoveUnitEffect(e);
	}
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

	case EUnitEffectStat::ATTACK_RATE:
		if (effect->Type == EUnitEffectType::OVER_TIME)
			break;
		else
		{
			float percentage = float(effect->Magnitude) / 100.0f;
			FAttackStat upgradedAttackStats = Team->GetUpgradedAttackStats(this);
			CurrentAttackStats.AttackCooldown += FMath::Clamp(upgradedAttackStats.AttackCooldown * percentage, 0.0f, upgradedAttackStats.AttackCooldown);
			GetMesh()->GlobalAnimRateScale -= percentage;
		}
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

bool ARTS_Entity::ApplyDamage(int damage, bool armor)
{
	//Apply the damage
	if (armor)
	{
		int d = damage;
		d -= CurrentDefenceStats.Armor;
		Health -= FMath::Clamp(d, 1, damage);
	}
	else
	{
		Health -= FMath::Clamp(damage, 1, damage);
	}

	//Check if character is dead
	if (Health <= 0)
	{
		Health = 0;
		Kill();
		return true;
	}
	return false;
}

void ARTS_Entity::ApplyHealing(int healing)
{
	Health += healing;
	if (Health > CurrentDefenceStats.MaxHealth)
	{
		Health = CurrentDefenceStats.MaxHealth;
	}
}

void ARTS_Entity::Kill()
{
	Health = 0;
	SetSelected(false);
	LocationOfDeath = GetActorLocation();
	ForwardOnDeath = GetCapsuleComponent()->GetForwardVector();

	//GameState notification
	ARTS_GameState* gameState = Cast<ARTS_GameState>(GetWorld()->GetGameState());
	gameState->OnDeathDelegate.Broadcast(this);

	// Play sound
	if (DeathSound && SoundAttenuation && SoundConcurrency)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation(), 1.f, 1.f, 0.f, SoundAttenuation, SoundConcurrency);
	}

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
				rtsPlayerController->UpdateSpecialistAbilityButtons();
				URTS_HUDBase* hud = rtsPlayerController->GetRTS_HUDBase();
				if (hud)
				{
					hud->UpdateSelectedEntities(castedGameState->SelectedEntities);
				}
			}
		}
	}

	//FDetachmentTransformRules rules = FDetachmentTransformRules::KeepWorldTransform;
	//GetMesh()->DetachFromComponent(rules);
	//GetMesh()->DetachFromParent();
	//Destroy(true);
}

bool ARTS_Entity::IsAlive()
{
	return (Health > 0);
}

void ARTS_Entity::AddToLumaSaturation(int32 LumaToAdd)
{
	CurrentLumaStats.LumaSaturation += LumaToAdd;
	if (CurrentLumaStats.LumaSaturation >= CurrentLumaStats.MaxLumaSaturation)
	{
		CurrentLumaStats.LumaSaturation = CurrentLumaStats.MaxLumaSaturation;
		ARTS_AIController* aiController = Cast<ARTS_AIController>(Controller);
		if (aiController)
		{
			aiController->SetCurrentTask(EUNIT_TASK::OVERDOSED);
		}
	}
}

void ARTS_Entity::RemoveLumaSaturation(int32 LumaToRemove)
{
	ARTS_AIController* aiController = Cast<ARTS_AIController>(Controller);
	if (aiController && aiController->GetCurrentTask() != EUNIT_TASK::OVERDOSED)
	{
		CurrentLumaStats.LumaSaturation -= LumaToRemove;
		if (CurrentLumaStats.LumaSaturation < 0)
		{
			CurrentLumaStats.LumaSaturation = 0;
		}
	}
}

bool ARTS_Entity::IsSelectableByPlayer() const
{
	ARTS_AIController* aiController = Cast<ARTS_AIController>(Controller);
	if (aiController)
	{
		bool selectable = (Health > 0) && 
			(aiController->GetCurrentTask() != EUNIT_TASK::OVERDOSED);
		return selectable;
	}

	return (Health > 0);
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

		case EUnitEffectStat::ATTACK_RATE:
		{
			float percentage = float(effect->Magnitude) / 100.0f;
			FAttackStat upgradedAttackStats = Team->GetUpgradedAttackStats(this);
			CurrentAttackStats.AttackCooldown -= FMath::Clamp(upgradedAttackStats.AttackCooldown * percentage, 0.0f, upgradedAttackStats.AttackCooldown);
			GetMesh()->GlobalAnimRateScale += percentage;
			break;
		}

		default:
			break;
		}
	}

	if (effect->Elapsed > effect->Duration)
	{
		effect->IsFinished = true;
	}
}

