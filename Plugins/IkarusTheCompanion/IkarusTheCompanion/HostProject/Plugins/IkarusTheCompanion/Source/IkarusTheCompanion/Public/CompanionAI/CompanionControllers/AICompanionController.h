// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AICompanionController.generated.h"

class UBehaviorTreeComponent;
class UCompanionTaskComponent;

/**
 * Advanced AI controller for companion characters in multiplayer medieval survival game.
 * Handles perception, behavior trees, and owner relationships with optimized networking support.
 */
UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="AI Companion Controller"))
class IKARUSTHECOMPANION_API AAICompanionController : public AAIController
{
	GENERATED_BODY()

public:
	/** Constructor that sets up the controller components */
	AAICompanionController();

	/** Get the blackboard component */
	UFUNCTION(BlueprintCallable, Category = "AI")
	UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
	
	/** Get the behavior tree component */
	UFUNCTION(BlueprintCallable, Category = "AI")
	UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return BehaviorTreeComponent; }
	
	/** Get the companion task component (if available) */
	UFUNCTION(BlueprintCallable, Category = "AI")
	UCompanionTaskComponent* GetCompanionTaskComponent() const;
	
	/** Set the owner player for this companion (multiplayer support) */
	UFUNCTION(BlueprintCallable, Category = "AI|Multiplayer")
	void SetOwnerPlayer(ACharacter* NewOwnerPlayer);
	
	/** Get the owner player for this companion */
	UFUNCTION(BlueprintCallable, Category = "AI|Multiplayer")
	ACharacter* GetOwnerPlayer() const;

	/** Update companion's awareness of threats in the area */
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	void UpdateThreatAwareness();
	
	/** Force update all blackboard values related to owner and state */
	UFUNCTION(BlueprintCallable, Category = "AI|Multiplayer")
	void ForceUpdateBlackboardValues();

protected:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
	
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Overriding Possess to initialize AI and acquire reference to CompanionComponent */
	virtual void OnPossess(APawn* InPawn) override;
	
	/** Handle unposses for clean AI shutdown */
	virtual void OnUnPossess() override;
	
	/** Handle replication setup for multiplayer */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** Update companion perception intensity based on environment and game state */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Perception")
	void UpdatePerceptionSettings();
	virtual void UpdatePerceptionSettings_Implementation();
	
private:
	/** Behavior tree asset to run */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** Behavior tree component for executing logic */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	/** Blackboard component for storing data */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBlackboardComponent> BlackboardComponent;
	
	/** Reference to the companion's task component */
	UPROPERTY(VisibleAnywhere, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCompanionTaskComponent> CompanionTaskComponent;
	
	/** The player character that owns this companion (replicated for multiplayer) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="AI|Multiplayer", meta=(AllowPrivateAccess="true"))
	TObjectPtr<ACharacter> OwnerPlayer;
	
	/** How often (in seconds) to update blackboard values */
	UPROPERTY(EditDefaultsOnly, Category="AI|Performance", meta=(AllowPrivateAccess="true", ClampMin="0.1", ClampMax="1.0"))
	float BlackboardUpdateInterval = 0.25f;
	
	/** Timer handle for blackboard updates */
	FTimerHandle BlackboardUpdateTimerHandle;

	/** Time accumulator for optimized updates */
	float UpdateTimeAccumulator = 0.0f;
	
	/** Sight configuration for perception */
	class UAISenseConfig_Sight* SightConfig;
	
	/** Sound configuration for perception */
	class UAISenseConfig_Hearing* HearingConfig;
	
	/** Setup perception system components and configs */
	void SetupPerceptionSystem();
	
	/** Handler for when target is detected by AI perception */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus);
	
	/** Initialize our blackboard with companion-specific values */
	void SetupCompanionBlackboardValues();
	
	/** Blackboard update function for optimized performance */
	void UpdateBlackboardValues();
	
	/** Update relationship with nearby NPCs and players for social behaviors */
	void UpdateSocialAwareness();
};