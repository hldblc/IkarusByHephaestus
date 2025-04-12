// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/Controllers/AICompanionController.h"
#include "AI/Components/CompanionComponent.h"
#include "GameFramework/Pawn.h"

// Sets default values
AAICompanionController::AAICompanionController()
{
	PrimaryActorTick.bCanEverTick = true;
	CompanionComponent = nullptr;
}

// Called when the game starts or when spawned
void AAICompanionController::BeginPlay()
{
	Super::BeginPlay();
	// Additional initialization if needed
}

// When possessing a pawn, try to find and cache the CompanionComponent
void AAICompanionController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		CompanionComponent = InPawn->FindComponentByClass<UCompanionComponent>();
		if (CompanionComponent)
		{
			UE_LOG(LogTemp, Log, TEXT("CompanionComponent successfully found on possessed pawn."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No CompanionComponent found on possessed pawn."));
		}
	}
}

// Called every frame
void AAICompanionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Example: Check if the companion has requested assistance, then handle decision making.
	if (CompanionComponent && CompanionComponent->bIsAssisting)
	{
		// Implement assistance behavior (e.g., moving to cover, notifying other systems, etc.)
		UE_LOG(LogTemp, Log, TEXT("Assistance requested - updating behavior logic."));
		// For now, we could reset the flag after processing.
		CompanionComponent->bIsAssisting = false;
	}
}

// Returns the cached CompanionComponent
UCompanionComponent* AAICompanionController::GetCompanionComponent() const
{
	return CompanionComponent;
}
