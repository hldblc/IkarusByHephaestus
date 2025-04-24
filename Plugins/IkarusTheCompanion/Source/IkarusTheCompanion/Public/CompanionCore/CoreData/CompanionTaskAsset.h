// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"
#include "CompanionCore/CoreStructs/CompanionTaskData.h"
#include "GameplayTagContainer.h"
#include "CompanionTaskAsset.generated.h"

/**
 * Data asset that defines a companion task configuration.
 * These assets can be loaded from the Data Registry and referenced by the Companion AI system.
 * All task-specific data is contained directly in the TaskData struct.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Companion Task Asset"))
class IKARUSTHECOMPANION_API UCompanionTaskAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    UCompanionTaskAsset();

    // Primary asset identifier for this task
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Task Identification")
    FPrimaryAssetId TaskId;

    // Primary configuration data for this task
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Configuration", meta=(ShowOnlyInnerProperties))
    FCompanionTaskData TaskData;
    
    // Custom scoring modifiers for task utility
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Utility", meta=(ClampMin="0.0", ClampMax="2.0"))
    float UtilityScoreMultiplier = 1.0f;
    
    // Optional task-specific blackboard keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Task Requirements")
    TArray<FName> RequiredBlackboardKeys;
    
    // Description for designers
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Task Documentation", meta=(MultiLine=true))
    FText Description;

    // Optional developer notes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Task Documentation", meta=(MultiLine=true))
    FText DeveloperNotes;
    
    // Version tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Development")
    int32 Version = 1;
    
    // Optional icon to represent this task in UI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    UTexture2D* TaskIcon;

    // Override to provide custom asset identifier
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    
    // Returns task type of this asset
    UFUNCTION(BlueprintCallable, Category="Companion")
    ECompanionTask GetTaskType() const { return TaskData.TaskType; }
    
    // Returns task tag of this asset
    UFUNCTION(BlueprintCallable, Category="Companion")
    FGameplayTag GetTaskTag() const { return TaskData.TaskTag; }
    
    // Calculate utility score for this task based on blackboard state
    UFUNCTION(BlueprintCallable, Category="Companion")
    float CalculateTaskUtility(class UBlackboardComponent* Blackboard) const;
    
    // Check if task requirements are met
    UFUNCTION(BlueprintCallable, Category="Companion")
    bool AreRequirementsMet(class UBlackboardComponent* Blackboard) const;
};