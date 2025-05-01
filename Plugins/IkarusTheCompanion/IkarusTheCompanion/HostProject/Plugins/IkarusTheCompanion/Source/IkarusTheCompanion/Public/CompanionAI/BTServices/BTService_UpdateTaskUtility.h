// BTService_UpdateTaskUtility.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTaskUtility.generated.h"

/**
 * Service that updates the utility scores for companion tasks
 */
UCLASS()
class IKARUSTHECOMPANION_API UBTService_UpdateTaskUtility : public UBTService
{
	GENERATED_BODY()
    
public:
	UBTService_UpdateTaskUtility();
    
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    
protected:
	// Score blackboard keys
	UPROPERTY(EditAnywhere, Category = "Utility")
	FName IdleScoreKey = "IdleScore";
    
	UPROPERTY(EditAnywhere, Category = "Utility")
	FName FollowScoreKey = "FollowScore";
    
	UPROPERTY(EditAnywhere, Category = "Utility")
	FName PatrolScoreKey = "PatrolScore";
    
	UPROPERTY(EditAnywhere, Category = "Utility")
	FName GatherScoreKey = "GatherScore";
    
	UPROPERTY(EditAnywhere, Category = "Utility")
	FName SearchScoreKey = "SearchScore";
};