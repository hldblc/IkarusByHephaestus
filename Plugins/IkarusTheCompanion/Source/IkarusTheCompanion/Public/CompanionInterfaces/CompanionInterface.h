// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "CompanionCore/CoreEnums/CompanionEnums.h"
#include "CompanionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UCompanionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IKARUSTHECOMPANION_API ICompanionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Companion|Movement")
	void SetCompanionMovementSpeed(ECompanionMovementSpeed CompanionSpeed, UPARAM(Ref) float& OutSpeedValue);
};
