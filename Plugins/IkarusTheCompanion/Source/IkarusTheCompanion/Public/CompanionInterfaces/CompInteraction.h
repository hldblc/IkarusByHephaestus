// CompInteraction.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CompInteraction.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UCompInteraction : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IKARUSTHECOMPANION_API ICompInteraction
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Companion|Interaction")
	void OnInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Companion|Interaction")
	bool CanInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Companion|Interaction")
	FText GetInteractionText();
	
};
