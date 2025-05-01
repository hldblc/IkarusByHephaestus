// Copyright Epic Games, Inc. All Rights Reserved.

#include "IkarusTheCompanion/Public/CompanionAI/CompanionControllers/AICompanionController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Sets default values
AAICompanionController::AAICompanionController()
{
    PrimaryActorTick.bCanEverTick = true;
    bAttachToPawn = true;
    bWantsPlayerState = true; // Important for multiplayer identification
    
    // Create core components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    
    // Setup perception system
    SetupPerceptionSystem();
}

// Called when the game starts or when spawned
void AAICompanionController::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup timer for optimized blackboard updates
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BlackboardUpdateTimerHandle,
            this,
            &AAICompanionController::UpdateBlackboardValues,
            BlackboardUpdateInterval,
            true);
    }
}

void AAICompanionController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Setup replication for multiplayer
    DOREPLIFETIME(AAICompanionController, OwnerPlayer);
}

// When possessing a pawn, initialize AI and cache components
void AAICompanionController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    
    // Initialize and run behavior tree
    if (BehaviorTree && BlackboardComponent)
    {
        // Initialize blackboard with data asset
        if (BehaviorTree->BlackboardAsset && InitializeBlackboard(*BlackboardComponent, *BehaviorTree->BlackboardAsset))
        {
            // Setup companion-specific blackboard values
            SetupCompanionBlackboardValues();
            
            // Run behavior tree
            BehaviorTreeComponent->StartTree(*BehaviorTree);
            
            // Log initialization
            UE_LOG(LogTemp, Log, TEXT("AICompanionController initialized behavior tree for %s"), *GetNameSafe(InPawn));
            DebugBlackboardValues();
        }
    }
}

void AAICompanionController::OnUnPossess()
{
    // Cleanup behavior tree and timers
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BlackboardUpdateTimerHandle);
    }
    
    Super::OnUnPossess();
}

// Setup AI perception system with sight and hearing
void AAICompanionController::SetupPerceptionSystem()
{
    // Create perception component
    SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
    
    // Setup sight configuration
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 1000.f;
        SightConfig->LoseSightRadius = SightConfig->SightRadius + 50.f;
        SightConfig->PeripheralVisionAngleDegrees = 90.f;
        SightConfig->SetMaxAge(5.f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.f;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        
        GetPerceptionComponent()->ConfigureSense(*SightConfig);
    }
    
    // Setup hearing configuration
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.f;
        // Fixed: Removed deprecated LoSHearingRange property, using HearingRange instead
        // HearingConfig->LoSHearingRange = 1500.f; // This line is deprecated
        HearingConfig->SetMaxAge(7.f);
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        
        GetPerceptionComponent()->ConfigureSense(*HearingConfig);
    }
    
    // Set dominant sense and bind perception update function
    GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AAICompanionController::OnTargetPerceptionUpdated);
}

// Handle perception updates
void AAICompanionController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus)
{
    if (!BlackboardComponent)
    {
        return;
    }
    
    // If the detected actor is our owner player
    if (Actor == OwnerPlayer)
    {
        BlackboardComponent->SetValueAsBool("IsOwnerSeen", Stimulus.WasSuccessfullySensed());
        return;
    }
    
    // If the detected actor is a player but not our owner (for multiplayer)
    if (Actor->IsA(ACharacter::StaticClass()) && UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) == Actor)
    {
        BlackboardComponent->SetValueAsBool("IsPlayerSeen", Stimulus.WasSuccessfullySensed());
        
        // Store last known player location
        if (Stimulus.WasSuccessfullySensed())
        {
            BlackboardComponent->SetValueAsVector("LastKnownPlayerLocation", Stimulus.StimulusLocation);
        }
    }
    
    // Handle threat detection
    if (Stimulus.Tag == "Threat" || Stimulus.Tag == "Enemy")
    {
        BlackboardComponent->SetValueAsBool("IsThreatDetected", Stimulus.WasSuccessfullySensed());
        BlackboardComponent->SetValueAsObject("ThreatActor", Actor);
        
        if (Stimulus.WasSuccessfullySensed())
        {
            BlackboardComponent->SetValueAsVector("ThreatLocation", Stimulus.StimulusLocation);
        }
    }
    
    // Handle resource detection for survival gameplay
    if (Stimulus.Tag == "Resource")
    {
        BlackboardComponent->SetValueAsBool("IsResourceDetected", Stimulus.WasSuccessfullySensed());
        BlackboardComponent->SetValueAsObject("ResourceActor", Actor);
        
        if (Stimulus.WasSuccessfullySensed())
        {
            BlackboardComponent->SetValueAsVector("ResourceLocation", Stimulus.StimulusLocation);
        }
    }
}

// Setup companion-specific blackboard values
void AAICompanionController::SetupCompanionBlackboardValues()
{
    if (!BlackboardComponent || !GetPawn())
    {
        return;
    }
    
    // Set core references
    BlackboardComponent->SetValueAsObject("SelfActor", GetPawn());
    BlackboardComponent->SetValueAsObject("CompanionRef", GetPawn());
    
    // Set the owner player reference if available
    SetOwnerPlayer(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    // Set home location to current position
    BlackboardComponent->SetValueAsVector("HomeLocation", GetPawn()->GetActorLocation());
    
    // Initialize scores with survival focus
    BlackboardComponent->SetValueAsFloat("IdleScore", 0.2f);
    BlackboardComponent->SetValueAsFloat("FollowScore", 0.5f);
    BlackboardComponent->SetValueAsFloat("PatrolScore", 0.3f);
    BlackboardComponent->SetValueAsFloat("GatherScore", 0.8f); // Higher priority for survival game
    BlackboardComponent->SetValueAsFloat("CombatScore", 0.9f); // Higher for combat in survival game
    
    // Initialize states
    BlackboardComponent->SetValueAsBool("IsTaskActive", false);
    BlackboardComponent->SetValueAsBool("IsFollowing", false);
    BlackboardComponent->SetValueAsBool("IsPatrolling", false);
    BlackboardComponent->SetValueAsBool("IsGathering", false);
    BlackboardComponent->SetValueAsBool("HasValidTarget", false);
    BlackboardComponent->SetValueAsBool("IsOwnerInDanger", false);
    BlackboardComponent->SetValueAsBool("IsSelfInDanger", false);
    BlackboardComponent->SetValueAsBool("IsResourceDetected", false);
    BlackboardComponent->SetValueAsBool("IsThreatDetected", false);
    BlackboardComponent->SetValueAsEnum("CurrentTaskType", 0); // None
    
    // Survival-specific values
    BlackboardComponent->SetValueAsInt("InventorySpace", 10);
    BlackboardComponent->SetValueAsInt("ResourceAmount", 0);
    BlackboardComponent->SetValueAsFloat("OwnerProximity", 0.0f);
    BlackboardComponent->SetValueAsFloat("CurrentStamina", 100.0f);
    BlackboardComponent->SetValueAsFloat("MaxStamina", 100.0f);
}

// Set the owner player for this companion
void AAICompanionController::SetOwnerPlayer(ACharacter* NewOwnerPlayer)
{
    // Store previous owner for comparison
    ACharacter* PreviousOwner = OwnerPlayer;
    
    // Update owner reference
    OwnerPlayer = NewOwnerPlayer;
    
    // Update blackboard if available
    if (BlackboardComponent && OwnerPlayer)
    {
        BlackboardComponent->SetValueAsObject("PlayerRef", OwnerPlayer);
        BlackboardComponent->SetValueAsVector("OwnerLocation", OwnerPlayer->GetActorLocation());
        
        UE_LOG(LogTemp, Log, TEXT("AICompanionController: Owner player set to %s"), *GetNameSafe(OwnerPlayer));
    }
}

// Get the owner player for this companion
ACharacter* AAICompanionController::GetOwnerPlayer() const
{
    return OwnerPlayer;
}

// Update blackboard values periodically for performance
void AAICompanionController::UpdateBlackboardValues()
{
    if (!BlackboardComponent || !GetPawn() || !OwnerPlayer)
    {
        return;
    }
    
    // Update owner location and distance
    FVector OwnerLocation = OwnerPlayer->GetActorLocation();
    BlackboardComponent->SetValueAsVector("OwnerLocation", OwnerLocation);
    
    // Calculate distance to owner
    float OwnerDistance = FVector::Distance(GetPawn()->GetActorLocation(), OwnerLocation);
    BlackboardComponent->SetValueAsFloat("OwnerDistance", OwnerDistance);
    
    // Calculate normalized proximity (1.0 = very close, 0.0 = far away)
    float ProximityValue = FMath::Clamp(1.0f - (OwnerDistance / 2000.0f), 0.0f, 1.0f);
    BlackboardComponent->SetValueAsFloat("OwnerProximity", ProximityValue);
    
    // Update threat awareness less frequently for performance
    static int32 UpdateCounter = 0;
    if (++UpdateCounter % 4 == 0)
    {
        UpdateThreatAwareness();
        UpdateSocialAwareness();
    }
}

// Force update all blackboard values
void AAICompanionController::ForceUpdateBlackboardValues()
{
    UpdateBlackboardValues();
    UpdateThreatAwareness();
    UpdateSocialAwareness();
    UpdatePerceptionSettings();
    DebugBlackboardValues();
}

// New debug method to log blackboard values
void AAICompanionController::DebugBlackboardValues()
{
    if (!BlackboardComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AICompanionController: Cannot debug - BlackboardComponent is null"));
        return;
    }
    
    // Log all important values
    UE_LOG(LogTemp, Warning, TEXT("---- Blackboard Debug Values ----"));
    UE_LOG(LogTemp, Warning, TEXT("CurrentTaskType: %d"), BlackboardComponent->GetValueAsEnum("CurrentTaskType"));
    UE_LOG(LogTemp, Warning, TEXT("IsTaskActive: %s"), BlackboardComponent->GetValueAsBool("IsTaskActive") ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("IsFollowing: %s"), BlackboardComponent->GetValueAsBool("IsFollowing") ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("IsPatrolling: %s"), BlackboardComponent->GetValueAsBool("IsPatrolling") ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Warning, TEXT("IsGathering: %s"), BlackboardComponent->GetValueAsBool("IsGathering") ? TEXT("true") : TEXT("false"));
    
    // Log player reference
    AActor* Player = Cast<AActor>(BlackboardComponent->GetValueAsObject("PlayerRef"));
    UE_LOG(LogTemp, Warning, TEXT("PlayerRef: %s"), Player ? *Player->GetName() : TEXT("nullptr"));
    
    // Log location
    FVector PlayerLoc = BlackboardComponent->GetValueAsVector("LastKnownPlayerLocation");
    UE_LOG(LogTemp, Warning, TEXT("PlayerLocation: %s"), *PlayerLoc.ToString());
    
    // Log distance values
    UE_LOG(LogTemp, Warning, TEXT("OwnerDistance: %.2f"), BlackboardComponent->GetValueAsFloat("OwnerDistance"));
    UE_LOG(LogTemp, Warning, TEXT("OwnerProximity: %.2f"), BlackboardComponent->GetValueAsFloat("OwnerProximity"));
}

// Log companion status with context
void AAICompanionController::LogCompanionStatus(const FString& Context)
{
    if (!BlackboardComponent || !GetPawn())
    {
        return;
    }
    
    const FString PawnName = GetNameSafe(GetPawn());
    const int32 TaskType = BlackboardComponent->GetValueAsEnum("CurrentTaskType");
    const bool bIsActive = BlackboardComponent->GetValueAsBool("IsTaskActive");
    const bool bIsFollowing = BlackboardComponent->GetValueAsBool("IsFollowing");
    
    UE_LOG(LogTemp, Log, TEXT("[%s] - %s: Task=%d, Active=%s, Following=%s"),
        *Context,
        *PawnName,
        TaskType,
        bIsActive ? TEXT("Yes") : TEXT("No"),
        bIsFollowing ? TEXT("Yes") : TEXT("No"));
    
    // Log location and debug visuals
    if (bIsFollowing)
    {
        const FVector OwnerLoc = BlackboardComponent->GetValueAsVector("OwnerLocation");
        const FVector MyLoc = GetPawn()->GetActorLocation();
        const float Distance = FVector::Dist(MyLoc, OwnerLoc);
        
        UE_LOG(LogTemp, Log, TEXT("Following - Distance: %.2f, My Location: %s, Target: %s"),
            Distance, *MyLoc.ToString(), *OwnerLoc.ToString());
        
        // Optional debug drawing
        DrawDebugLine(GetWorld(), MyLoc, OwnerLoc, FColor::Green, false, 5.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), OwnerLoc, 50.0f, 8, FColor::Blue, false, 5.0f);
    }
}

// Update companion's awareness of threats in the area
void AAICompanionController::UpdateThreatAwareness()
{
    if (!BlackboardComponent || !GetPawn() || !GetWorld())
    {
        return;
    }
    
    // This would scan for threats in the environment
    // Implementation depends on your game's threat system
}

// Update companion's social awareness for multiplayer
void AAICompanionController::UpdateSocialAwareness()
{
    if (!BlackboardComponent || !GetPawn() || !GetWorld())
    {
        return;
    }
    
    // This would detect and respond to other players and NPCs
    // Implemention depends on your game's social systems
}

// Update perception settings based on environment
void AAICompanionController::UpdatePerceptionSettings_Implementation()
{
    // Adjust perception based on environment
    // e.g., reduce sight in dark areas, increase hearing in quiet areas
}

// Called every frame
void AAICompanionController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Use time accumulator for optimized updates instead of updating every frame
    UpdateTimeAccumulator += DeltaTime;
    
    // Only update intensive operations periodically
    if (UpdateTimeAccumulator >= BlackboardUpdateInterval)
    {
        UpdateTimeAccumulator = 0.0f;
        
        // Updates are now handled by timer for better performance
        // Critical updates can still be done here if needed
    }
}