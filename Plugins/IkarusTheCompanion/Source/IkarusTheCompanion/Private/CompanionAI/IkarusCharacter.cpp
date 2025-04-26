#include "CompanionAI/IkarusCharacter.h"
#include "CompanionAI/Components/CompanionTaskComponent.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
    constexpr TCHAR KEY_PlayerRef[]          = TEXT("PlayerRef");
    constexpr TCHAR KEY_CompanionLocation[]  = TEXT("CompanionLocation");
    constexpr TCHAR KEY_CompanionDistance[]  = TEXT("CompanionDistance");
    constexpr TCHAR KEY_CompanionProximity[] = TEXT("CompanionProximity");
    constexpr TCHAR KEY_LastKnownPlayerLocation[] = TEXT("LastKnownPlayerLocation");
    constexpr float MaxProximityRange = 2000.f;
}

/* ===== ctor ===== */
AIkarusCharacter::AIkarusCharacter()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    CompanionTaskComponent = CreateDefaultSubobject<UCompanionTaskComponent>(TEXT("CompanionTaskComponent"));

    MoveComp = GetCharacterMovement();
    check(MoveComp);

    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate              = {0.f, 540.f, 0.f};
    MoveComp->JumpZVelocity             = 600.f;
    MoveComp->AirControl                = 0.2f;

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
    UpdateBlackboard();
}

void AIkarusCharacter::OnRep_MovementPresetRow()
{
    LoadMovementPreset();
}

/* ===== frame ===== */
void AIkarusCharacter::Tick(float)
{
    Super::Tick(0.f);
    UpdateBlackboard();
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
