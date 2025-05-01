// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"
#include "SetMovementSpeed.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta=(DisplayName="Set Companion Movement Speed"))
class IKARUSTHECOMPANION_API USetMovementSpeed : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	/** Required UObject-style constructor */
	USetMovementSpeed(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category="Speed")
	ECompanionMovementSpeed CompanionSpeed = ECompanionMovementSpeed::Walking;

	UPROPERTY(EditAnywhere, Category="Speed", meta=(InlineEditConditionToggle))
	bool bAutomaticFromDistance = false;

	UPROPERTY(EditAnywhere, Category="Speed", meta=(EditCondition="bAutomaticFromDistance"))
	FBlackboardKeySelector CompanionProximityKey;

	UPROPERTY(EditAnywhere, Category="Debug")
	FBlackboardKeySelector DebugSpeedKey;

private:
	static ECompanionMovementSpeed ProximityToEnum(float Proximity);
};
