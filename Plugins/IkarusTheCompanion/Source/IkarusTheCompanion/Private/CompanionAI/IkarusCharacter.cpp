// Fill out your copyright notice in the Description page of Project Settings.

#include "IkarusTheCompanion/Public/CompanionAI/IkarusCharacter.h"
#include "CompanionAI/Components/CompanionTaskComponent.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AIkarusCharacter::AIkarusCharacter()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Create and attach the companion task component
    CompanionTaskComponent = CreateDefaultSubobject<UCompanionTaskComponent>(TEXT("CompanionTaskComponent"));
    
    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    
    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void AIkarusCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Get and cache the AI controller
    AIController = Cast<AAICompanionController>(GetController());
    
    // Initial blackboard update
    UpdateBlackboard();
}

// Called every frame
void AIkarusCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update blackboard values every frame
    UpdateBlackboard();
}

// Called to bind functionality to input
void AIkarusCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Set up movement bindings
    PlayerInputComponent->BindAxis("MoveForward", this, &AIkarusCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AIkarusCharacter::MoveRight);
    
    // Set up look bindings
    PlayerInputComponent->BindAxis("Turn", this, &AIkarusCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &AIkarusCharacter::LookUp);
    
    // Set up action bindings
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
}

void AIkarusCharacter::UpdateBlackboard()
{
    if (AIController)
    {
        UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
        if (BlackboardComp)
        {
            // Update location for AI to follow
            BlackboardComp->SetValueAsVector(FName("OwnerLocation"), GetActorLocation());
            
            // Update whether the player is moving
            bool bIsMoving = GetVelocity().Size() > 10.0f;
            BlackboardComp->SetValueAsBool(FName("IsPlayerMoving"), bIsMoving);
            
            // Reference to self
            BlackboardComp->SetValueAsObject(FName("OwnerPlayerRef"), this);
        }
    }
}

void AIkarusCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AIkarusCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get right vector 
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AIkarusCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void AIkarusCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}