#pragma once

#include "CoreMinimal.h"
#include "Command.generated.h"

class AAbility;
class ARTS_Entity;

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
	UCommand() {}

	//UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Ability Use Functions")
	//	void Execute();

	UPROPERTY(BlueprintReadWrite)
		bool IsCompleted = false;
	UPROPERTY(BlueprintReadWrite)
		ECOMMAND_TYPE Type = ECOMMAND_TYPE::NONE;
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_Stop : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Stop() 
	{ Type = ECOMMAND_TYPE::STOP; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_MoveToLocation : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_MoveToLocation(FVector target) : Target(target) 
	{ Type = ECOMMAND_TYPE::MOVE_TO_LOCATION; }
	
	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_MoveToLocation() 
	{ Type = ECOMMAND_TYPE::MOVE_TO_LOCATION; }
};

UCLASS(Blueprintable)
class WIPGATE_API UCommand_MoveToEntity : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_MoveToEntity(ARTS_Entity* target) : Target(target) 
	{ Type = ECOMMAND_TYPE::MOVE_TO_ENTITY; }

	UPROPERTY(BlueprintReadWrite)
		ARTS_Entity* Target;

private:
	UCommand_MoveToEntity() 
	{ Type = ECOMMAND_TYPE::MOVE_TO_ENTITY; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_AttackMove : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_AttackMove(FVector target) : Target(target) 
	{ Type = ECOMMAND_TYPE::ATTACK_MOVE; }

	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_AttackMove() 
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
	UCommand_Patrol()
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
	UCommand_Attack()
	{ Type = ECOMMAND_TYPE::ATTACK; }
};


UCLASS(Blueprintable)
class WIPGATE_API UCommand_Cast : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Cast(AAbility* ability) : Ability(ability) 
	{ Type = ECOMMAND_TYPE::CAST; }

	UPROPERTY(BlueprintReadWrite)
		AAbility* Ability;

private:
	UCommand_Cast() 
	{ Type = ECOMMAND_TYPE::CAST; }
};