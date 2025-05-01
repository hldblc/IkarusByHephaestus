// Fill out your copyright notice in the Description page of Project Settings.

#include "CompanionCore/CoreData/CompanionTaskAsset.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"

UCompanionTaskAsset::UCompanionTaskAsset()
{
    // Default initialization
    TaskData.TaskType = ECompanionTask::None;
    TaskData.MinDuration = 10.0f;
    TaskData.MaxDuration = 30.0f;
    TaskData.BaseScore = 0.5f;
    TaskData.SearchRadius = 500.0f;
    TaskData.InteractionDistance = 200.0f;
    TaskData.MaxTargets = 1;
}

FPrimaryAssetId UCompanionTaskAsset::GetPrimaryAssetId() const
{
    // If the TaskId is valid, use it. Otherwise, fall back to the parent implementation
    if (TaskId.IsValid())
    {
        return TaskId;
    }

    // Use the standard PrimaryDataAsset implementation
    return Super::GetPrimaryAssetId();
}

float UCompanionTaskAsset::CalculateTaskUtility(UBlackboardComponent* Blackboard) const
{
    if (!Blackboard)
    {
        return 0.0f;
    }
    
    // Start with base score
    float Score = TaskData.BaseScore * UtilityScoreMultiplier;
    
    // Apply task-specific modifiers based on context
    switch (TaskData.TaskType)
    {
        case ECompanionTask::Idle:
            // Idle is more useful when character is tired
            if (Blackboard->GetValueAsFloat(FName("CurrentStamina")) < Blackboard->GetValueAsFloat(FName("MaxStamina")) * 0.5f)
            {
                Score += 0.3f;
            }
            break;
            
        case ECompanionTask::Follow:
            // Follow is more useful when far from player
            {
                float Distance = Blackboard->GetValueAsFloat(FName("OwnerDistance"));
                Score += FMath::Clamp(Distance / 1000.0f, 0.0f, 0.5f);
            }
            break;
            
        case ECompanionTask::Patrol:
            // Patrol is more useful when no threats detected
            if (!Blackboard->GetValueAsBool(FName("IsThreatDetected")))
            {
                Score += 0.2f;
            }
            break;
            
        case ECompanionTask::Gather:
            // Gather is more useful when resources detected
            if (Blackboard->GetValueAsBool(FName("IsResourceDetected")))
            {
                Score += 0.3f;
            }
            break;
            
        case ECompanionTask::Search:
            // Search is more useful in unexplored areas
            if (Blackboard->GetValueAsFloat(FName("ExplorationPercentage")) < 0.5f)
            {
                Score += 0.25f;
            }
            break;
            
        default:
            break;
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

bool UCompanionTaskAsset::AreRequirementsMet(UBlackboardComponent* Blackboard) const
{
    if (!Blackboard)
    {
        return false;
    }
    
    // Check that all required keys exist and have valid values
    for (const FName& KeyName : RequiredBlackboardKeys)
    {
        if (KeyName == NAME_None)
        {
            continue; // Skip invalid keys
        }

        FBlackboard::FKey KeyID = Blackboard->GetKeyID(KeyName);
        if (KeyID == FBlackboard::InvalidKey)
        {
            return false; // Key doesn't exist
        }
        
        // Check value based on key type
        TSubclassOf<UBlackboardKeyType> KeyType = Blackboard->GetKeyType(KeyID);
        
        if (KeyType == UBlackboardKeyType_Vector::StaticClass())
        {
            if (!Blackboard->IsVectorValueSet(KeyName))
            {
                return false;
            }
        }
        else if (KeyType == UBlackboardKeyType_Object::StaticClass())
        {
            if (Blackboard->GetValueAsObject(KeyName) == nullptr)
            {
                return false;
            }
        }
    }
    
    // Check task data requirements from the component
    for (const FBlackboardKeySelector& KeySelector : TaskData.RequiredKeys)
    {
        const FName KeyName = KeySelector.SelectedKeyName;
        if (KeyName == NAME_None)
        {
            continue;
        }
        
        if (Blackboard->GetKeyID(KeyName) == FBlackboard::InvalidKey)
        {
            return false;
        }
    }
    
    return true;
}