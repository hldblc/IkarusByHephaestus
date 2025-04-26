// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"
#include "BTTask_FindPlayerLocation.generated.h"

/**
 * Behavior Tree Task that locates the player or finds a position near them
 * based on configurable parameters.
 */
UCLASS(BlueprintType, meta=(DisplayName="Find Player Location"))
class IKARUSTHECOMPANION_API UBTTask_FindPlayerLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	explicit UBTTask_FindPlayerLocation(FObjectInitializer const& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Core Parameters
	UPROPERTY(EditAnywhere, Category = "Task|Core")
	bool UseExactLocation;  // Use exact player position vs nearby point

	// Validation Parameters
	UPROPERTY(EditAnywhere, Category = "Task|Validation")
	bool RequireNavigablePath;  // Must be reachable via navmesh

	UPROPERTY(EditAnywhere, Category = "Task|Validation")
	bool RequireLineOfSight;  // Must have clear sight to location

	// Positioning Parameters
	UPROPERTY(EditAnywhere, Category = "Task|Positioning", meta = (EditCondition = "!UseExactLocation", ClampMin = "0.0"))
	float SearchRadius;  // Radius around player to search within

	UPROPERTY(EditAnywhere, Category = "Task|Positioning", meta = (EditCondition = "!UseExactLocation", ClampMin = "0.0"))
	float MinDistanceFromPlayer;  // Minimum distance to maintain (personal space)

	UPROPERTY(EditAnywhere, Category = "Task|Positioning", meta = (EditCondition = "!UseExactLocation"))
	bool UseDirectionalBias;  // Bias location search in a specific direction

	UPROPERTY(EditAnywhere, Category = "Task|Positioning", meta = (EditCondition = "UseDirectionalBias"))
	EPositioningPreference DirectionalPreference;  // Direction preference relative to player

	// Advanced Parameters
	UPROPERTY(EditAnywhere, Category = "Task|Advanced")
	bool AvoidPreviousLocations;  // Avoid previously selected points

	UPROPERTY(EditAnywhere, Category = "Task|Advanced", meta = (EditCondition = "AvoidPreviousLocations", ClampMin = "0"))
	int32 LocationMemorySize;  // How many previous locations to remember

	// Debug Parameters
	UPROPERTY(EditAnywhere, Category = "Task|Debug")
	bool DrawDebugPoints;  // Visualize search attempts in game

	UPROPERTY(EditAnywhere, Category = "Task|Debug")
	float DebugDuration;  // How long to show debug visuals

private:
	// Stores previously used locations when AvoidPreviousLocations is true
	TArray<FVector> PreviousLocations;

	// Helper function to check if a potential location has been used recently
	bool HasLocationBeenUsedRecently(const FVector& Location) const;
	
	// Helper function to find a valid location near the player
	bool FindNearbyLocation(AActor* PlayerActor, FVector& OutLocation);
	
	// Apply directional bias to candidate location
	FVector ApplyDirectionalBias(AActor* PlayerActor, const FVector& OriginLocation, float Distance) const;
};
