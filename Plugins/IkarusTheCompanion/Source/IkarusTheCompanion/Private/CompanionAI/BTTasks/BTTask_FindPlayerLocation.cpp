// Fill out your copyright notice in the Description page of Project Settings.

#include "CompanionAI/BTTasks/BTTask_FindPlayerLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Find Player Location");
	
	// Initialize default values
	UseExactLocation = false;
	RequireNavigablePath = true;
	RequireLineOfSight = true;
	SearchRadius = 1000.0f;
	MinDistanceFromPlayer = 100.0f;
	UseDirectionalBias = false;
	DirectionalPreference = EPositioningPreference::NoPreference;
	AvoidPreviousLocations = false;
	LocationMemorySize = 5;
	DrawDebugPoints = false;
	DebugDuration = 5.0f;
	
	// Configure blackboard key
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPlayerLocation, BlackboardKey));
}

EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get AI controller and owner
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayerLocation: Failed to get AIController"));
		return EBTNodeResult::Failed;
	}
	
	// Get blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayerLocation: Failed to get BlackboardComponent"));
		return EBTNodeResult::Failed;
	}
	
	// Get the world
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayerLocation: Failed to get World"));
		return EBTNodeResult::Failed;
	}
	
	// Get player character
	if (auto* const PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0))
	{
		FVector TargetPlayerLocation;
		
		if (UseExactLocation)
		{
			// Use exact player location
			TargetPlayerLocation = PlayerCharacter->GetActorLocation();
			
			// Set value in blackboard
			BlackboardComp->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetPlayerLocation);
			
			if (DrawDebugPoints)
			{
				DrawDebugSphere(World, TargetPlayerLocation, 50.0f, 8, FColor::Green, false, DebugDuration);
			}
			
			return EBTNodeResult::Succeeded;
		}
		else
		{
			// Find a nearby location
			if (FindNearbyLocation(PlayerCharacter, TargetPlayerLocation))
			{
				// Set value in blackboard
				BlackboardComp->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetPlayerLocation);
				
				if (DrawDebugPoints)
				{
					DrawDebugSphere(World, TargetPlayerLocation, 50.0f, 8, FColor::Green, false, DebugDuration);
				}
				
				return EBTNodeResult::Succeeded;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayerLocation: Failed to find player character"));
	}
	
	return EBTNodeResult::Failed;
}

bool UBTTask_FindPlayerLocation::FindNearbyLocation(AActor* PlayerActor, FVector& OutLocation)
{
	UWorld* World = GetWorld();
	if (!World || !PlayerActor)
	{
		return false;
	}
	
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayerLocation: Failed to get NavigationSystem"));
		return false;
	}
	
	const FVector PlayerLocation = PlayerActor->GetActorLocation();
	FNavLocation ResultLocation;
	
	// Try to find a valid location up to MaxAttempts times
	const int32 MaxAttempts = 10; // Default max attempts
	
	for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
	{
		// Get a random point in navigable radius
		FVector OriginLocation = PlayerLocation;
		
		// Apply directional bias if needed
		if (UseDirectionalBias)
		{
			OriginLocation = ApplyDirectionalBias(PlayerActor, PlayerLocation, MinDistanceFromPlayer);
		}
		
		// Find random point in navigable radius
		if (NavSys->GetRandomReachablePointInRadius(OriginLocation, SearchRadius, ResultLocation))
		{
			FVector PotentialLocation = ResultLocation.Location;
			
			// Check minimum distance from player
			float DistanceToPlayer = FVector::Dist(PotentialLocation, PlayerLocation);
			if (DistanceToPlayer < MinDistanceFromPlayer)
			{
				if (DrawDebugPoints)
				{
					DrawDebugSphere(World, PotentialLocation, 20.0f, 8, FColor::Red, false, DebugDuration);
				}
				continue; // Too close to player, try again
			}
			
			// Check line of sight if required
			if (RequireLineOfSight)
			{
				FHitResult HitResult;
				bool bHasLineOfSight = !World->LineTraceTestByChannel(
					PlayerLocation,
					PotentialLocation,
					ECC_Visibility,
					FCollisionQueryParams(TEXT("LineOfSight"), true, PlayerActor)
				);
				
				if (!bHasLineOfSight)
				{
					if (DrawDebugPoints)
					{
						DrawDebugSphere(World, PotentialLocation, 20.0f, 8, FColor::Yellow, false, DebugDuration);
						DrawDebugLine(World, PlayerLocation, PotentialLocation, FColor::Yellow, false, DebugDuration);
					}
					continue; // No line of sight, try again
				}
			}
			
			// Check if this location has been used recently
			if (AvoidPreviousLocations && HasLocationBeenUsedRecently(PotentialLocation))
			{
				if (DrawDebugPoints)
				{
					DrawDebugSphere(World, PotentialLocation, 20.0f, 8, FColor::Purple, false, DebugDuration);
				}
				continue; // Location used recently, try again
			}
			
			// Found a valid location
			OutLocation = PotentialLocation;
			
			// Add to previous locations if needed
			if (AvoidPreviousLocations)
			{
				if (PreviousLocations.Num() >= LocationMemorySize)
				{
					PreviousLocations.RemoveAt(0);
				}
				PreviousLocations.Add(PotentialLocation);
			}
			
			return true;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayerLocation: Failed to find valid location after %d attempts"), MaxAttempts);
	return false;
}

bool UBTTask_FindPlayerLocation::HasLocationBeenUsedRecently(const FVector& Location) const
{
	const float MinDistanceBetweenLocations = 200.0f;
	
	for (const FVector& PreviousLocation : PreviousLocations)
	{
		if (FVector::Dist(Location, PreviousLocation) < MinDistanceBetweenLocations)
		{
			return true; // Too close to a previously used location
		}
	}
	
	return false;
}

FVector UBTTask_FindPlayerLocation::ApplyDirectionalBias(AActor* PlayerActor, const FVector& OriginLocation, float Distance) const
{
	if (!PlayerActor)
	{
		return OriginLocation;
	}
	
	FVector Direction;
	FVector UpVector = FVector::UpVector;
	const FVector ForwardVector = PlayerActor->GetActorForwardVector();
	const FVector RightVector = PlayerActor->GetActorRightVector();
	
	switch (DirectionalPreference)
	{
		case EPositioningPreference::InFront:
			Direction = ForwardVector;
			break;
		case EPositioningPreference::Behind:
			Direction = -ForwardVector;
			break;
		case EPositioningPreference::ToLeft:
			Direction = -RightVector;
			break;
		case EPositioningPreference::ToRight:
			Direction = RightVector;
			break;
		case EPositioningPreference::Above:
			Direction = UpVector;
			break;
		case EPositioningPreference::Below:
			Direction = -UpVector;
			break;
		case EPositioningPreference::NoPreference:
		default:
			// Random direction
			Direction = FMath::VRand();
			break;
	}
	
	return OriginLocation + Direction * Distance;
}
