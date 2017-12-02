#pragma once

#include "CoreMinimal.h"
#include "Command.generated.h"

class AAbility;

UCLASS()
class WIPGATE_API UCommand : public UObject
{
	GENERATED_BODY()
public:
	UCommand() {}

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Ability Use Functions")
		void Execute();

	UPROPERTY(BlueprintReadWrite)
		bool IsComplete = false;
};


UCLASS()
class WIPGATE_API UCommand_Stop : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Stop() {}
};


UCLASS()
class WIPGATE_API UCommand_Move : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Move(FVector target) : Target(target) {}
	
	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_Move() {}
};


UCLASS()
class WIPGATE_API UCommand_AttackMove : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_AttackMove(FVector target) : Target(target) {}

	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_AttackMove() {}
};

UCLASS()
class WIPGATE_API UCommand_Patrol : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Patrol(FVector current, FVector target) : Current(current), Target(target) {}

	UPROPERTY(BlueprintReadWrite)
		FVector Current;
	UPROPERTY(BlueprintReadWrite)
		FVector Target;

private:
	UCommand_Patrol() {}
};


UCLASS()
class WIPGATE_API UCommand_Attack : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Attack() {} // entity target

	// entity target
	//UPROPERTY(BlueprintReadWrite)
	//	FVector Target;

//private:
	//UCommand_Attack() {}
};


UCLASS()
class WIPGATE_API UCommand_Cast : public UCommand
{
	GENERATED_BODY()
public:
	UCommand_Cast(AAbility* ability) : Ability(ability) {}

	UPROPERTY(BlueprintReadWrite)
		AAbility* Ability;

private:
	UCommand_Cast() {}
};