#include "CompanionAI/IkarusCharacter.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"
#include "CompanionCore/CoreUI/AICommandPanel.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"

namespace
{
    constexpr TCHAR KEY_PlayerRef[]          = TEXT("PlayerRef");
    constexpr TCHAR KEY_CompanionLocation[]  = TEXT("CompanionLocation");
    constexpr TCHAR KEY_CompanionDistance[]  = TEXT("CompanionDistance");
    constexpr TCHAR KEY_CompanionProximity[] = TEXT("CompanionProximity");
    constexpr TCHAR KEY_LastKnownPlayerLocation[] = TEXT("LastKnownPlayerLocation");
    constexpr TCHAR KEY_CurrentTaskType[]    = TEXT("CurrentTaskType");
    constexpr TCHAR KEY_IsTaskActive[]       = TEXT("IsTaskActive");
    constexpr TCHAR KEY_IsFollowing[]        = TEXT("IsFollowing");
    constexpr TCHAR KEY_IsPatrolling[]       = TEXT("IsPatrolling");
    constexpr TCHAR KEY_IsGathering[]        = TEXT("IsGathering");
    
    constexpr float MaxProximityRange = 2000.f;
    constexpr float BlackboardUpdateInterval = 0.1f; // Update blackboard less frequently
}

/* ===== ctor ===== */
AIkarusCharacter::AIkarusCharacter()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize with default available commands
    AvailableCommands.Add("Follow");
    AvailableCommands.Add("Stay");
    AvailableCommands.Add("Patrol");
    AvailableCommands.Add("Gather");

    MoveComp = GetCharacterMovement();
    check(MoveComp);

    // Character movement settings for smoother movement
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate              = {0.f, 540.f, 0.f};
    MoveComp->JumpZVelocity             = 600.f;
    MoveComp->AirControl                = 0.2f;
    
    // Additional smoothing settings
    MoveComp->bUseRVOAvoidance = true;
    MoveComp->AvoidanceConsiderationRadius = 500.f;
    
    // Smooth acceleration
    MoveComp->bRequestedMoveUseAcceleration = true;
    MoveComp->MaxAcceleration = 1000.f;
    MoveComp->BrakingDecelerationWalking = 1000.f;

    bUseControllerRotationPitch =
    bUseControllerRotationYaw   =
    bUseControllerRotationRoll  = false;
}

/* ===== replication ===== */
void AIkarusCharacter::GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const   
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AIkarusCharacter, MovementPresetRow);
}

/* ===== BeginPlay ===== */
void AIkarusCharacter::BeginPlay()
{
    Super::BeginPlay();
    AIController = Cast<AAICompanionController>(GetController());
    LoadMovementPreset();
    UpdateBlackboard(); // Initial update
    
    // Set timer for blackboard updates instead of every tick
    GetWorld()->GetTimerManager().SetTimer(
        BlackboardUpdateTimer,
        this,
        &AIkarusCharacter::UpdateBlackboard,
        BlackboardUpdateInterval,
        true
    );
}

void AIkarusCharacter::OnRep_MovementPresetRow()
{
    LoadMovementPreset();
}

/* ===== frame ===== */
void AIkarusCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Removed UpdateBlackboard() call to prevent jittering
}

/* ===== Blackboard sync ===== */
void AIkarusCharacter::UpdateBlackboard()
{
    if (!AIController) return;
    if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
    {
        BB->SetValueAsVector(KEY_CompanionLocation, GetActorLocation());

        if (AActor* Player = Cast<AActor>(BB->GetValueAsObject(KEY_PlayerRef)))
        {
            const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
            BB->SetValueAsFloat(KEY_CompanionDistance, Dist);

            const float Prox = FMath::Clamp(Dist / MaxProximityRange, 0.f, 1.f);
            BB->SetValueAsFloat(KEY_CompanionProximity, Prox);

            BB->SetValueAsVector(KEY_LastKnownPlayerLocation, Player->GetActorLocation());
        }
    }
}

/* ===== preset loader ===== */
void AIkarusCharacter::LoadMovementPreset()
{
    const UDataTable* Table = MovementPresetTable.LoadSynchronous();
    if (!Table) return;

    const FCompanionMovementPreset* Row =
        Table->FindRow<FCompanionMovementPreset>(MovementPresetRow, TEXT("MovementPreset"));
    if (!Row) return;

    IdleSpeed     = Row->IdleSpeed;
    WalkingSpeed  = Row->WalkingSpeed;
    RunningSpeed  = Row->RunningSpeed;
    SprintSpeed   = Row->SprintSpeed;
    SwimmingSpeed = Row->SwimmingSpeed;
    FlyingSpeed   = Row->FlyingSpeed;
}

/* ===== Movement helpers ===== */
void AIkarusCharacter::SetMovementSpeed(ECompanionMovementSpeed Speed, float& Out)
{
    switch (Speed)
    {
        case ECompanionMovementSpeed::Idle:       Out = IdleSpeed;     break;
        case ECompanionMovementSpeed::Walking:    Out = WalkingSpeed;  break;
        case ECompanionMovementSpeed::Running:    Out = RunningSpeed;  break;
        case ECompanionMovementSpeed::Sprinting:  Out = SprintSpeed;   break;
        case ECompanionMovementSpeed::Swimming:   Out = SwimmingSpeed; break;
        case ECompanionMovementSpeed::Flying:     Out = FlyingSpeed;   break;
        default:                                  Out = IdleSpeed;     break;
    }

    if (MoveComp && Speed != ECompanionMovementSpeed::Teleporting)
        MoveComp->MaxWalkSpeed = Out;
}

void AIkarusCharacter::SetCompanionMovementSpeed_Implementation(
        ECompanionMovementSpeed Speed, float& Out)
{
    SetMovementSpeed(Speed, Out);
}

/* ===== (unused) input helpers ===== */
void AIkarusCharacter::MoveForward(float V){ if(V && Controller){ const FRotator Yaw(0,Controller->GetControlRotation().Yaw,0); AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X),V);} }
void AIkarusCharacter::MoveRight (float V){ if(V && Controller){ const FRotator Yaw(0,Controller->GetControlRotation().Yaw,0); AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y),V);} }
void AIkarusCharacter::Turn      (float V){ AddControllerYawInput  (V); }
void AIkarusCharacter::LookUp    (float V){ AddControllerPitchInput(V); }


/* ==================== Interaction ==================== */
void AIkarusCharacter::OnInteract_Implementation(AActor* Interactor)
{
    // When a player interacts with this character, show command UI
    ShowCommandUI(Interactor);
}

bool AIkarusCharacter::CanInteract_Implementation(AActor* Interactor)
{
    // Basic validation - add game-specific logic as needed
    return IsValid(Interactor) && Cast<ACharacter>(Interactor) != nullptr;
}

FText AIkarusCharacter::GetInteractionText_Implementation()
{
    return FText::FromString(FString::Printf(TEXT("Talk to %s"), *GetName()));
}

void AIkarusCharacter::ShowCommandUI(AActor* Interactor)
{
    // Only proceed if we're on authority or owning client
    APlayerController* PC = Cast<APlayerController>(Cast<APawn>(Interactor)->GetController());
    if (!PC || !PC->IsLocalController())
    {
        return;
    }
    
    // Create command panel if class is specified
    if (CommandPanelClass)
    {
        UAICommandPanel* CommandPanel = CreateWidget<UAICommandPanel>(PC, CommandPanelClass);
        if (CommandPanel)
        {
            // Pass commands and setup callbacks
            CommandPanel->SetupForCompanion(this, AvailableCommands);
            
            // Bind command execution
            CommandPanel->OnCommandSelected.AddDynamic(this, &AIkarusCharacter::ExecuteCommand);
            
            // Display the panel
            CommandPanel->AddToViewport();
        }
    }
}

void AIkarusCharacter::ExecuteCommand(FName CommandName, AActor* Commander)
{
    // Local validation
    if (!IsValid(Commander))
    {
        return;
    }
    
    // For multiplayer, send to server
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerExecuteCommand(CommandName, Commander);
        return;
    }
    
    // Handle commands - these will update blackboard values
if (AIController && AIController->GetBlackboardComponent())
    {
        UBlackboardComponent* BB = AIController->GetBlackboardComponent();
        
        // Reset all task states first
        BB->SetValueAsBool("IsFollowing", false);
        BB->SetValueAsBool("IsPatrolling", false);
        BB->SetValueAsBool("IsGathering", false);
        BB->SetValueAsBool("IsTaskActive", true);  // Always set this to true for any command
        
        if (CommandName == "Follow")
        {
            // Use the exact enum value from ECompanionTask::Follow (which is 2)
            BB->SetValueAsEnum("CurrentTaskType", static_cast<uint8>(ECompanionTask::Follow));
            BB->SetValueAsBool("IsFollowing", true);
            BB->SetValueAsObject("PlayerRef", Commander);
            
            // Debug logging
            UE_LOG(LogTemp, Warning, TEXT("Follow command received - Setting CurrentTaskType to %d (Follow)"), 
                static_cast<uint8>(ECompanionTask::Follow));
                
            // Force debug output
            if (AIController)
            {
                AIController->DebugBlackboardValues();
            }
        }
        else if (CommandName == "Stay")
        {
            // Use the exact enum value for Idle
            BB->SetValueAsEnum("CurrentTaskType", static_cast<uint8>(ECompanionTask::Idle));
            
            // Debug logging
            UE_LOG(LogTemp, Warning, TEXT("Stay command received - Setting CurrentTaskType to %d (Idle)"), 
                static_cast<uint8>(ECompanionTask::Idle));
        }
        else if (CommandName == "Patrol")
        {
            BB->SetValueAsEnum("CurrentTaskType", static_cast<uint8>(ECompanionTask::Patrol));
            BB->SetValueAsBool("IsPatrolling", true);
            
            // Debug logging
            UE_LOG(LogTemp, Warning, TEXT("Patrol command received - Setting CurrentTaskType to %d (Patrol)"), 
                static_cast<uint8>(ECompanionTask::Patrol));
        }
        
        // Always log the final state after setting values
        UE_LOG(LogTemp, Warning, TEXT("Final state: TaskType=%d, IsTaskActive=%s, IsFollowing=%s"),
            BB->GetValueAsEnum("CurrentTaskType"),
            BB->GetValueAsBool("IsTaskActive") ? TEXT("true") : TEXT("false"),
            BB->GetValueAsBool("IsFollowing") ? TEXT("true") : TEXT("false"));
    }
}

bool AIkarusCharacter::ServerExecuteCommand_Validate(FName CommandName, AActor* Commander)
{
    // Validate command is in available commands
    return AvailableCommands.Contains(CommandName) && IsValid(Commander);
}

void AIkarusCharacter::ServerExecuteCommand_Implementation(FName CommandName, AActor* Commander)
{
    // Server execution of the command
    ExecuteCommand(CommandName, Commander);
}