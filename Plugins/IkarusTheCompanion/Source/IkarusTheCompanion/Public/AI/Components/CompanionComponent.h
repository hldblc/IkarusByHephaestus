// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CompanionComponent.generated.h"

// Delegate to broadcast when the companion’s state changes (for example, assistance requested)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCompanionAssistanceRequested);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class IKARUSTHECOMPANION_API UCompanionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCompanionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	// Request assistance from the companion
	// This can be called from Blueprint or C++
	UFUNCTION(BlueprintCallable, Category = "Companion")
	void RequestAssistance();

	// Event dispatcher for when assistance is requested
	UPROPERTY(BlueprintAssignable, Category = "Companion")
	FOnCompanionAssistanceRequested OnAssistanceRequested;

	// Example state variable; expand with additional properties as needed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	bool bIsAssisting;
};
