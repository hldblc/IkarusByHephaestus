// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "Engine/DataTable.h"
#include "CompanionCoreStructs.generated.h"

/**
 * A structure representing the core stats of a companion entity.
 * This structure is designed to track critical vitals such as health, stamina,
 * hunger, and thirst, enabling dynamic interaction with gameplay systems.
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FCompanionStats
{
    GENERATED_BODY()

    /** Current health level, 0 is death */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats", meta=(ClampMin="0.0", ClampMax="100.0", UIMin="0.0", UIMax="100.0"))
    float Health = 100.f;

    /** Current stamina level, depleted when running or performing special actions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats", meta=(ClampMin="0.0", ClampMax="100.0", UIMin="0.0", UIMax="100.0"))
    float Stamina = 100.f;

    /** Current hunger level, 100 is starving */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats", meta=(ClampMin="0.0", ClampMax="100.0", UIMin="0.0", UIMax="100.0"))
    float Hunger = 0.f;

    /** Current thirst level, 100 is dehydrated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats", meta=(ClampMin="0.0", ClampMax="100.0", UIMin="0.0", UIMax="100.0"))
    float Thirst = 0.f;
};

/**
 * A structure that maintains memory for entities following a player character.
 * This structure records essential data, such as the last known target location
 * and the time of the last path update, to simulate logical follow behavior.
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FFollowPlayerMemory : public FTableRowBase
{
    GENERATED_BODY()

    FFollowPlayerMemory();

    /** The last known location of the target being followed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Follow")
    FVector LastTargetLocation;

    /** Time since the last path update in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Follow")
    float LastPathUpdateTime = 0.0f;

    /** Time stamp when we last detected potential "stuck" (no movement) */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Follow")
    float LastStuckCheckTime = 0.0f;

    /** The pawn location when we last checked for movement — helps detect "no progress" */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Follow")
    FVector LastCheckedPawnLocation;
    
    /** ID for the current movement request, used to abort if needed */
    UPROPERTY(Transient)
    FAIRequestID CurrentRequestID;
    
    /** Whether the follower is actively following the target */
    UPROPERTY(Transient)
    uint8 bIsFollowing : 1;
};

/**
 * Memory structure for AI patrol behavior
 * Tracks patrol points and current patrol status
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FPatrolMemory
{
    GENERATED_BODY()

    /** List of patrol points (world positions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Patrol")
    TArray<FVector> PatrolPoints;

    /** Index of the next patrol point */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Patrol")
    int32 CurrentPointIndex = 0;

    /** Time when we last reached a patrol point */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Patrol")
    float LastPointArrivalTime = 0.f;
    
    /** Whether we should patrol in reverse order when reaching the end */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Patrol")
    uint8 bBidirectionalPatrol : 1;
    
    /** Direction of patrol (1 = forward, -1 = reverse) */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Patrol", meta=(EditCondition="bBidirectionalPatrol"))
    int32 PatrolDirection = 1;
};

/** Resource‐gathering memory */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FGatherResourceMemory
{
    GENERATED_BODY()

    /** Type of resource we're targeting (e.g. "Herb", "Wood") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gather")
    FName ResourceType;

    /** Location of the last successful gather */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    FVector LastGatherLocation;

    /** Timestamp of last gather action */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    float LastGatherTime = 0.f;

    /** How many units we've collected this session */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    int32 TotalGathered = 0;
    
    /** Time in seconds between gather attempts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gather")
    float GatherCooldown = 3.0f;
    
    /** Maximum distance at which gathering is possible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gather")
    float GatherRange = 200.0f;
};