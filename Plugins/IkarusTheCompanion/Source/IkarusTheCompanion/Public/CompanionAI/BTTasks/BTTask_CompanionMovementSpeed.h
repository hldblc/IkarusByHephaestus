#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"
#include "BTTask_CompanionMovementSpeed.generated.h"

UCLASS(BlueprintType, meta=(DisplayName="Set Companion Movement Speed"))
class IKARUSTHECOMPANION_API UBTTask_CompanionMovementSpeed : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	/** Required UObject-style constructor */
	UBTTask_CompanionMovementSpeed(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
