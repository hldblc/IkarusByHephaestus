// Fill out your copyright notice in the Description page of Project Settings.

#include "CompanionAI/Components/LoyaltyComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values for this component's properties
ULoyaltyComponent::ULoyaltyComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize default values for mood modifiers
    InitializeDefaultMoodModifiers();
    
    // Initialize default interaction effects
    InitializeDefaultInteractionEffects();
}

// Called when the game starts
void ULoyaltyComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner pawn
    OwnerPawn = Cast<APawn>(GetOwner());
    
    // Get player pawn
    if (UWorld* World = GetWorld())
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    }
}

// Called every frame
void ULoyaltyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Process mood decay
    ProcessMoodDecay(DeltaTime);
    
    // Process loyalty decay
    ProcessLoyaltyDecay(DeltaTime);
    
    // Update needs system
    UpdateNeeds(DeltaTime);
}

void ULoyaltyComponent::Initialize(UBlackboardComponent* BlackboardComp)
{
    if (!BlackboardComp)
        return;
    
    // Set initial values in blackboard
    BlackboardComp->SetValueAsEnum("CurrentMood", (uint8)CurrentMood);
    BlackboardComp->SetValueAsFloat("MoodIntensity", MoodIntensity);
    BlackboardComp->SetValueAsFloat("LoyaltyLevel", LoyaltyLevel);
    
    // Set initial needs
    BlackboardComp->SetValueAsFloat("RestNeed", RestNeed);
    BlackboardComp->SetValueAsFloat("FoodNeed", FoodNeed);
    BlackboardComp->SetValueAsFloat("SocialNeed", SocialNeed);
}

void ULoyaltyComponent::UpdateBlackboard(UBlackboardComponent* BlackboardComp)
{
    if (!BlackboardComp)
        return;
    
    // Update mood and loyalty values
    BlackboardComp->SetValueAsEnum("CurrentMood", (uint8)CurrentMood);
    BlackboardComp->SetValueAsFloat("MoodIntensity", MoodIntensity);
    BlackboardComp->SetValueAsFloat("LoyaltyLevel", LoyaltyLevel);
    
    // Update behavior modifiers based on current mood
    BlackboardComp->SetValueAsFloat("IdleModifier", GetBehaviorModifier("IdleScore"));
    BlackboardComp->SetValueAsFloat("FollowModifier", GetBehaviorModifier("FollowScore"));
    BlackboardComp->SetValueAsFloat("PatrolModifier", GetBehaviorModifier("PatrolScore"));
    BlackboardComp->SetValueAsFloat("GatherModifier", GetBehaviorModifier("GatherScore"));
    BlackboardComp->SetValueAsFloat("CombatModifier", GetBehaviorModifier("CombatScore"));
    BlackboardComp->SetValueAsFloat("FleeModifier", GetBehaviorModifier("FleeScore"));
    
    // Update need values
    BlackboardComp->SetValueAsFloat("RestNeed", RestNeed);
    BlackboardComp->SetValueAsFloat("FoodNeed", FoodNeed);
    BlackboardComp->SetValueAsFloat("SocialNeed", SocialNeed);
    
    // Set dominant need if any need is high
    FName DominantNeed = NAME_None;
    float HighestNeedValue = 0.0f;
    
    if (RestNeed > HighestNeedValue && RestNeed > 0.7f)
    {
        HighestNeedValue = RestNeed;
        DominantNeed = "Rest";
    }
    
    if (FoodNeed > HighestNeedValue && FoodNeed > 0.7f)
    {
        HighestNeedValue = FoodNeed;
        DominantNeed = "Food";
    }
    
    if (SocialNeed > HighestNeedValue && SocialNeed > 0.7f)
    {
        HighestNeedValue = SocialNeed;
        DominantNeed = "Social";
    }
    
    BlackboardComp->SetValueAsName("DominantNeed", DominantNeed);
    BlackboardComp->SetValueAsBool("HasDominantNeed", DominantNeed != NAME_None);
}

void ULoyaltyComponent::SetMood(EMoodType NewMood, float Intensity)
{
    // Set new mood
    CurrentMood = NewMood;
    MoodIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Log mood change
    UE_LOG(LogTemp, Display, TEXT("Companion mood changed to %s (Intensity: %.2f)"), 
        *UEnum::GetValueAsString(CurrentMood), MoodIntensity);
    
    // Visual feedback could be added here
    
    // Record interaction
    FInteractionMemory Memory;
    Memory.Type = "MoodChange";
    Memory.Time = GetWorld()->GetTimeSeconds();
    Memory.Impact = 0.0f;
    
    RecentInteractions.Add(Memory);
    
    // Limit memory size
    if (RecentInteractions.Num() > MaxInteractionMemories)
    {
        RecentInteractions.RemoveAt(0);
    }
}

float ULoyaltyComponent::GetBehaviorModifier(FName BehaviorName) const
{
    // Default modifier is 1.0 (no change)
    float Modifier = 1.0f;
    
    // Get mood modifiers for current mood
    if (MoodModifiers.Contains(CurrentMood))
    {
        const FMoodBehaviorModifiers& MoodMods = MoodModifiers[CurrentMood];
        
        // Apply appropriate modifier based on behavior name
        if (BehaviorName == "IdleScore") Modifier = MoodMods.IdleModifier;
        else if (BehaviorName == "FollowScore") Modifier = MoodMods.FollowModifier;
        else if (BehaviorName == "PatrolScore") Modifier = MoodMods.PatrolModifier;
        else if (BehaviorName == "GatherScore") Modifier = MoodMods.GatherModifier;
        else if (BehaviorName == "CombatScore") Modifier = MoodMods.CombatModifier;
        else if (BehaviorName == "FleeScore") Modifier = MoodMods.FleeModifier;
    }
    
    // Scale by mood intensity
    Modifier = 1.0f + ((Modifier - 1.0f) * MoodIntensity);
    
    // Modify based on loyalty level
    if (BehaviorName == "FollowScore")
    {
        // Increase follow score with higher loyalty
        Modifier *= (0.5f + (LoyaltyLevel / 100.0f) * 0.5f);
    }
    
    return Modifier;
}

void ULoyaltyComponent::ProcessInteraction(FName InteractionType)
{
    // Find impact for this interaction type
    if (InteractionEffects.Contains(InteractionType))
    {
        const FInteractionImpact& Impact = InteractionEffects[InteractionType];
        
        // Apply loyalty change
        LoyaltyLevel = FMath::Clamp(LoyaltyLevel + Impact.LoyaltyChange, 0.0f, 100.0f);
        
        // Apply mood change if needed
        if (Impact.bOverrideMood)
        {
            CurrentMood = Impact.ResultingMood;
        }
        
        // Apply mood intensity change
        MoodIntensity = FMath::Clamp(MoodIntensity + Impact.MoodIntensityChange, 0.0f, 1.0f);
        
        // Record interaction
        FInteractionMemory Memory;
        Memory.Type = InteractionType;
        Memory.Time = GetWorld()->GetTimeSeconds();
        Memory.Impact = Impact.LoyaltyChange;
        
        RecentInteractions.Add(Memory);
        
        // Limit memory size
        if (RecentInteractions.Num() > MaxInteractionMemories)
        {
            RecentInteractions.RemoveAt(0);
        }
        
        // Log interaction
        UE_LOG(LogTemp, Display, TEXT("Companion interaction: %s (Loyalty: %.1f, Mood: %s, Intensity: %.2f)"), 
            *InteractionType.ToString(), LoyaltyLevel, *UEnum::GetValueAsString(CurrentMood), MoodIntensity);
    }
}

void ULoyaltyComponent::RecordPlayerHelp(float Significance)
{
    // Calculate loyalty boost based on significance
    float LoyaltyBoost = Significance * 5.0f;
    
    // Apply loyalty change
    LoyaltyLevel = FMath::Clamp(LoyaltyLevel + LoyaltyBoost, 0.0f, 100.0f);
    
    // Create a positive mood
    if (Significance >= 0.5f)
    {
        CurrentMood = EMoodType::Happy;
        MoodIntensity = FMath::Clamp(MoodIntensity + (Significance * 0.5f), 0.0f, 1.0f);
    }
    
    // Record interaction
    FInteractionMemory Memory;
    Memory.Type = "PlayerHelp";
    Memory.Time = GetWorld()->GetTimeSeconds();
    Memory.Impact = LoyaltyBoost;
    
    RecentInteractions.Add(Memory);
    
    // Limit memory size
    if (RecentInteractions.Num() > MaxInteractionMemories)
    {
        RecentInteractions.RemoveAt(0);
    }
    
    // Log interaction
    UE_LOG(LogTemp, Display, TEXT("Player helped companion (Significance: %.2f, Loyalty: %.1f)"), 
        Significance, LoyaltyLevel);
}

void ULoyaltyComponent::RecordPlayerEndangerment(float Severity)
{
    // Calculate loyalty reduction based on severity
    float LoyaltyReduction = Severity * 10.0f;
    
    // Apply loyalty change
    LoyaltyLevel = FMath::Clamp(LoyaltyLevel - LoyaltyReduction, 0.0f, 100.0f);
    
    // Create a negative mood
    if (Severity >= 0.3f)
    {
        if (FMath::RandBool()) // Randomly pick between scared and aggressive
        {
            CurrentMood = EMoodType::Scared;
        }
        else
        {
            CurrentMood = EMoodType::Aggressive;
        }
        
        MoodIntensity = FMath::Clamp(Severity * 2.0f, 0.0f, 1.0f);
    }
    
    // Record interaction
    FInteractionMemory Memory;
    Memory.Type = "PlayerEndangerment";
    Memory.Time = GetWorld()->GetTimeSeconds();
    Memory.Impact = -LoyaltyReduction;
    
    RecentInteractions.Add(Memory);
    
    // Limit memory size
    if (RecentInteractions.Num() > MaxInteractionMemories)
    {
        RecentInteractions.RemoveAt(0);
    }
    
    // Log interaction
    UE_LOG(LogTemp, Display, TEXT("Player endangered companion (Severity: %.2f, Loyalty: %.1f)"), 
        Severity, LoyaltyLevel);
}

void ULoyaltyComponent::RecordPlayerGift(float Value)
{
    // Calculate loyalty boost based on value
    float LoyaltyBoost = Value * 3.0f;
    
    // Apply loyalty change
    LoyaltyLevel = FMath::Clamp(LoyaltyLevel + LoyaltyBoost, 0.0f, 100.0f);
    
    // Create a happy mood
    CurrentMood = EMoodType::Happy;
    MoodIntensity = FMath::Clamp(Value * 1.5f, 0.0f, 1.0f);
    
    // Record interaction
    FInteractionMemory Memory;
    Memory.Type = "PlayerGift";
    Memory.Time = GetWorld()->GetTimeSeconds();
    Memory.Impact = LoyaltyBoost;
    
    RecentInteractions.Add(Memory);
    
    // Limit memory size
    if (RecentInteractions.Num() > MaxInteractionMemories)
    {
        RecentInteractions.RemoveAt(0);
    }
    
    // Log interaction
    UE_LOG(LogTemp, Display, TEXT("Player gave gift to companion (Value: %.2f, Loyalty: %.1f)"), 
        Value, LoyaltyLevel);
}

void ULoyaltyComponent::ProcessMoodDecay(float DeltaTime)
{
    // Only apply decay if not in neutral mood or intensity isn't already minimal
    if (CurrentMood != EMoodType::Neutral || MoodIntensity > 0.1f)
    {
        // Reduce mood intensity over time
        MoodIntensity = FMath::Max(0.1f, MoodIntensity - (MoodDecayRate * DeltaTime));
        
        // If intensity gets low enough, revert to neutral mood
        if (MoodIntensity <= 0.2f && CurrentMood != EMoodType::Neutral)
        {
            CurrentMood = EMoodType::Neutral;
            MoodIntensity = 0.1f;
            
            // Log mood change
            UE_LOG(LogTemp, Verbose, TEXT("Companion mood reverted to Neutral"));
        }
    }
}

void ULoyaltyComponent::ProcessLoyaltyDecay(float DeltaTime)
{
    // Only apply decay if loyalty is above minimum threshold
    if (LoyaltyLevel > 30.0f)
    {
        // Reduce loyalty slightly over time (very slow decay)
        LoyaltyLevel = FMath::Max(30.0f, LoyaltyLevel - (LoyaltyDecayRate * DeltaTime));
    }
}

void ULoyaltyComponent::UpdateNeeds(float DeltaTime)
{
    // Increase needs over time
    RestNeed = FMath::Min(1.0f, RestNeed + (0.01f * DeltaTime));
    FoodNeed = FMath::Min(1.0f, FoodNeed + (0.005f * DeltaTime));
    SocialNeed = FMath::Min(1.0f, SocialNeed + (0.003f * DeltaTime));
    
    // Check if companion is near player
    bool bNearPlayer = false;
    
    if (OwnerPawn.IsValid() && PlayerPawn.IsValid())
    {
        float Distance = FVector::Distance(
            OwnerPawn->GetActorLocation(), 
            PlayerPawn->GetActorLocation()
        );
        
        bNearPlayer = Distance < 500.0f;
    }
    
    // Being near player satisfies social need
    if (bNearPlayer)
    {
        SocialNeed = FMath::Max(0.0f, SocialNeed - (0.05f * DeltaTime));
    }
    
    // High needs can affect mood
    if (RestNeed > 0.8f && CurrentMood != EMoodType::Tired)
    {
        SetMood(EMoodType::Tired, FMath::Min(1.0f, RestNeed));
    }
    
    if (FoodNeed > 0.9f)
    {
        // Very hungry companion gets irritable
        SetMood(EMoodType::Aggressive, FMath::Min(1.0f, FoodNeed - 0.5f));
    }
    
    if (SocialNeed > 0.85f && bNearPlayer)
    {
        // Very social need + near player = happy to see player
        SetMood(EMoodType::Happy, FMath::Min(1.0f, SocialNeed));
        
        // Being near player after long absence boosts loyalty
        LoyaltyLevel = FMath::Min(100.0f, LoyaltyLevel + (SocialNeed * 0.5f));
        
        // Reset social need when reunited
        SocialNeed = 0.0f;
    }
}

void ULoyaltyComponent::InitializeDefaultMoodModifiers()
{
    // Create modifiers for each mood type
    
    // Neutral mood - baseline
    FMoodBehaviorModifiers NeutralMods;
    NeutralMods.IdleModifier = 1.0f;
    NeutralMods.FollowModifier = 1.0f;
    NeutralMods.PatrolModifier = 1.0f;
    NeutralMods.GatherModifier = 1.0f;
    NeutralMods.CombatModifier = 1.0f;
    NeutralMods.FleeModifier = 1.0f;
    MoodModifiers.Add(EMoodType::Neutral, NeutralMods);
    
    // Happy mood - more active, social
    FMoodBehaviorModifiers HappyMods;
    HappyMods.IdleModifier = 0.8f;
    HappyMods.FollowModifier = 1.2f;
    HappyMods.PatrolModifier = 1.1f;
    HappyMods.GatherModifier = 1.3f;
    HappyMods.CombatModifier = 1.0f;
    HappyMods.FleeModifier = 0.7f;
    MoodModifiers.Add(EMoodType::Happy, HappyMods);
    
    // Scared mood - more cautious, stays close to player
    FMoodBehaviorModifiers ScaredMods;
    ScaredMods.IdleModifier = 0.6f;
    ScaredMods.FollowModifier = 1.4f;
    ScaredMods.PatrolModifier = 0.5f;
    ScaredMods.GatherModifier = 0.6f;
    ScaredMods.CombatModifier = 0.4f;
    ScaredMods.FleeModifier = 2.0f;
    MoodModifiers.Add(EMoodType::Scared, ScaredMods);
    
    // Curious mood - more exploration
    FMoodBehaviorModifiers CuriousMods;
    CuriousMods.IdleModifier = 0.7f;
    CuriousMods.FollowModifier = 0.9f;
    CuriousMods.PatrolModifier = 1.5f;
    CuriousMods.GatherModifier = 1.2f;
    CuriousMods.CombatModifier = 0.8f;
    CuriousMods.FleeModifier = 0.9f;
    MoodModifiers.Add(EMoodType::Curious, CuriousMods);
    
    // Tired mood - more idle, less active
    FMoodBehaviorModifiers TiredMods;
    TiredMods.IdleModifier = 1.5f;
    TiredMods.FollowModifier = 0.8f;
    TiredMods.PatrolModifier = 0.6f;
    TiredMods.GatherModifier = 0.7f;
    TiredMods.CombatModifier = 0.5f;
    TiredMods.FleeModifier = 0.9f;
    MoodModifiers.Add(EMoodType::Tired, TiredMods);
    
    // Aggressive mood - more combat, less passive
    FMoodBehaviorModifiers AggressiveMods;
    AggressiveMods.IdleModifier = 0.6f;
    AggressiveMods.FollowModifier = 0.8f;
    AggressiveMods.PatrolModifier = 1.1f;
    AggressiveMods.GatherModifier = 0.7f;
    AggressiveMods.CombatModifier = 1.8f;
    AggressiveMods.FleeModifier = 0.5f;
    MoodModifiers.Add(EMoodType::Aggressive, AggressiveMods);
}

void ULoyaltyComponent::InitializeDefaultInteractionEffects()
{
    // Initialize common interaction types
    
    // Player gives food
    FInteractionImpact FoodGift;
    FoodGift.LoyaltyChange = 2.0f;
    FoodGift.MoodIntensityChange = 0.2f;
    FoodGift.ResultingMood = EMoodType::Happy;
    FoodGift.bOverrideMood = true;
    InteractionEffects.Add("GiveFood", FoodGift);
    
    // Player petpets companion
    FInteractionImpact PetPet;
    PetPet.LoyaltyChange = 1.0f;
    PetPet.MoodIntensityChange = 0.1f;
    PetPet.ResultingMood = EMoodType::Happy;
    PetPet.bOverrideMood = true;
    InteractionEffects.Add("PetCompanion", PetPet);
    
    // Player scolds companion
    FInteractionImpact Scold;
    Scold.LoyaltyChange = -1.5f;
    Scold.MoodIntensityChange = 0.3f;
    Scold.ResultingMood = EMoodType::Scared;
    Scold.bOverrideMood = true;
    InteractionEffects.Add("ScoldCompanion", Scold);
    
    // Player plays with companion
    FInteractionImpact Play;
    Play.LoyaltyChange = 3.0f;
    Play.MoodIntensityChange = 0.4f;
    Play.ResultingMood = EMoodType::Happy;
    Play.bOverrideMood = true;
    InteractionEffects.Add("PlayWithCompanion", Play);
    
    // Player ignores companion for long time
    FInteractionImpact Ignore;
    Ignore.LoyaltyChange = -5.0f;
    Ignore.MoodIntensityChange = 0.3f;
    Ignore.ResultingMood = EMoodType::Curious;
    Ignore.bOverrideMood = true;
    InteractionEffects.Add("IgnoreCompanion", Ignore);
    
    // Player commands companion to rest
    FInteractionImpact Rest;
    Rest.LoyaltyChange = 0.5f;
    Rest.MoodIntensityChange = -0.2f;
    Rest.ResultingMood = EMoodType::Tired;
    Rest.bOverrideMood = true;
    InteractionEffects.Add("CommandRest", Rest);
    
    // Player finds treasure with companion
    FInteractionImpact FindTreasure;
    FindTreasure.LoyaltyChange = 4.0f;
    FindTreasure.MoodIntensityChange = 0.5f;
    FindTreasure.ResultingMood = EMoodType::Happy;
    FindTreasure.bOverrideMood = true;
    InteractionEffects.Add("FindTreasure", FindTreasure);
    
    // Player defeats enemy with companion
    FInteractionImpact DefeatEnemy;
    DefeatEnemy.LoyaltyChange = 2.5f;
    DefeatEnemy.MoodIntensityChange = 0.3f;
    DefeatEnemy.ResultingMood = EMoodType::Happy;
    DefeatEnemy.bOverrideMood = true;
    InteractionEffects.Add("DefeatEnemy", DefeatEnemy);
    
    // Enemy attacks companion
    FInteractionImpact EnemyAttack;
    EnemyAttack.LoyaltyChange = 0.0f;
    EnemyAttack.MoodIntensityChange = 0.6f;
    EnemyAttack.ResultingMood = EMoodType::Aggressive;
    EnemyAttack.bOverrideMood = true;
    InteractionEffects.Add("EnemyAttack", EnemyAttack);
    
    // Player discovers new area with companion
    FInteractionImpact Discover;
    Discover.LoyaltyChange = 1.0f;
    Discover.MoodIntensityChange = 0.4f;
    Discover.ResultingMood = EMoodType::Curious;
    Discover.bOverrideMood = true;
    InteractionEffects.Add("DiscoverArea", Discover);
    
    // Player heals companion
    FInteractionImpact Heal;
    Heal.LoyaltyChange = 5.0f;
    Heal.MoodIntensityChange = 0.3f;
    Heal.ResultingMood = EMoodType::Happy;
    Heal.bOverrideMood = true;
    InteractionEffects.Add("HealCompanion", Heal);
}