// Fill out your copyright notice in the Description page of Project Settings.

#include "CompanionAI/Components/CompanionTaskComponent.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "NavigationSystem.h"

UCompanionTaskComponent::UCompanionTaskComponent()
{
    // Enable component tick and replication
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Optimize performance with reduced tick rate
    SetIsReplicatedByDefault(true);
    
    // Initialize task templates with default values
    // Idle task template
    FCompanionTaskData IdleTemplate;
    IdleTemplate.TaskType = ECompanionTask::Idle;
    IdleTemplate.MinDuration = 5.0f;
    IdleTemplate.MaxDuration = 15.0f;
    IdleTemplate.BaseScore = 0.2f;
    IdleTemplate.SearchRadius = 300.0f;
    TaskTemplates.Add(ECompanionTask::Idle, IdleTemplate);
    
    // Follow task template
    FCompanionTaskData FollowTemplate;
    FollowTemplate.TaskType = ECompanionTask::Follow;
    FollowTemplate.MinDuration = 30.0f;
    FollowTemplate.MaxDuration = 300.0f;
    FollowTemplate.BaseScore = 0.5f;
    FollowTemplate.SearchRadius = 500.0f;
    FollowTemplate.InteractionDistance = 200.0f;
    TaskTemplates.Add(ECompanionTask::Follow, FollowTemplate);
    
    // Patrol task template
    FCompanionTaskData PatrolTemplate;
    PatrolTemplate.TaskType = ECompanionTask::Patrol;
    PatrolTemplate.MinDuration = 60.0f;
    PatrolTemplate.MaxDuration = 180.0f;
    PatrolTemplate.BaseScore = 0.3f;
    PatrolTemplate.SearchRadius = 1000.0f;
    TaskTemplates.Add(ECompanionTask::Patrol, PatrolTemplate);
    
    // Gather task template
    FCompanionTaskData GatherTemplate;
    GatherTemplate.TaskType = ECompanionTask::Gather;
    GatherTemplate.MinDuration = 20.0f;
    GatherTemplate.MaxDuration = 120.0f;
    GatherTemplate.BaseScore = 0.8f;
    GatherTemplate.SearchRadius = 800.0f;
    GatherTemplate.InteractionDistance = 150.0f;
    GatherTemplate.MaxTargets = 5;
    TaskTemplates.Add(ECompanionTask::Gather, GatherTemplate);
    
    // Search task template
    FCompanionTaskData SearchTemplate;
    SearchTemplate.TaskType = ECompanionTask::Search;
    SearchTemplate.MinDuration = 30.0f;
    SearchTemplate.MaxDuration = 90.0f;
    SearchTemplate.BaseScore = 0.6f;
    SearchTemplate.SearchRadius = 1500.0f;
    TaskTemplates.Add(ECompanionTask::Search, SearchTemplate);
}

void UCompanionTaskComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize current task to None
    CurrentTask.TaskType = ECompanionTask::None;
    
    // Setup blackboard if we're on the server
    if (GetOwnerRole() == ROLE_Authority)
    {
        UBlackboardComponent* Blackboard = GetBlackboard();
        if (Blackboard)
        {
            // Initialize blackboard values for task tracking
            Blackboard->SetValueAsEnum(FName("CurrentTaskType"), (uint8)ECompanionTask::None);
            Blackboard->SetValueAsBool(FName("IsTaskActive"), false);
            Blackboard->SetValueAsFloat(FName("TaskCompletion"), 0.0f);
        }
    }
}

void UCompanionTaskComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only tick on authority
    if (GetOwnerRole() == ROLE_Authority)
    {
        TickTask(DeltaTime);
    }
}

void UCompanionTaskComponent::ServerStartTask_Implementation(const FCompanionTaskData& TaskData)
{
    // Abort any current task
    ServerAbortTask();

    // Store the task data
    CurrentTask = TaskData;

    // Setup task timing
    TaskElapsedTime = 0.0f;
    
    // Calculate end time based on min/max duration
    if (CurrentTask.MinDuration == CurrentTask.MaxDuration)
    {
        TaskEndTime = CurrentTask.MinDuration;
    }
    else
    {
        TaskEndTime = FMath::RandRange(CurrentTask.MinDuration, CurrentTask.MaxDuration);
    }
    
    // Initialize specialized task data based on task type
    InitializeSpecializedTaskData();

    // Update blackboard
    UpdateBlackboardWithTaskState();
}

void UCompanionTaskComponent::ServerAbortTask_Implementation()
{
    if (CurrentTask.TaskType == ECompanionTask::None)
    {
        return; // No task to abort
    }
    
    // Cleanup based on task type
    switch (CurrentTask.TaskType)
    {
        case ECompanionTask::Follow:
            {
                // Abort movement request
                AAICompanionController* Controller = GetCompanionController();
                if (Controller)
                {
                    UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
                    if (PathComp && !FollowData.CurrentRequestID.IsValid())
                    {
                        PathComp->AbortMove(*this, FPathFollowingResultFlags::MovementStop, FollowData.CurrentRequestID);
                    }
                }
            }
            break;
            
        case ECompanionTask::Patrol:
            {
                // Abort movement request
                AAICompanionController* Controller = GetCompanionController();
                if (Controller)
                {
                    UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
                    if (PathComp && !PatrolData.PatrolRequestID.IsValid())
                    {
                        PathComp->AbortMove(*this, FPathFollowingResultFlags::MovementStop, PatrolData.PatrolRequestID);
                    }
                }
            }
            break;
            
        default:
            break;
    }
    
    // Reset current task
    CurrentTask.TaskType = ECompanionTask::None;
    
    // Update blackboard
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(FName("CurrentTaskType"), (uint8)ECompanionTask::None);
        Blackboard->SetValueAsBool(FName("IsTaskActive"), false);
        Blackboard->SetValueAsFloat(FName("TaskCompletion"), 0.0f);
    }
}

void UCompanionTaskComponent::TickTask(float DeltaTime)
{
    // Check if we have an active task
    if (CurrentTask.TaskType == ECompanionTask::None)
    {
        return;
    }

    // Update elapsed time
    TaskElapsedTime += DeltaTime;
    
    // Update task completion in blackboard
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (Blackboard)
    {
        Blackboard->SetValueAsFloat(FName("TaskCompletion"), GetTaskCompletion());
    }

    // Execute the appropriate task function
    switch (CurrentTask.TaskType)
    {
        case ECompanionTask::Idle:
            ExecuteIdleTask(DeltaTime);
            break;
        case ECompanionTask::Follow:
            ExecuteFollowTask(DeltaTime);
            break;
        case ECompanionTask::Patrol:
            ExecutePatrolTask(DeltaTime);
            break;
        case ECompanionTask::Gather:
            ExecuteGatherTask(DeltaTime);
            break;
        case ECompanionTask::Search:
            ExecuteSearchTask(DeltaTime);
            break;
        default:
            break;
    }

    // Check if task duration expired
    if (TaskEndTime > 0.0f && TaskElapsedTime >= TaskEndTime)
    {
        CompleteTask(true);
    }
}

bool UCompanionTaskComponent::ValidateBlackboardRequirements(const FCompanionTaskData& TaskData)
{
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (!Blackboard)
    {
        return false;
    }

    // Check that all required keys exist and have valid values
    for (const FBlackboardKeySelector& KeySelector : TaskData.RequiredKeys)
    {
        const FName KeyName = KeySelector.SelectedKeyName;
        if (KeyName == NAME_None)
        {
            continue; // Skip invalid keys
        }

        // Check if the key has a valid value based on its type
        UClass* KeyTypeClass = KeySelector.SelectedKeyType;
        if (!KeyTypeClass)
        {
            continue;
        }

        // Check common key types
        if (KeyTypeClass == UBlackboardKeyType_Vector::StaticClass())
        {
            if (!Blackboard->IsVectorValueSet(KeyName))
            {
                return false;
            }
        }
        else if (KeyTypeClass == UBlackboardKeyType_Object::StaticClass())
        {
            if (Blackboard->GetValueAsObject(KeyName) == nullptr)
            {
                return false;
            }
        }
        else if (KeyTypeClass == UBlackboardKeyType_Bool::StaticClass())
        {
            // For boolean keys, just check they exist - no specific validation needed
        }
        else if (KeyTypeClass == UBlackboardKeyType_Float::StaticClass())
        {
            // For float keys, just check they exist - no specific validation needed
        }
    }

    return true;
}

void UCompanionTaskComponent::InitializeSpecializedTaskData()
{
    switch (CurrentTask.TaskType)
    {
        case ECompanionTask::Follow:
            {
                // Initialize follow data
                FollowData = FFollowTaskData();
                FollowData.TaskType = ECompanionTask::Follow;
                FollowData.bIsFollowing = true;
                
                // Get initial player location if available
                AAICompanionController* Controller = GetCompanionController();
                if (Controller)
                {
                    ACharacter* OwnerPlayer = Controller->GetOwnerPlayer();
                    if (OwnerPlayer)
                    {
                        FollowData.LastTargetLocation = OwnerPlayer->GetActorLocation();
                    }
                }
                
                // Initialize AI pawn location for movement tracking
                APawn* OwnerPawn = Cast<APawn>(GetOwner());
                if (OwnerPawn)
                {
                    FollowData.LastCheckedPawnLocation = OwnerPawn->GetActorLocation();
                }
            }
            break;
            
        case ECompanionTask::Patrol:
            {
                // Initialize patrol data
                PatrolData = FPatrolTaskData();
                PatrolData.TaskType = ECompanionTask::Patrol;
                PatrolData.CurrentPointIndex = 0;
                PatrolData.PatrolDirection = 1;
                PatrolData.bBidirectionalPatrol = true;
                
                // Generate patrol points if needed
                if (PatrolData.PatrolPoints.Num() == 0)
                {
                    // This would generate patrol points based on current location
                    // Simplified version for this example
                    APawn* OwnerPawn = Cast<APawn>(GetOwner());
                    if (OwnerPawn)
                    {
                        FVector BaseLocation = OwnerPawn->GetActorLocation();
                        PatrolData.PatrolPoints.Add(BaseLocation);
                        
                        // Add some points around the base location
                        for (int32 i = 0; i < 4; ++i)
                        {
                            FVector Offset = FVector(
                                FMath::RandRange(-CurrentTask.SearchRadius, CurrentTask.SearchRadius),
                                FMath::RandRange(-CurrentTask.SearchRadius, CurrentTask.SearchRadius),
                                0.0f
                            );
                            PatrolData.PatrolPoints.Add(BaseLocation + Offset);
                        }
                    }
                }
            }
            break;
            
        case ECompanionTask::Gather:
            {
                // Initialize gather data
                GatherData = FGatherTaskData();
                GatherData.TaskType = ECompanionTask::Gather;
                
                // Set resource type if available from blackboard
                UBlackboardComponent* Blackboard = GetBlackboard();
                if (Blackboard && Blackboard->GetKeyID(FName("ResourceType")) != FBlackboard::InvalidKey)
                {
                    GatherData.ResourceType = Blackboard->GetValueAsName(FName("ResourceType"));
                }
                else
                {
                    GatherData.ResourceType = FName("Default");
                }
            }
            break;
            
        case ECompanionTask::Search:
            {
                // Initialize search data
                SearchData = FSearchTaskData();
                SearchData.TaskType = ECompanionTask::Search;
                SearchData.bIsSearching = true;
                
                // Set search origin to current location
                APawn* OwnerPawn = Cast<APawn>(GetOwner());
                if (OwnerPawn)
                {
                    SearchData.SearchOrigin = OwnerPawn->GetActorLocation();
                }
            }
            break;
            
        default:
            break;
    }
}

void UCompanionTaskComponent::UpdateBlackboardWithTaskState()
{
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (!Blackboard)
    {
        return;
    }
    
    // Update common task state
    Blackboard->SetValueAsEnum(FName("CurrentTaskType"), (uint8)CurrentTask.TaskType);
    Blackboard->SetValueAsBool(FName("IsTaskActive"), CurrentTask.TaskType != ECompanionTask::None);
    Blackboard->SetValueAsFloat(FName("TaskStartTime"), GetWorld()->GetTimeSeconds());
    Blackboard->SetValueAsFloat(FName("TaskDuration"), TaskEndTime);
    Blackboard->SetValueAsFloat(FName("TaskCompletion"), 0.0f);
    
    // Task-specific blackboard updates
    switch (CurrentTask.TaskType)
    {
        case ECompanionTask::Follow:
            Blackboard->SetValueAsBool(FName("IsFollowing"), true);
            break;
        case ECompanionTask::Patrol:
            Blackboard->SetValueAsBool(FName("IsPatrolling"), true);
            break;
        case ECompanionTask::Gather:
            Blackboard->SetValueAsBool(FName("IsGathering"), true);
            Blackboard->SetValueAsName(FName("GatherResourceType"), GatherData.ResourceType);
            break;
        case ECompanionTask::Search:
            Blackboard->SetValueAsBool(FName("IsSearching"), true);
            break;
        default:
            break;
    }
}

float UCompanionTaskComponent::CalculateTaskUtility(ECompanionTask TaskType)
{
    // Get base score from task template
    float Score = 0.0f;
    if (TaskTemplates.Contains(TaskType))
    {
        Score = TaskTemplates[TaskType].BaseScore;
    }
    
    // Apply modifiers based on contextual factors
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (!Blackboard)
    {
        return Score;
    }
    
    const FName CurrentStaminaKey = FName("CurrentStamina");
    const FName MaxStaminaKey = FName("MaxStamina");
    const FName OwnerDistanceKey = FName("OwnerDistance");
    const FName IsPlayerMovingKey = FName("IsPlayerMoving");
    const FName IsThreatDetectedKey = FName("IsThreatDetected");
    const FName IsResourceDetectedKey = FName("IsResourceDetected");
    const FName InventorySpaceKey = FName("InventorySpace");
    const FName ResourceAmountKey = FName("ResourceAmount");
    const FName ExplorationPercentageKey = FName("ExplorationPercentage");
    
    switch (TaskType)
    {
        case ECompanionTask::Idle:
            // Idle is more useful when character is tired
            if (Blackboard->GetKeyID(CurrentStaminaKey) != FBlackboard::InvalidKey && 
                Blackboard->GetKeyID(MaxStaminaKey) != FBlackboard::InvalidKey)
            {
                float StaminaRatio = Blackboard->GetValueAsFloat(CurrentStaminaKey) / 
                                    Blackboard->GetValueAsFloat(MaxStaminaKey);
                Score += (1.0f - StaminaRatio) * 0.5f;
            }
            break;
            
        case ECompanionTask::Follow:
            // Follow is more useful when far from player
            if (Blackboard->GetKeyID(OwnerDistanceKey) != FBlackboard::InvalidKey)
            {
                float Distance = Blackboard->GetValueAsFloat(OwnerDistanceKey);
                Score += FMath::Clamp(Distance / 1000.0f, 0.0f, 0.5f);
            }
            // And less useful when player is stationary
            if (Blackboard->GetKeyID(IsPlayerMovingKey) != FBlackboard::InvalidKey)
            {
                bool bPlayerMoving = Blackboard->GetValueAsBool(IsPlayerMovingKey);
                if (!bPlayerMoving)
                {
                    Score -= 0.2f;
                }
            }
            break;
            
        case ECompanionTask::Patrol:
            // Patrol is more useful when no threats detected
            if (Blackboard->GetKeyID(IsThreatDetectedKey) != FBlackboard::InvalidKey)
            {
                bool bThreatDetected = Blackboard->GetValueAsBool(IsThreatDetectedKey);
                if (!bThreatDetected)
                {
                    Score += 0.2f;
                }
            }
            break;
            
        case ECompanionTask::Gather:
            // Gather is more useful when resources detected or when inventory not full
            if (Blackboard->GetKeyID(IsResourceDetectedKey) != FBlackboard::InvalidKey)
            {
                bool bResourceDetected = Blackboard->GetValueAsBool(IsResourceDetectedKey);
                if (bResourceDetected)
                {
                    Score += 0.3f;
                }
            }
            if (Blackboard->GetKeyID(InventorySpaceKey) != FBlackboard::InvalidKey && 
                Blackboard->GetKeyID(ResourceAmountKey) != FBlackboard::InvalidKey)
            {
                int32 InventorySpace = Blackboard->GetValueAsInt(InventorySpaceKey);
                int32 ResourceAmount = Blackboard->GetValueAsInt(ResourceAmountKey);
                float InventoryRatio = (float)ResourceAmount / (float)InventorySpace;
                Score += (1.0f - InventoryRatio) * 0.4f;
            }
            break;
            
        case ECompanionTask::Search:
            // Search is more useful when in unexplored areas
            if (Blackboard->GetKeyID(ExplorationPercentageKey) != FBlackboard::InvalidKey)
            {
                float ExplorationPct = Blackboard->GetValueAsFloat(ExplorationPercentageKey);
                Score += (1.0f - ExplorationPct) * 0.4f;
            }
            break;
            
        default:
            break;
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

FCompanionTaskData UCompanionTaskComponent::GetTaskDataForType(ECompanionTask TaskType)
{
    // Return template if available
    if (TaskTemplates.Contains(TaskType))
    {
        return TaskTemplates[TaskType];
    }
    
    // Otherwise, create a default task data
    FCompanionTaskData TaskData;
    TaskData.TaskType = TaskType;
    TaskData.MinDuration = 30.0f;
    TaskData.MaxDuration = 60.0f;
    TaskData.BaseScore = 0.5f;
    TaskData.SearchRadius = 500.0f;
    TaskData.InteractionDistance = 200.0f;
    
    return TaskData;
}

float UCompanionTaskComponent::GetTaskCompletion() const
{
    if (TaskEndTime <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(TaskElapsedTime / TaskEndTime, 0.0f, 1.0f);
}

void UCompanionTaskComponent::CompleteTask(bool bSuccess)
{
    // Notify blackboard of task completion
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (Blackboard)
    {
        Blackboard->SetValueAsBool(FName("TaskCompleted"), true);
        Blackboard->SetValueAsBool(FName("TaskSucceeded"), bSuccess);
    }
    
    // Abort the task to clean up
    ServerAbortTask();
}

AAICompanionController* UCompanionTaskComponent::GetCompanionController() const
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        return Cast<AAICompanionController>(OwnerPawn->GetController());
    }
    return nullptr;
}

UBlackboardComponent* UCompanionTaskComponent::GetBlackboard() const
{
    AAICompanionController* CompanionController = GetCompanionController();
    if (CompanionController)
    {
        return CompanionController->GetBlackboardComponent();
    }
    return nullptr;
}

// Task implementations
void UCompanionTaskComponent::ExecuteIdleTask(float DeltaTime)
{
    // Simple idle behavior
    // For idle, we mostly just wait until task duration expires
    
    // Random head look to make idle more natural
    if (FMath::RandBool() && TaskElapsedTime > 2.0f && FMath::Rand() % 100 < 5)
    {
        UBlackboardComponent* Blackboard = GetBlackboard();
        if (Blackboard && Blackboard->GetKeyID(FName("LookAtLocation")) != FBlackboard::InvalidKey)
        {
            // Generate random location to look at
            APawn* OwnerPawn = Cast<APawn>(GetOwner());
            if (OwnerPawn)
            {
                FVector BaseLocation = OwnerPawn->GetActorLocation();
                FVector LookOffset = FVector(
                    FMath::RandRange(-500.f, 500.f),
                    FMath::RandRange(-500.f, 500.f),
                    FMath::RandRange(0.f, 200.f)
                );
                Blackboard->SetValueAsVector(FName("LookAtLocation"), BaseLocation + LookOffset);
            }
        }
    }
}

void UCompanionTaskComponent::ExecuteFollowTask(float DeltaTime)
{
    AAICompanionController* Controller = GetCompanionController();
    if (!Controller)
    {
        return;
    }
    
    // Get player location
    ACharacter* OwnerPlayer = Controller->GetOwnerPlayer();
    if (!OwnerPlayer)
    {
        return;
    }
    
    // Get pawn for position checks
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }
    
    // Check if we need to update path
    FollowData.LastPathUpdateTime += DeltaTime;
    if (FollowData.LastPathUpdateTime >= 0.5f)
    {
        FVector PlayerLocation = OwnerPlayer->GetActorLocation();
        float DistanceMoved = FVector::Distance(PlayerLocation, FollowData.LastTargetLocation);
        
        // Update path if player moved significantly
        if (DistanceMoved > 100.0f)
        {
            // Calculate follow position - maintain follow distance from target
            FVector CurrentLocation = OwnerPawn->GetActorLocation();
            FVector DirectionToTarget = (PlayerLocation - CurrentLocation).GetSafeNormal();
            float FollowDistance = CurrentTask.InteractionDistance;
            FVector DesiredFollowPosition = PlayerLocation - (DirectionToTarget * FollowDistance);

            
            // Move to the follow position
            FollowData.CurrentRequestID = Controller->MoveToLocation(
                DesiredFollowPosition, 
                50.0f,      // Acceptance radius
                true,       // Stop on overlap
                true,       // Use pathfinding
                false,      // Project destination to nav
                true,       // Can strafe
                nullptr,    // Filter class
                true        // Allow partial path
            );
            
            // Update last known position
            FollowData.LastTargetLocation = PlayerLocation;
            FollowData.LastPathUpdateTime = 0.0f;
        }
        
        // Check for "stuck" condition
        FollowData.LastStuckCheckTime += DeltaTime;
        if (FollowData.LastStuckCheckTime >= 2.0f)
        {
            FVector CurrentPawnLocation = OwnerPawn->GetActorLocation();
            float MovementAmount = FVector::Distance(CurrentPawnLocation, FollowData.LastCheckedPawnLocation);
            
            // If barely moved and not at destination, we might be stuck
            if (MovementAmount < 50.0f && FVector::Distance(CurrentPawnLocation, FollowData.LastTargetLocation) > 200.0f)
            {
                // Try a more direct approach when stuck
                FollowData.FailedPathCount++;
                
                if (FollowData.FailedPathCount > 3)
                {
                    // After multiple failures, try direct movement
                    FollowData.bHasDirectPath = true;
                    FollowData.CurrentRequestID = Controller->MoveToLocation(
                        FollowData.LastTargetLocation,
                        100.0f,     // More lenient acceptance radius
                        true,       // Stop on overlap
                        false,      // No pathfinding - direct
                        false,      // Project destination to nav
                        true,       // Can strafe
                        nullptr,    // Filter class
                        false       // No partial paths
                    );
                }
            }
            else
            {
                // Reset failure count if moving properly
                FollowData.FailedPathCount = 0;
            }
            
            FollowData.LastCheckedPawnLocation = CurrentPawnLocation;
            FollowData.LastStuckCheckTime = 0.0f;
        }
    }
    
    // Update blackboard with follow status
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(FName("TargetLocation"), FollowData.LastTargetLocation);
        Blackboard->SetValueAsFloat(FName("FollowDistance"), FVector::Distance(OwnerPawn->GetActorLocation(), FollowData.LastTargetLocation));
        Blackboard->SetValueAsBool(FName("HasDirectPath"), FollowData.bHasDirectPath);
    }
}

void UCompanionTaskComponent::ExecutePatrolTask(float DeltaTime)
{
    // Make sure we have patrol points
    if (PatrolData.PatrolPoints.Num() == 0)
    {
        CompleteTask(false);
        return;
    }
    
    AAICompanionController* Controller = GetCompanionController();
    if (!Controller)
    {
        return;
    }
    
    // Get the current patrol point
    FVector CurrentTarget = PatrolData.PatrolPoints[PatrolData.CurrentPointIndex];
    
    // Check if we've reached the current patrol point
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }
    
    // Distance to current target
    float DistanceToTarget = FVector::Distance(OwnerPawn->GetActorLocation(), CurrentTarget);
    
    // If we're already moving, check if we've reached the destination
    UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
    if (PathComp)
    {
        // If we've reached the point or the move was completed
        if (DistanceToTarget < 150.0f || PathComp->DidMoveReachGoal())
        {
            // Update last point arrival time
            PatrolData.LastPointArrivalTime = GetWorld()->GetTimeSeconds();
            
            // Move to next point
            MoveToNextPatrolPoint();
        }
    }
    else
    {
        // Start initial movement to first point
        MoveToPatrolPoint(PatrolData.CurrentPointIndex);
    }
    
    // Update blackboard with patrol info
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(FName("PatrolTarget"), CurrentTarget);
        Blackboard->SetValueAsInt(FName("PatrolPointIndex"), PatrolData.CurrentPointIndex);
        Blackboard->SetValueAsInt(FName("PatrolPointsTotal"), PatrolData.PatrolPoints.Num());
    }
}

void UCompanionTaskComponent::MoveToNextPatrolPoint()
{
    // Wait at patrol point for a brief period
    float WaitTime = FMath::RandRange(1.0f, 3.0f);
    if (GetWorld()->GetTimeSeconds() - PatrolData.LastPointArrivalTime < WaitTime)
    {
        return; // Still waiting at current point
    }
    
    // Update patrol point index based on direction
    if (PatrolData.bBidirectionalPatrol)
    {
        // Bidirectional patrol (back and forth)
        PatrolData.CurrentPointIndex += PatrolData.PatrolDirection;
        
        // If we reached an end, reverse direction
        if (PatrolData.CurrentPointIndex >= PatrolData.PatrolPoints.Num() || 
            PatrolData.CurrentPointIndex < 0)
        {
            PatrolData.PatrolDirection *= -1;
            PatrolData.CurrentPointIndex += PatrolData.PatrolDirection * 2; // Adjust to valid index
        }
    }
    else
    {
        // Circular patrol (loop around)
        PatrolData.CurrentPointIndex = (PatrolData.CurrentPointIndex + 1) % PatrolData.PatrolPoints.Num();
    }
    
    // Move to the next point
    MoveToPatrolPoint(PatrolData.CurrentPointIndex);
}

void UCompanionTaskComponent::MoveToPatrolPoint(int32 PointIndex)
{
    if (PointIndex < 0 || PointIndex >= PatrolData.PatrolPoints.Num())
    {
        return;
    }
    
    AAICompanionController* Controller = GetCompanionController();
    if (!Controller)
    {
        return;
    }
    
    // Move to the specified patrol point
    FVector TargetPoint = PatrolData.PatrolPoints[PointIndex];
    PatrolData.PatrolRequestID = Controller->MoveToLocation(
        TargetPoint,
        100.0f,     // Acceptance radius
        true,       // Stop on overlap
        true,       // Use pathfinding
        false,      // Project destination to nav
        true,       // Can strafe
        nullptr,    // Filter class
        true        // Allow partial path
    );
}

void UCompanionTaskComponent::ExecuteGatherTask(float DeltaTime)
{
    AAICompanionController* Controller = GetCompanionController();
    if (!Controller)
    {
        return;
    }
    
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (!Blackboard)
    {
        return;
    }
    
    // Get resource location from blackboard if available
    const FName ResourceLocationKey = FName("ResourceLocation");
    if (Blackboard->GetKeyID(ResourceLocationKey) != FBlackboard::InvalidKey)
    {
        FVector ResourceLocation = Blackboard->GetValueAsVector(ResourceLocationKey);
        
        // If we have a valid resource location
        if (!ResourceLocation.IsZero())
        {
            // If we have a new target
            if (FVector::DistSquared(ResourceLocation, GatherData.CurrentTargetLocation) > 100.0f)
            {
                GatherData.CurrentTargetLocation = ResourceLocation;
                
                // Move to the resource location
                Controller->MoveToLocation(
                    ResourceLocation,
                    CurrentTask.InteractionDistance * 0.5f, // Get closer for gathering
                    true,       // Stop on overlap
                    true,       // Use pathfinding
                    false,      // Project destination to nav
                    true,       // Can strafe
                    nullptr,    // Filter class
                    true        // Allow partial path
                );
                
                GatherData.bFoundResource = true;
            }
            
            // Check if we're close enough to gather
            APawn* OwnerPawn = Cast<APawn>(GetOwner());
            if (OwnerPawn)
            {
                float DistanceToResource = FVector::Distance(OwnerPawn->GetActorLocation(), ResourceLocation);
                
                // If within gathering range and cooldown expired
                if (DistanceToResource <= CurrentTask.InteractionDistance && 
                    GatherData.GatherCooldownRemaining <= 0.0f)
                {
                    // Gather the resource
                    GatherResource();
                    
                    // Set cooldown
                    GatherData.GatherCooldownRemaining = 3.0f;
                }
            }
        }
    }
    
    // Update gather cooldown
    if (GatherData.GatherCooldownRemaining > 0.0f)
    {
        GatherData.GatherCooldownRemaining -= DeltaTime;
    }
    
    // Check if inventory is full
    const FName InventorySpaceKey = FName("InventorySpace");
    const FName ResourceAmountKey = FName("ResourceAmount");
    if (Blackboard->GetKeyID(InventorySpaceKey) != FBlackboard::InvalidKey && 
        Blackboard->GetKeyID(ResourceAmountKey) != FBlackboard::InvalidKey)
    {
        int32 InventorySpace = Blackboard->GetValueAsInt(InventorySpaceKey);
        int32 ResourceAmount = Blackboard->GetValueAsInt(ResourceAmountKey);
        
        if (ResourceAmount >= InventorySpace)
        {
            // Inventory full, complete task
            CompleteTask(true);
        }
    }
    
    // If we couldn't find resources for too long, complete the task
    if (!GatherData.bFoundResource && TaskElapsedTime > 20.0f)
    {
        CompleteTask(false);
    }
}

void UCompanionTaskComponent::GatherResource()
{
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (!Blackboard)
    {
        return;
    }
    
    // Increase resource amount in blackboard
    const FName ResourceAmountKey = FName("ResourceAmount");
    if (Blackboard->GetKeyID(ResourceAmountKey) != FBlackboard::InvalidKey)
    {
        int32 CurrentAmount = Blackboard->GetValueAsInt(ResourceAmountKey);
        Blackboard->SetValueAsInt(ResourceAmountKey, CurrentAmount + 1);
        
        // Update gather data
        GatherData.TotalGathered++;
        GatherData.LastGatherTime = GetWorld()->GetTimeSeconds();
        GatherData.LastGatherLocation = GatherData.CurrentTargetLocation;
        
        // Play gather effects if needed
        // ...
    }
}

void UCompanionTaskComponent::ExecuteSearchTask(float DeltaTime)
{
    AAICompanionController* Controller = GetCompanionController();
    if (!Controller)
    {
        return;
    }
    
    UBlackboardComponent* Blackboard = GetBlackboard();
    if (!Blackboard)
    {
        return;
    }
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }
    
    // Search behavior - move between different points in the search area
    
    // Check if we've reached current search point
    float DistanceToSearchPoint = FVector::Distance(OwnerPawn->GetActorLocation(), SearchData.CurrentSearchPoint);
    UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
    
    if ((PathComp && PathComp->DidMoveReachGoal()) || DistanceToSearchPoint < 100.0f)
    {
        // We've reached the current search point
        // Spend some time investigating this point
        SearchData.TimeAtCurrentPoint += DeltaTime;
        
        // After investigating for a short time, find a new point
        if (SearchData.TimeAtCurrentPoint >= 3.0f)
        {
            FindNewSearchPoint();
        }
    }
    else if (SearchData.CurrentSearchPoint.IsZero())
    {
        // No current search point, find one
        FindNewSearchPoint();
    }
    
    // Check if we've found an item of interest
    const FName ItemOfInterestFoundKey = FName("IsItemOfInterestFound");
    if (Blackboard->GetKeyID(ItemOfInterestFoundKey) != FBlackboard::InvalidKey)
    {
        bool bItemFound = Blackboard->GetValueAsBool(ItemOfInterestFoundKey);
        if (bItemFound)
        {
            SearchData.bFoundItemOfInterest = true;
            
            // Complete search task early if we found something
            CompleteTask(true);
        }
    }
}

void UCompanionTaskComponent::FindNewSearchPoint()
{
    AAICompanionController* Controller = GetCompanionController();
    if (!Controller)
    {
        return;
    }
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }
    
    // Reset time at point
    SearchData.TimeAtCurrentPoint = 0.0f;
    
    // Generate a new search point in the area
    FVector Origin = SearchData.SearchOrigin;
    float SearchRadius = CurrentTask.SearchRadius;
    
    // Get navigation system
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        return;
    }
    
    // Try to find a valid point
    bool bFoundPoint = false;
    FNavLocation ResultLocation;
    
    // Try multiple times
    for (int32 Attempt = 0; Attempt < MaxSearchAttempts; ++Attempt)
    {
        if (NavSystem->GetRandomReachablePointInRadius(Origin, SearchRadius, ResultLocation))
        {
            // Check if we've already investigated this point
            bool bAlreadyVisited = false;
            for (const FVector& VisitedPoint : SearchData.InvestigatedPoints)
            {
                if (FVector::DistSquared(ResultLocation.Location, VisitedPoint) < 250000.0f) // 500^2
                {
                    bAlreadyVisited = true;
                    break;
                }
            }
            
            if (!bAlreadyVisited)
            {
                bFoundPoint = true;
                SearchData.CurrentSearchPoint = ResultLocation.Location;
                SearchData.InvestigatedPoints.Add(ResultLocation.Location);
                break;
            }
        }
    }
    
    // If we found a valid point, move there
    if (bFoundPoint)
    {
        Controller->MoveToLocation(
            SearchData.CurrentSearchPoint,
            100.0f,     // Acceptance radius
            true,       // Stop on overlap
            true,       // Use pathfinding
            false,      // Project destination to nav
            true,       // Can strafe
            nullptr,    // Filter class
            true        // Allow partial path
        );
    }
    else
    {
        // If we can't find a new point, the search area might be exhausted
        CompleteTask(true);
    }
}

void UCompanionTaskComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Register properties for replication
    DOREPLIFETIME(UCompanionTaskComponent, CurrentTask);
}