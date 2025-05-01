// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CompanionInterfaces/CompInteraction.h"
#include "Components/ActorComponent.h"
#include "CompanionInterfaces/CompInteraction.h"
#include "CompanionInteraction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AActor*, InteractableActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteraction, AActor*, InteractedActor);



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class IKARUSTHECOMPANION_API UCompanionInteraction : public UActorComponent 
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCompanionInteraction();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

	// Try to interact with the closest valid interactable in range
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();
    
	// Event fired when an interactable is found
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableFound OnInteractableFound;
    
	// Event fired when interaction occurs
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteraction OnInteraction;

protected:
	// The range within which to detect interactable objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 200.0f;
    
	// Whether to show debug lines for interaction traces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugTraces = false;
    
	// Trace and find the best interactable actor
	AActor* FindBestInteractable();

	// Networked interaction methods
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryInteract(AActor* InteractableActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnInteraction(AActor* InteractableActor);

	// Helper to perform the actual interaction
	void PerformInteraction(AActor* InteractableActor);

	// Check if this component is on a locally controlled pawn
	bool IsLocallyControlled() const;
    
private:
	// Currently detected interactable actor
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentInteractable;
};

