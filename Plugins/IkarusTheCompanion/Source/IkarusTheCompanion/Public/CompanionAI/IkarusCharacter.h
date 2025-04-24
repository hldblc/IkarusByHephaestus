// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IkarusCharacter.generated.h"

UCLASS()
class IKARUSTHECOMPANION_API AIkarusCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AIkarusCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Get the task component
	UFUNCTION(BlueprintCallable, Category = "AI")
	class UCompanionTaskComponent* GetTaskComponent() const { return CompanionTaskComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    
	// Companion task component for managing AI tasks
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UCompanionTaskComponent* CompanionTaskComponent;
    
	// Cached reference to the companion's AI controller
	UPROPERTY()
	class AAICompanionController* AIController;
    
	// Update the blackboard with current status
	void UpdateBlackboard();
    
	// Movement input functions
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
};