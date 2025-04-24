#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdatePlayerLocation.generated.h"

/**
 * Service that continuously updates a vector location in the blackboard
 * based on a target actor's position
 */
UCLASS(BlueprintType, meta=(DisplayName="Update Player Location"))
class IKARUSTHECOMPANION_API UBTService_UpdatePlayerLocation : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_UpdatePlayerLocation();
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	// The blackboard key to store the location (must be Vector type)
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector LocationKey;
	
	// The blackboard key containing the target actor (must be Object type)
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;
	
	// Update frequency in seconds
	UPROPERTY(EditAnywhere, Category="Service")
	float UpdateInterval = 0.5f;
}; 