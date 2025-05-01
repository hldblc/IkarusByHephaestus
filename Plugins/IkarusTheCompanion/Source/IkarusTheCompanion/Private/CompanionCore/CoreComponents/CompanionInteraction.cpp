// CompanionInteraction.cpp
#include "CompanionCore/CoreComponents/CompanionInteraction.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCompanionInteraction::UCompanionInteraction()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCompanionInteraction::BeginPlay()
{
    Super::BeginPlay();
}

void UCompanionInteraction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Only perform interaction checks on client that controls the owner
    if (GetOwnerRole() != ROLE_AutonomousProxy && !GetOwner()->HasAuthority())
        return;
    
    // Find interactable each tick
    AActor* NewInteractable = FindBestInteractable();
    
    // If the interactable changed, trigger the delegate
    if (NewInteractable != CurrentInteractable.Get())
    {
        CurrentInteractable = NewInteractable;
        
        if (NewInteractable)
        {
            OnInteractableFound.Broadcast(NewInteractable);
        }
        else
        {
            // Broadcast null to indicate no interactable is available
            OnInteractableFound.Broadcast(nullptr);
        }
    }
}

void UCompanionInteraction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out);
    
    // Nothing to replicate in this component currently
    // If we add replicated properties later, add them here
}

AActor* UCompanionInteraction::FindBestInteractable()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return nullptr;
    }
    
    // Get owner's location and forward vector
    FVector Location = OwnerCharacter->GetActorLocation();
    FVector Forward = OwnerCharacter->GetActorForwardVector();
    
    // Do a sphere trace to find potential interactables
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    GetWorld()->SweepMultiByChannel(
        HitResults,
        Location,
        Location + Forward * 10.0f, // Just to make it a minimal sweep
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(InteractionRange),
        QueryParams
    );
    
    // Debug visualization
    if (bShowDebugTraces)
    {
        DrawDebugSphere(GetWorld(), Location, InteractionRange, 12, FColor::Green, false, -1.0f, 0, 1.0f);
    }
    
    // Find the closest interactable that implements our interface
    AActor* ClosestInteractable = nullptr;
    float ClosestDistance = FLT_MAX;
    
    for (const FHitResult& Hit : HitResults)
    {
        if (Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UCompInteraction::StaticClass()))
        {
            // Check if the actor can be interacted with
            bool bCanInteract = ICompInteraction::Execute_CanInteract(Hit.GetActor(), GetOwner());
            
            if (bCanInteract)
            {
                float Distance = FVector::Dist(Location, Hit.GetActor()->GetActorLocation());
                
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestInteractable = Hit.GetActor();
                }
            }
        }
    }
    
    return ClosestInteractable;
}

void UCompanionInteraction::TryInteract()
{
    if (CurrentInteractable.IsValid())
    {
        // For multiplayer, we need to route this through the server
        if (GetOwnerRole() != ROLE_Authority)
        {
            ServerTryInteract(CurrentInteractable.Get());
        }
        else
        {
            // Direct interaction on server
            PerformInteraction(CurrentInteractable.Get());
        }
    }
}

void UCompanionInteraction::ServerTryInteract_Implementation(AActor* InteractableActor)
{
    // Validate the interactable again on the server side
    if (InteractableActor && InteractableActor->GetClass()->ImplementsInterface(UCompInteraction::StaticClass()))
    {
        bool bCanInteract = ICompInteraction::Execute_CanInteract(InteractableActor, GetOwner());
        
        if (bCanInteract)
        {
            PerformInteraction(InteractableActor);
        }
    }
}

bool UCompanionInteraction::ServerTryInteract_Validate(AActor* InteractableActor)
{
    // Basic validation - we could add more checks here
    return true;
}

void UCompanionInteraction::PerformInteraction(AActor* InteractableActor)
{
    // Execute the interaction
    ICompInteraction::Execute_OnInteract(InteractableActor, GetOwner());
    
    // Notify about interaction
    OnInteraction.Broadcast(InteractableActor);
    
    // Multicast to notify all clients about the interaction
    MulticastOnInteraction(InteractableActor);
}

void UCompanionInteraction::MulticastOnInteraction_Implementation(AActor* InteractableActor)
{
    // Only execute on remote clients, not on server or owning client (they already handled it)
    if (GetOwnerRole() != ROLE_Authority && !IsLocallyControlled())
    {
        OnInteraction.Broadcast(InteractableActor);
    }
}

bool UCompanionInteraction::IsLocallyControlled() const
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        return OwnerPawn->IsLocallyControlled();
    }
    return false;
}