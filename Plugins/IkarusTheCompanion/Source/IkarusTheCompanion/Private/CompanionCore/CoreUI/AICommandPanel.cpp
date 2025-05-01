// AICommandPanel.cpp
#include "CompanionCore/CoreUI/AICommandPanel.h"
#include "CompanionAI/IkarusCharacter.h"

UAICommandPanel::UAICommandPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default initialization
}

void UAICommandPanel::NativeConstruct()
{
	Super::NativeConstruct();
    
	// Focus and input setup
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
    
	if (GetOwningPlayer())
	{
		// Set input mode to UI only with game visibility
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		GetOwningPlayer()->SetInputMode(InputMode);
		GetOwningPlayer()->bShowMouseCursor = true;
	}
}

void UAICommandPanel::NativeDestruct()
{
	// Restore game input mode when widget is closed
	if (GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		GetOwningPlayer()->SetInputMode(InputMode);
		GetOwningPlayer()->bShowMouseCursor = false;
	}
    
	Super::NativeDestruct();
}

void UAICommandPanel::SetupForCompanion(AIkarusCharacter* InCompanion, const TArray<FName>& Commands)
{
	// Set target companion
	TargetCompanion = InCompanion;
    
	// Call blueprint event to create command UI
	CreateCommandUI(Commands);
}

void UAICommandPanel::HandleCommandSelected(FName CommandName)
{
	UE_LOG(LogTemp, Warning, TEXT("AICommandPanel: Command selected: %s"), *CommandName.ToString());
    
	if (TargetCompanion.IsValid() && GetOwningPlayerPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("AICommandPanel: Broadcasting command to companion"));
        
		// Broadcast the command
		OnCommandSelected.Broadcast(CommandName, GetOwningPlayerPawn());
        
		// Close the panel
		CloseCommandPanel();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AICommandPanel: Invalid companion or player pawn!"));
	}
}

void UAICommandPanel::CloseCommandPanel()
{
	// Remove from parent and clean up
	RemoveFromParent();
}