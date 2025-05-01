#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "BTTask_FollowPlayerTask.generated.h"

UCLASS(BlueprintType, meta=(DisplayName="Follow Player Task"))
class IKARUSTHECOMPANION_API UBTTask_FollowPlayerTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FollowPlayerTask(const FObjectInitializer&);

	// BT overrides
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}

protected:
	/* === Designer knobs === */
	UPROPERTY(EditAnywhere, Category="Follow")
	float AcceptanceRadius = 200.f;

	UPROPERTY(EditAnywhere, Category="Follow")
	bool bAllowPartialPath = false;

	UPROPERTY(EditAnywhere, Category="Follow")
	bool bCanStrafe = true;

	/** Optional class filter (pick in Details panel). */
	UPROPERTY(EditAnywhere, Category="Follow")
	TSubclassOf<UNavigationQueryFilter> NavFilter;

	/** Player must move this far (cm) before we re-plan. */
	UPROPERTY(EditAnywhere, Category="Follow")
	float RepathThreshold = 150.f;

private:
	FVector CachedTarget = FVector::ZeroVector;
	
	// Track current move request
	FAIRequestID CurrentMoveRequestID;
	bool bIsMoveActive = false;

	/* — delegate — */
	UFUNCTION()
	void HandleMoveFinished(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
	// Helper functions
	bool StartMoveRequest(AAIController* Controller, const FVector& Target);
	void CleanupDelegates(AAIController* Controller);
};