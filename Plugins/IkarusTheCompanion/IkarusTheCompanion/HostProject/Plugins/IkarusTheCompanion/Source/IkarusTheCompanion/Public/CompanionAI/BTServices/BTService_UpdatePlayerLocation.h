#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdatePlayerLocation.generated.h"

/**
 * Server-side (default) service that writes the player’s world-space
 * location to a Vector blackboard key – *only* when it matters.
 *
 *   • skips work on clients (toggleable) and during replays if desired  
 *   • early-outs when player is out of MaxFollowRange or not in line-of-sight  
 *   • ignores sub-centimetre jitter thanks to MoveThreshold  
 *   • caches key-IDs & player pointer for zero string hashing / casting in tick
 */
UCLASS(BlueprintType, meta=(DisplayName="Update Player Location (Optimised)"))
class IKARUSTHECOMPANION_API UBTService_UpdatePlayerLocation : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdatePlayerLocation();

protected:
	/* -------- Designer knobs -------- */

	/** Vector key updated each time the player moves far enough. */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector PlayerLocationKey;

	/** (Optional) Object key that already stores the player pawn / character. */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector PlayerActorKey;

	/** Player must move this many centimetres before we rewrite the key. */
	UPROPERTY(EditAnywhere, Category="Performance", meta=(ClampMin="1"))
	float MoveThreshold = 15.f;

	/** Hard distance cap – outside this the service does no work. */
	UPROPERTY(EditAnywhere, Category="Performance", meta=(ClampMin="10"))
	float MaxFollowRange = 8000.f;

	/** Run update logic on simulated clients as well (usually false). */
	UPROPERTY(EditAnywhere, Category="Performance")
	bool bRunOnClients = false;

	/** Also tick while the game is replaying (spectator). */
	UPROPERTY(EditAnywhere, Category="Performance")
	bool bRunWhileReplaying = false;

	/* -------- BT overrides -------- */
	void InitializeFromAsset(UBehaviorTree& Asset) override;
	void TickNode(UBehaviorTreeComponent& OwnerComp,
	              uint8* NodeMemory,
	              float DeltaSeconds) override;

private:
	/* Cached blackboard key IDs (filled in InitializeFromAsset) */
	FBlackboard::FKey LocKeyId  = FBlackboard::InvalidKey;
	FBlackboard::FKey ActKeyId  = FBlackboard::InvalidKey;

	/* MoveThreshold squared – avoids FMath::Square each tick */
	float MoveThresholdSq = 225.f;            /* 15 cm² default */

	/* Local cache of last location and player pointer */
	FVector               LastWritten = FVector::ZeroVector;
	TWeakObjectPtr<AActor> CachedPlayer;
};
