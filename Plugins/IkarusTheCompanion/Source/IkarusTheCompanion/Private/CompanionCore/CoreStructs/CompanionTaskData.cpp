// Fill out your copyright notice in the Description page of Project Settings.

#include "CompanionCore/CoreStructs//CompanionTaskData.h"

FCompanionTaskData::FCompanionTaskData()
    : TaskType(ECompanionTask::None)
    , MinDuration(0.0f)
    , MaxDuration(0.0f)
    , BaseScore(0.0f)
    , SearchRadius(500.0f)
    , InteractionDistance(200.0f)
    , MaxTargets(1)
    , bTaskInitialized(false)
    , bTaskActive(false)
    , bRequiresTick(false)
    , LastUpdateTime(0.0f)
    , CachedNavSystem(nullptr)
{
    // FBlackboardKeySelector elements don't need specific initialization
    RequiredKeys = TArray<FBlackboardKeySelector>();
}

void FCompanionTaskData::Reset()
{
    bTaskInitialized = false;
    bTaskActive = false;
    LastUpdateTime = 0.0f;
    // Note: We don't reset the navigation system cache as it's expensive to get
}

FFollowTaskData::FFollowTaskData()
    : FCompanionTaskData()
    , LastTargetLocation(FVector::ZeroVector)
    , LastPathUpdateTime(0.0f)
    , LastStuckCheckTime(0.0f)
    , LastCheckedPawnLocation(FVector::ZeroVector)
    , bIsFollowing(false)
    , bHasDirectPath(false)
    , FailedPathCount(0)
{
    TaskType = ECompanionTask::Follow;
    CurrentRequestID = FAIRequestID::InvalidRequest;
}

FLocationTaskData::FLocationTaskData()
    : FCompanionTaskData()
    , OriginLocation(FVector::ZeroVector)
    , FoundLocation(FVector::ZeroVector)
    , AttemptCount(0)
    , bFoundValidLocation(false)
{
}

FPatrolTaskData::FPatrolTaskData()
    : FCompanionTaskData()
    , CurrentPointIndex(0)
    , LastPointArrivalTime(0.0f)
    , bBidirectionalPatrol(false)
    , PatrolDirection(1)
{
    TaskType = ECompanionTask::Patrol;
    PatrolRequestID = FAIRequestID::InvalidRequest;
}

FGatherTaskData::FGatherTaskData()
    : FCompanionTaskData()
    , ResourceType(NAME_None)
    , CurrentTargetLocation(FVector::ZeroVector)
    , LastGatherLocation(FVector::ZeroVector)
    , LastGatherTime(0.0f)
    , TotalGathered(0)
    , GatherCooldownRemaining(0.0f)
    , bIsGathering(false)
    , bFoundResource(false)
{
    TaskType = ECompanionTask::Gather;
}

FSearchTaskData::FSearchTaskData()
    : FCompanionTaskData()
    , SearchOrigin(FVector::ZeroVector)
    , CurrentSearchPoint(FVector::ZeroVector)
    , TimeAtCurrentPoint(0.0f)
    , bFoundItemOfInterest(false)
    , bIsSearching(false)
{
    TaskType = ECompanionTask::Search;
}