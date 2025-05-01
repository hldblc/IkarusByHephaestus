// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UpdatePlayerLocation.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta=(DisplayName="Update Player Location"))
class IKARUSTHECOMPANION_API UUpdatePlayerLocation : public UBTService
{
	GENERATED_BODY()

public:
	UUpdatePlayerLocation();


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
