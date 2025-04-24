#include "CompanionAI/BTServices/BTService_UpdatePlayerLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"

UBTService_UpdatePlayerLocation::UBTService_UpdatePlayerLocation()
{
    NodeName = "Update Player Location";
    
    // Set default interval
    Interval = 0.5f;
    RandomDeviation = 0.1f;
    
    // Configure blackboard key filters
    LocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdatePlayerLocation, LocationKey));
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdatePlayerLocation, TargetActorKey), UObject::StaticClass());
}

void UBTService_UpdatePlayerLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    // Get the blackboard
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }
    
    // Check if we have valid keys
    if (!LocationKey.SelectedKeyName.IsNone() && !TargetActorKey.SelectedKeyName.IsNone())
    {
        // Get the target object from blackboard
        UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName);
        if (!TargetObject)
        {
            return;
        }
        
        // Try to get the actor's location using different approaches
        FVector TargetLocation = FVector::ZeroVector;
        bool bLocationFound = false;
        
        // Try direct cast to Actor first
        if (AActor* TargetActor = Cast<AActor>(TargetObject))
        {
            TargetLocation = TargetActor->GetActorLocation();
            bLocationFound = true;
        }
        // If not an actor, check if it's an actor component
        else if (UActorComponent* Component = Cast<UActorComponent>(TargetObject))
        {
            if (AActor* Owner = Component->GetOwner())
            {
                TargetLocation = Owner->GetActorLocation();
                bLocationFound = true;
            }
        }
        
        // Update the location in the blackboard if we found it
        if (bLocationFound)
        {
            BlackboardComp->SetValueAsVector(LocationKey.SelectedKeyName, TargetLocation);
        }
    }
} 