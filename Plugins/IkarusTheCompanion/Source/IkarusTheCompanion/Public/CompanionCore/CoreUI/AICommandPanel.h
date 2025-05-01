// AICommandPanel.h
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AICommandPanel.generated.h"

class AIkarusCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommandSelected, FName, CommandName, AActor*, Commander);

UCLASS()
class IKARUSTHECOMPANION_API UAICommandPanel : public UCommonUserWidget
{
	GENERATED_BODY()
    
public:
	UAICommandPanel(const FObjectInitializer& ObjectInitializer);
    
	// Setup the panel with available commands for a specific companion
	UFUNCTION(BlueprintCallable, Category = "Companion|UI")
	void SetupForCompanion(AIkarusCharacter* InCompanion, const TArray<FName>& Commands);
    
	// Event when a command is selected
	UPROPERTY(BlueprintAssignable, Category = "Companion|UI")
	FOnCommandSelected OnCommandSelected;
    
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
    
	// Reference to target companion
	UPROPERTY(BlueprintReadOnly, Category = "Companion|UI")
	TWeakObjectPtr<AIkarusCharacter> TargetCompanion;
    
	// Blueprint implementable event for creating command buttons
	UFUNCTION(BlueprintImplementableEvent, Category = "Companion|UI")
	void CreateCommandUI(const TArray<FName>& Commands);
    
	// Handle command button clicks
	UFUNCTION(BlueprintCallable, Category = "Companion|UI")
	void HandleCommandSelected(FName CommandName);
    
	// Close the panel
	UFUNCTION(BlueprintCallable, Category = "Companion|UI")
	void CloseCommandPanel();
};