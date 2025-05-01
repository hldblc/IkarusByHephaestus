#pragma once

#include "CoreMinimal.h"
#include "CompanionInterfaces/CompanionInterface.h"
#include "GameFramework/Character.h"
#include "CompanionCore/CoreStructs/CompanionCoreStructs.h"          
#include "Net/UnrealNetwork.h"
#include "CompanionInterfaces/CompInteraction.h"
#include "CompanionCore/CoreUI/AICommandPanel.h"
#include "IkarusCharacter.generated.h"

class UCharacterMovementComponent;
class UCompanionTaskComponent;
class AAICompanionController;

UCLASS()
class IKARUSTHECOMPANION_API AIkarusCharacter
        : public ACharacter
        , public ICompanionInterface
        , public ICompInteraction
{
    GENERATED_BODY()

public:
    AIkarusCharacter();

    /* ---------- Actor overrides ---------- */
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

    /* ---------- Public helpers ----------- */

    FORCEINLINE UCharacterMovementComponent* GetMoveComp() const noexcept
    { return MoveComp; }

    /* ---------- Interface --------------- */
    virtual void SetCompanionMovementSpeed_Implementation(
        ECompanionMovementSpeed CompanionSpeed,
        float&                  OutSpeedValue) override;

    UFUNCTION(BlueprintCallable, Category="Companion|Movement")
    void SetMovementSpeed(ECompanionMovementSpeed CompanionSpeed, float& OutSpeedValue);

    /* ---------- Data-driven -------------- */
    UPROPERTY(EditDefaultsOnly, Category="Companion|Movement",
              meta=(RowType="/Script/IkarustheCompanion.FCompanionMovementPreset"))
    TSoftObjectPtr<UDataTable> MovementPresetTable;

    /** Replicates so every client picks the same row. */
    UPROPERTY(ReplicatedUsing=OnRep_MovementPresetRow, EditAnywhere,
              BlueprintReadWrite, Category="Companion|Movement")
    FName MovementPresetRow = "Default";


    /* ---------- Interaction ------------- */
    // ICompInteraction Interface implementation
    virtual void OnInteract_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionText_Implementation() override;
    
    // Command system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Interaction")
    TArray<FName> AvailableCommands;
    
    // Execute a specific command
    UFUNCTION(BlueprintCallable, Category = "Companion|Interaction")
    void ExecuteCommand(FName CommandName, AActor* Commander);
    
    // Server RPC for command execution
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerExecuteCommand(FName CommandName, AActor* Commander);
private:

    // UI components
    UPROPERTY(EditDefaultsOnly, Category = "Companion|UI")
    TSubclassOf<class UAICommandPanel> CommandPanelClass;
    
    // Helper to show interaction UI
    void ShowCommandUI(AActor* Interactor);

    
    /* ---------- Components -------------- */
    UPROPERTY(Transient) AAICompanionController*      AIController = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* MoveComp     = nullptr;

    /* ---------- Cached preset floats ----- */
    float IdleSpeed     = 0.f;
    float WalkingSpeed  = 225.f;
    float RunningSpeed  = 450.f;
    float SprintSpeed   = 675.f;
    float SwimmingSpeed = 200.f;
    float FlyingSpeed   = 500.f;

    /* ---------- Timers ------------------ */
    FTimerHandle BlackboardUpdateTimer;

    /* ---------- Helpers ------------------ */
    void UpdateBlackboard();
    void LoadMovementPreset();
    UFUNCTION() void OnRep_MovementPresetRow();

    void MoveForward(float Value);
    void MoveRight (float Value);
    void Turn      (float Value);
    void LookUp    (float Value);
};