#pragma once

#include "CoreMinimal.h"
#include "Command.generated.h"

class AAbility;
class ARTS_Entity;
class ARTS_Unit;

UENUM(BlueprintType)
enum class ECOMMAND_TYPE : uint8
{
	NONE 					UMETA(DisplayName = "None"),
	STOP 					UMETA(DisplayName = "Stop"),
	MOVE_TO_LOCATION 		UMETA(DisplayName = "Move To Location"),
	MOVE_TO_ENTITY 			UMETA(DisplayName = "Move To Entity"),
	ATTACK_MOVE 			UMETA(DisplayName = "Attack Move"),
	PATROL 					UMETA(DisplayName = "Patrol"),
	ATTACK 					UMETA(DisplayName = "Attack"),
	CAST 					UMETA(DisplayName = "Cast Ability"),
};


UCLASS(Blueprintable, BlueprintType)
class WIPGATE_API UCommand : public UObject
{
	GENERATED_BODY()
public:
	UCommand() {};
	UCommand(bool isForced) : IsForced(IsForced) {}

	UPROPERTY(BlueprintReadWrite)
		ECOMMAND_TYPE Type = ECOMMAND_TYPE::NONE;
	UPROPERTY(BlueprintReadWrite)
		bool IsForced = true;

};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_Stop : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Stop() : UCommand(true)
	{ Type = ECOMMAND_TYPE::STOP; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_MoveToLocation : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_MoveToLocation(FVector target, bool isForced) : UCommand(isForced), Target(target)
	{ Type = ECOMMAND_TYPE::MOVE_TO_LOCATION; }
	
	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_MoveToLocation() : UCommand(true)
	{ Type = ECOMMAND_TYPE::MOVE_TO_LOCATION; }
};

UCLASS(Blueprintable)
class WIPGATE_API UCommand_MoveToEntity : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_MoveToEntity(ARTS_Entity* target, bool isForced) : UCommand(isForced), Target(target)
	{ Type = ECOMMAND_TYPE::MOVE_TO_ENTITY; }

	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity* Target;

private:
	UCommand_MoveToEntity() : UCommand(true)
	{ Type = ECOMMAND_TYPE::MOVE_TO_ENTITY; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_AttackMove : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_AttackMove(FVector target, bool isForced) : UCommand(isForced), Target(target)
	{ Type = ECOMMAND_TYPE::ATTACK_MOVE; }

	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_AttackMove() : UCommand(true)
	{ Type = ECOMMAND_TYPE::ATTACK_MOVE; }
};

UCLASS(Blueprintable)
class WIPGATE_API UCommand_Patrol : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Patrol(FVector current, FVector target) : Current(current), Target(target)
	{ Type = ECOMMAND_TYPE::PATROL; }

	UPROPERTY(BlueprintReadWrite)
		FVector Current;
	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_Patrol() : UCommand(true)
	{ Type = ECOMMAND_TYPE::PATROL; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_Attack : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Attack(ARTS_Entity* target) : Target(target)
	{ Type = ECOMMAND_TYPE::ATTACK; } 

	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity* Target;

private:
	UCommand_Attack() : UCommand(true)
	{ Type = ECOMMAND_TYPE::ATTACK; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_CastTarget : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_CastTarget(AAbility* ability, ARTS_Unit* target)
		: Ability(ability), TargetUnit(target)
	{ Type = ECOMMAND_TYPE::CAST; }

	UPROPERTY(BlueprintReadWrite)
		AAbility* Ability;
	UPROPERTY(BlueprintReadWrite)
		ARTS_Unit* TargetUnit;

private:
	UCommand_CastTarget() : UCommand(true)
	{ Type = ECOMMAND_TYPE::CAST; }
};

UCLASS(Blueprintable)
class WIPGATE_API UCommand_CastGround : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_CastGround(AAbility* ability, const FVector target) 
		: Ability(ability), TargetLocation(target)
	{ Type = ECOMMAND_TYPE::CAST; }

	UPROPERTY(BlueprintReadWrite)
		AAbility* Ability;
	UPROPERTY(BlueprintReadWrite)
		FVector TargetLocation;

private:
	UCommand_CastGround() : UCommand(true)
	{
		Type = ECOMMAND_TYPE::CAST;
	}
};