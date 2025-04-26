#include "CompanionAI/BTServices/BTService_UpdatePlayerLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/PlayerController.h"

UBTService_UpdatePlayerLocation::UBTService_UpdatePlayerLocation()
{
	NodeName        = TEXT("Update Player Location (Optimised)");
	Interval        = 0.15f;          // 6–7 × per second near player
	RandomDeviation = 0.f;

	// Set up key filters
	PlayerLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdatePlayerLocation, PlayerLocationKey));
	PlayerActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdatePlayerLocation, PlayerActorKey), AActor::StaticClass());
}

void UBTService_UpdatePlayerLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (const UBlackboardData* BB = GetBlackboardAsset())
	{
		LocKeyId = BB->GetKeyID(PlayerLocationKey.SelectedKeyName);
		ActKeyId = BB->GetKeyID(PlayerActorKey.SelectedKeyName);
	}

	MoveThresholdSq = FMath::Square(MoveThreshold);
}

void UBTService_UpdatePlayerLocation::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMem*/, float /*Delta*/)
{
	/* ---------- authority & replay gating ---------- */
	const bool bIsReplay = OwnerComp.GetWorld()->IsPlayingReplay();
	if ((!bRunOnClients && !OwnerComp.GetOwner()->HasAuthority()) && ! (bIsReplay && bRunWhileReplaying))
	{
		return;
	}

	/* ---------- blackboard sanity ---------- */
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB || LocKeyId == FBlackboard::InvalidKey)
	{
		return;
	}

	/* ---------- resolve / cache player actor ---------- */
	AActor* Player = nullptr;

	if (ActKeyId != FBlackboard::InvalidKey)
	{
		Player = Cast<AActor>(BB->GetValueAsObject(PlayerActorKey.SelectedKeyName));
	}

	if (!Player || !Player->IsValidLowLevel())
	{
		Player = CachedPlayer.Get();
	}

	if (!Player)
	{
		if (const APlayerController* PC = OwnerComp.GetWorld()->GetFirstPlayerController())
		{
			Player = PC->GetPawn();
			CachedPlayer = Player;
		}
	}
	if (!Player) return;

	const FVector NewLoc = Player->GetActorLocation();

	/* ---------- distance gate ---------- */
	if (const APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr)
	{
		if (FVector::DistSquared(Pawn->GetActorLocation(), NewLoc) >
		    FMath::Square(MaxFollowRange))
		{
			return;                    // too far – skip update
		}
	}

	/* ---------- line-of-sight gate (optional but cheap) ---------- */
	if (AAIController* Ctrl = Cast<AAIController>(OwnerComp.GetAIOwner()))
	{
		if (UAIPerceptionComponent* Perc = Ctrl->GetPerceptionComponent())
		{
			FActorPerceptionBlueprintInfo Info;
			if (Perc->GetActorsPerception(Player, Info))
			{
				bool bSaw = false;
				// Check if we have sight stimuli
				for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
				{
					if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
					{
						bSaw = Stimulus.WasSuccessfullySensed();
						break;
					}
				}
				if (!bSaw) return;      // no LOS – let other tasks handle
			}
		}
	}

	/* ---------- threshold gate & write ---------- */
	if (FVector::DistSquared(NewLoc, LastWritten) >= MoveThresholdSq)
	{
		BB->SetValueAsVector(PlayerLocationKey.SelectedKeyName, NewLoc);
		LastWritten = NewLoc;
	}
}