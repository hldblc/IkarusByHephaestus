// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICompanionController.generated.h"

class UCompanionComponent;

UCLASS()
class IKARUSTHECOMPANION_API AAICompanionController : public AAIController
{
	GENERATED_BODY()

public:
	AAICompanionController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Overriding Possess to acquire reference to CompanionComponent
	virtual void OnPossess(APawn* InPawn) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Returns the current CompanionComponent reference (if any)
	UFUNCTION(BlueprintCallable, Category = "Companion")
	UCompanionComponent* GetCompanionComponent() const;

private:
	// Cache a reference to the companion component on the possessed pawn
	UPROPERTY(Transient)
	UCompanionComponent* CompanionComponent;
};
