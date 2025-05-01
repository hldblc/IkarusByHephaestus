// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "LoyaltyComponent.generated.h"

// Enum for mood types
UENUM(BlueprintType, Blueprintable, meta = (DisplayName = "Loyalty Component"))
enum class EMoodType : uint8
{
    Neutral UMETA(DisplayName = "Neutral"),
    Happy UMETA(DisplayName = "Happy"),
    Scared UMETA(DisplayName = "Scared"),
    Curious UMETA(DisplayName = "Curious"),
    Tired UMETA(DisplayName = "Tired"),
    Aggressive UMETA(DisplayName = "Aggressive")
};

// Struct to store mood modifiers for behaviors
USTRUCT(BlueprintType)
struct FMoodBehaviorModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood")
    float IdleModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood")
    float FollowModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood")
    float PatrolModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood")
    float GatherModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood")
    float CombatModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood")
    float FleeModifier = 1.0f;
};

// Struct to store event impact on mood and loyalty
USTRUCT(BlueprintType)
struct FInteractionImpact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Interaction")
    float LoyaltyChange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Interaction")
    float MoodIntensityChange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Interaction")
    EMoodType ResultingMood = EMoodType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Interaction")
    bool bOverrideMood = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class IKARUSTHECOMPANION_API ULoyaltyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    ULoyaltyComponent();

    // Initialize the component with references
    UFUNCTION(BlueprintCallable, Category = "Loyalty")
    void Initialize(UBlackboardComponent* BlackboardComp);

    // Update the blackboard with current values
    UFUNCTION(BlueprintCallable, Category = "Loyalty")
    void UpdateBlackboard(UBlackboardComponent* BlackboardComp);

    // Change the companion's mood
    UFUNCTION(BlueprintCallable, Category = "Loyalty|Mood")
    void SetMood(EMoodType NewMood, float Intensity = 1.0f);

    // Get the current mood
    UFUNCTION(BlueprintPure, Category = "Loyalty|Mood")
    EMoodType GetCurrentMood() const { return CurrentMood; }

    // Get the mood intensity
    UFUNCTION(BlueprintPure, Category = "Loyalty|Mood")
    float GetMoodIntensity() const { return MoodIntensity; }

    // Get the loyalty level
    UFUNCTION(BlueprintPure, Category = "Loyalty")
    float GetLoyaltyLevel() const { return LoyaltyLevel; }

    // Get modifier for a specific behavior based on current mood
    UFUNCTION(BlueprintPure, Category = "Loyalty|Behavior")
    float GetBehaviorModifier(FName BehaviorName) const;

    // Process an interaction with the player
    UFUNCTION(BlueprintCallable, Category = "Loyalty|Interaction")
    void ProcessInteraction(FName InteractionType);

    // Record the player helping the companion
    UFUNCTION(BlueprintCallable, Category = "Loyalty|Interaction")
    void RecordPlayerHelp(float Significance = 1.0f);

    // Record the player endangering the companion
    UFUNCTION(BlueprintCallable, Category = "Loyalty|Interaction")
    void RecordPlayerEndangerment(float Severity = 1.0f);

    // Record a gift from player to companion
    UFUNCTION(BlueprintCallable, Category = "Loyalty|Interaction")
    void RecordPlayerGift(float Value = 1.0f);

    // Update companion's needs
    UFUNCTION(BlueprintCallable, Category = "Loyalty|Needs")
    void UpdateNeeds(float DeltaTime);

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // Process mood decay over time
    void ProcessMoodDecay(float DeltaTime);

    // Process loyalty decay over time
    void ProcessLoyaltyDecay(float DeltaTime);

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Current mood state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood", meta = (AllowPrivateAccess = "true"))
    EMoodType CurrentMood = EMoodType::Neutral;

    // Intensity of the current mood (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood", meta = (AllowPrivateAccess = "true"))
    float MoodIntensity = 1.0f;

    // Current loyalty level (0.0-100.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty", meta = (AllowPrivateAccess = "true"))
    float LoyaltyLevel = 50.0f;

    // Rate at which mood returns to neutral
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood", meta = (AllowPrivateAccess = "true"))
    float MoodDecayRate = 0.05f;

    // Rate at which loyalty decays over time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty", meta = (AllowPrivateAccess = "true"))
    float LoyaltyDecayRate = 0.01f;

    // Modifiers for each mood type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Mood", meta = (AllowPrivateAccess = "true"))
    TMap<EMoodType, FMoodBehaviorModifiers> MoodModifiers;

    // Effects of different interaction types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Interaction", meta = (AllowPrivateAccess = "true"))
    TMap<FName, FInteractionImpact> InteractionEffects;

    // Needs system - simple for now
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Needs", meta = (AllowPrivateAccess = "true"))
    float RestNeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Needs", meta = (AllowPrivateAccess = "true"))
    float FoodNeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Needs", meta = (AllowPrivateAccess = "true"))
    float SocialNeed = 0.0f;

    // Memory of recent interactions
    struct FInteractionMemory
    {
        FName Type;
        float Time;
        float Impact;
    };

    // Recent memories
    TArray<FInteractionMemory> RecentInteractions;

    // Max number of interactions to remember
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loyalty|Memory", meta = (AllowPrivateAccess = "true"))
    int32 MaxInteractionMemories = 10;

    // Weak pointer to owner pawn
    TWeakObjectPtr<APawn> OwnerPawn;

    // Cached reference to player pawn
    TWeakObjectPtr<APawn> PlayerPawn;

    // Initialize mood modifiers with default values
    void InitializeDefaultMoodModifiers();

    // Initialize interaction effects with default values
    void InitializeDefaultInteractionEffects();
};