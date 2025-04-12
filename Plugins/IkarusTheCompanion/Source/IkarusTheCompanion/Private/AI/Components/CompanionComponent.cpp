// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/Components/CompanionComponent.h"

// Sets default values for this component's properties
UCompanionComponent::UCompanionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsAssisting = false;
}

// Called when the game starts
void UCompanionComponent::BeginPlay()
{
	Super::BeginPlay();
	// Initialization logic here (e.g., binding events, initial state setup)
}

// Called every frame
void UCompanionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
										FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Update logic - e.g., recalculate behavior, update animations, process sensor input, etc.
}

void UCompanionComponent::RequestAssistance()
{
	// Implement logic that should occur when assistance is requested.
	// This could trigger a change in state or notify the AIController.
	bIsAssisting = true;

	// Broadcast that assistance was requested
	OnAssistanceRequested.Broadcast();

	// You can also print to the screen or log for debugging purposes.
	UE_LOG(LogTemp, Log, TEXT("Companion assistance requested."));
}
