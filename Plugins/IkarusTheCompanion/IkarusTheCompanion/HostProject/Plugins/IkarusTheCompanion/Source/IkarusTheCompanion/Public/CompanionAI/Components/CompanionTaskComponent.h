// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CompanionCore/CoreStructs/CompanionTaskData.h"
#include "CompanionTaskComponent.generated.h"

/**
 * Component responsible for managing companion AI tasks
 * Handles task execution, duration tracking, and blackboard integration
 * Provides a clean interface for behavior trees to use for complex companion behaviors
 */
UCLASS(ClassGroup = (Companion), meta = (BlueprintSpawnableComponent))
class IKARUSTHECOMPANION_API UCompanionTaskComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /** Constructor with default initialization */
    UCompanionTaskComponent();

    /** Called every frame */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Current active task data */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Task")
    FCompanionTaskData CurrentTask;

    /** Start a new task (Server RPC) */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Task")
    void ServerStartTask(const FCompanionTaskData& TaskData);

    /** Abort the current task (Server RPC) */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Task")
    void ServerAbortTask();

    /** Core tick function for the active task */
    UFUNCTION(BlueprintCallable, Category = "Task")
    void TickTask(float DeltaTime);

    /** Validate blackboard requirements for a task */
    UFUNCTION(BlueprintCallable, Category = "Task")
    bool ValidateBlackboardRequirements(const FCompanionTaskData& TaskData);
    
    /** Calculate utility score for a specific task type */
    UFUNCTION(BlueprintCallable, Category = "Task")
    float CalculateTaskUtility(ECompanionTask TaskType);
    
    /** Get the task data for the specified task type */
    UFUNCTION(BlueprintCallable, Category = "Task")
    FCompanionTaskData GetTaskDataForType(ECompanionTask TaskType);
    
    /** Check if companion is currently executing a task */
    UFUNCTION(BlueprintPure, Category = "Task")
    bool IsExecutingTask() const { return CurrentTask.TaskType != ECompanionTask::None; }
    
    /** Get the current task type */
    UFUNCTION(BlueprintPure, Category = "Task")
    ECompanionTask GetCurrentTaskType() const { return CurrentTask.TaskType; }
    
    /** Get the time remaining for current task */
    UFUNCTION(BlueprintPure, Category = "Task")
    float GetTaskTimeRemaining() const { return FMath::Max(0.0f, TaskEndTime - TaskElapsedTime); }
    
    /** Get the completion percentage of current task (0.0-1.0) */
    UFUNCTION(BlueprintPure, Category = "Task")
    float GetTaskCompletion() const;

protected:
    /** Called when the game starts */
    virtual void BeginPlay() override;

    /** Get the AI controller */
    UFUNCTION(BlueprintCallable, Category = "Task")
    class AAICompanionController* GetCompanionController() const;

    /** Get the blackboard component */
    UFUNCTION(BlueprintCallable, Category = "Task")
    class UBlackboardComponent* GetBlackboard() const;

    /** Task timing values */
    float TaskElapsedTime = 0.0f;
    float TaskEndTime = 0.0f;
    
    /** Task templates library - define default task parameters */
    UPROPERTY(EditDefaultsOnly, Category = "Task Templates")
    TMap<ECompanionTask, FCompanionTaskData> TaskTemplates;
    
    /** Maximum search attempts for finding task locations */
    UPROPERTY(EditDefaultsOnly, Category = "Task Settings")
    int32 MaxSearchAttempts = 5;
    
    /** Cached task data for specialized task types */
    UPROPERTY(Transient)
    FFollowTaskData FollowData;
    
    UPROPERTY(Transient)
    FPatrolTaskData PatrolData;
    
    UPROPERTY(Transient)
    FGatherTaskData GatherData;
    
    UPROPERTY(Transient)
    FSearchTaskData SearchData;

    /** Task implementation functions */
    void ExecuteIdleTask(float DeltaTime);
    void ExecuteFollowTask(float DeltaTime);
    void ExecutePatrolTask(float DeltaTime);
    void ExecuteGatherTask(float DeltaTime);
    void ExecuteSearchTask(float DeltaTime);
    
    /** Create specialized task data based on current task */
    void InitializeSpecializedTaskData();
    
    /** Update blackboard with current task state */
    void UpdateBlackboardWithTaskState();

    /** Helper functions for task execution */
    void MoveToNextPatrolPoint();
    void MoveToPatrolPoint(int32 PointIndex);
    void GatherResource();
    void FindNewSearchPoint();
    
    /** Handle task completion */
    void CompleteTask(bool bSuccess);

    /** Setup for replication */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};