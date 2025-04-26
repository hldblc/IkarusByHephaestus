// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "NavigationSystem.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"
#include "CompanionTaskData.generated.h"



/**
 * Core data structure for Behavior Tree tasks related to companion AI.
 * Contains common properties used by the task component system.
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FCompanionTaskData : public FTableRowBase
{
    GENERATED_BODY()

    FCompanionTaskData();

    /** Type of task */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration")
    ECompanionTask TaskType;
    
    /** Tag identifying this task (for gameplay tag system) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration")
    FGameplayTag TaskTag;
    
    /** Minimum duration this task should run for (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ClampMin="0.0", UIMin="0.0"))
    float MinDuration;
    
    /** Maximum duration this task should run for (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ClampMin="0.0", UIMin="0.0"))
    float MaxDuration;
    
    /** Base utility score for this task */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ClampMin="0.0", UIMin="0.0"))
    float BaseScore;
    
    /** Maximum search radius for the task */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ClampMin="0.0", UIMin="0.0"))
    float SearchRadius;
    
    /** Maximum interaction distance for this task */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ClampMin="0.0", UIMin="0.0"))
    float InteractionDistance;
    
    /** Maximum number of targets for this task */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ClampMin="0", UIMin="0"))
    int32 MaxTargets;
    
    /** Required blackboard keys for this task */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration")
    TArray<FBlackboardKeySelector> RequiredKeys;

    /** Task has started execution */
    UPROPERTY(Transient)
    uint8 bTaskInitialized : 1;

    /** Task is currently executing */
    UPROPERTY(Transient)
    uint8 bTaskActive : 1;

    /** Task requires a tick update */
    UPROPERTY(Transient)
    uint8 bRequiresTick : 1;

    /** Last time task was updated */
    UPROPERTY(Transient)
    float LastUpdateTime;
    
    /** Navigation system cache for navigation queries (set when needed) */
    UPROPERTY(Transient)
    UNavigationSystemV1* CachedNavSystem;
    
    /** Clear the task data when it's finished executing */
    void Reset();
};

/**
 * Memory structure for tasks that follow the player or track targets
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FFollowTaskData : public FCompanionTaskData
{
    GENERATED_BODY()

    FFollowTaskData();
    
    /** The last known location of the target being followed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Follow")
    FVector LastTargetLocation;

    /** Time since the last path update in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Follow")
    float LastPathUpdateTime;

    /** Time stamp when we last detected potential "stuck" (no movement) */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Follow")
    float LastStuckCheckTime;

    /** The pawn location when we last checked for movement — helps detect "no progress" */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Follow")
    FVector LastCheckedPawnLocation;
    
    /** ID for the current movement request, used to abort if needed */
    UPROPERTY(Transient)
    FAIRequestID CurrentRequestID;
    
    /** Whether the follower is actively following the target */
    UPROPERTY(Transient)
    uint8 bIsFollowing : 1;
    
    /** Whether the follower has a direct path to target */
    UPROPERTY(Transient)
    uint8 bHasDirectPath : 1;
    
    /** Number of consecutive times the follower couldn't find a path */
    UPROPERTY(Transient)
    int32 FailedPathCount;
};

/**
 * Memory structure for location-finding tasks
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FLocationTaskData : public FCompanionTaskData
{
    GENERATED_BODY()

    FLocationTaskData();
    
    /** Origin point for the search */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location")
    FVector OriginLocation;
    
    /** Final location that was found */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location")
    FVector FoundLocation;
    
    /** Previously visited locations to avoid if needed */
    UPROPERTY(Transient)
    TArray<FVector> PreviousLocations;
    
    /** Number of attempts made to find a valid location */
    UPROPERTY(Transient)
    int32 AttemptCount;
    
    /** Whether this task found a valid location */
    UPROPERTY(Transient)
    uint8 bFoundValidLocation : 1;
};

/**
 * Memory structure for patrol-related tasks
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FPatrolTaskData : public FCompanionTaskData
{
    GENERATED_BODY()

    FPatrolTaskData();
    
    /** List of patrol points (world positions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Patrol")
    TArray<FVector> PatrolPoints;

    /** Index of the current/next patrol point */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Patrol")
    int32 CurrentPointIndex;

    /** Time when we last reached a patrol point */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Patrol")
    float LastPointArrivalTime;
    
    /** Whether we should patrol in reverse order when reaching the end */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Patrol")
    uint8 bBidirectionalPatrol : 1;
    
    /** Direction of patrol (1 = forward, -1 = reverse) */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Patrol", meta=(EditCondition="bBidirectionalPatrol"))
    int32 PatrolDirection;
    
    /** Current patrol request ID */
    UPROPERTY(Transient)
    FAIRequestID PatrolRequestID;
};

/**
 * Memory structure for resource gathering tasks
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FGatherTaskData : public FCompanionTaskData
{
    GENERATED_BODY()

    FGatherTaskData();
    
    /** Type of resource we're targeting (e.g. "Herb", "Wood") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gather")
    FName ResourceType;

    /** Location of the current gathering target */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    FVector CurrentTargetLocation;
    
    /** Location of the last successful gather */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    FVector LastGatherLocation;

    /** Timestamp of last gather action */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    float LastGatherTime;

    /** How many units we've collected this session */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Gather")
    int32 TotalGathered;
    
    /** Current gathering cooldown remaining (seconds) */
    UPROPERTY(Transient)
    float GatherCooldownRemaining;
    
    /** Whether we're currently gathering resources */
    UPROPERTY(Transient)
    uint8 bIsGathering : 1;
    
    /** Whether we've found a valid resource nearby */
    UPROPERTY(Transient)
    uint8 bFoundResource : 1;
};

/**
 * Memory structure for search and investigation tasks
 */
USTRUCT(BlueprintType)
struct IKARUSTHECOMPANION_API FSearchTaskData : public FCompanionTaskData
{
    GENERATED_BODY()

    FSearchTaskData();
    
    /** Origin point of the investigation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Search")
    FVector SearchOrigin;
    
    /** Current investigation point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Search")
    FVector CurrentSearchPoint;
    
    /** Points that have already been investigated */
    UPROPERTY(Transient)
    TArray<FVector> InvestigatedPoints;
    
    /** Time spent at current investigation point */
    UPROPERTY(Transient)
    float TimeAtCurrentPoint;
    
    /** Whether something of interest was found */
    UPROPERTY(Transient)
    uint8 bFoundItemOfInterest : 1;
    
    /** Whether the search is actively running */
    UPROPERTY(Transient)
    uint8 bIsSearching : 1;
};