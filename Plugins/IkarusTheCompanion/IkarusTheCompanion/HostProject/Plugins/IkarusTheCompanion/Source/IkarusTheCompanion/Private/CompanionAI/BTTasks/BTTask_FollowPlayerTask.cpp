// BTTask_FollowPlayerTask.cpp – Ikarus Companion

#include "CompanionAI/BTTasks/BTTask_FollowPlayerTask.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_FollowPlayerTask::UBTTask_FollowPlayerTask(const FObjectInitializer& ObjInit)
	: Super(ObjInit)
{
	NodeName = TEXT("Follow Player Task");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true; // Important for proper task instancing

	BlackboardKey.AddVectorFilter(
		this, GET_MEMBER_NAME_CHECKED(UBTTask_FollowPlayerTask, BlackboardKey));
}

EBTNodeResult::Type UBTTask_FollowPlayerTask::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* Controller = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!Controller)
	{
		return EBTNodeResult::Failed;
	}

	// Get target from blackboard
	CachedTarget = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());

	// Start the move request
	if (StartMoveRequest(Controller, CachedTarget))
	{
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

void UBTTask_FollowPlayerTask::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Only update if we're actively moving
	if (!bIsMoveActive)
	{
		return;
	}

	auto* Controller = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!Controller)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FVector NewTarget = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());

	// Check if the target has moved beyond our threshold
	if (FVector::DistSquared(NewTarget, CachedTarget) > FMath::Square(RepathThreshold))
	{
		CachedTarget = NewTarget;
		
		// Request a new move with the updated target
		FAIMoveRequest MoveRequest(CachedTarget);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		MoveRequest.SetAllowPartialPath(bAllowPartialPath);
		MoveRequest.SetUsePathfinding(true);
		MoveRequest.SetProjectGoalLocation(true);
		MoveRequest.SetCanStrafe(bCanStrafe);
		MoveRequest.SetNavigationFilter(NavFilter);

		// Stop current movement and start new request
		if (bIsMoveActive && CurrentMoveRequestID.IsValid())
		{
			Controller->StopMovement();
		}
		
		// Start new move request
		CleanupDelegates(Controller);
		Controller->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_FollowPlayerTask::HandleMoveFinished);
		CurrentMoveRequestID = Controller->MoveTo(MoveRequest);
		bIsMoveActive = CurrentMoveRequestID.IsValid();
	}
}

void UBTTask_FollowPlayerTask::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (auto* Controller = Cast<AAIController>(OwnerComp.GetAIOwner()))
	{
		CleanupDelegates(Controller);
	}
	
	bIsMoveActive = false;
	CurrentMoveRequestID = FAIRequestID();
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

EBTNodeResult::Type UBTTask_FollowPlayerTask::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* Controller = Cast<AAIController>(OwnerComp.GetAIOwner()))
	{
		if (bIsMoveActive && CurrentMoveRequestID.IsValid())
		{
			Controller->StopMovement();
		}
		CleanupDelegates(Controller);
	}
	
	bIsMoveActive = false;
	CurrentMoveRequestID = FAIRequestID();
	
	return EBTNodeResult::Aborted;
}

void UBTTask_FollowPlayerTask::HandleMoveFinished(
	FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	// Only handle if this is our current request
	if (RequestID != CurrentMoveRequestID)
	{
		return;
	}

	bIsMoveActive = false;
	
	// Notify the behavior tree we're done
	if (auto* BehaviorComp = Cast<UBehaviorTreeComponent>(GetOuter()))
	{
		FinishLatentTask(*BehaviorComp, 
			Result == EPathFollowingResult::Success ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}
}

bool UBTTask_FollowPlayerTask::StartMoveRequest(AAIController* Controller, const FVector& Target)
{
	if (!Controller || bIsMoveActive)
	{
		return false;
	}

	// Clean up any existing delegates first
	CleanupDelegates(Controller);

	// Setup move request
	FAIMoveRequest MoveRequest(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetAllowPartialPath(bAllowPartialPath);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetProjectGoalLocation(true);
	MoveRequest.SetCanStrafe(bCanStrafe);
	MoveRequest.SetNavigationFilter(NavFilter);

	// Bind to completion delegate
	Controller->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_FollowPlayerTask::HandleMoveFinished);
	
	// Start the move
	CurrentMoveRequestID = Controller->MoveTo(MoveRequest);
	bIsMoveActive = CurrentMoveRequestID.IsValid();
	
	return bIsMoveActive;
}

void UBTTask_FollowPlayerTask::CleanupDelegates(AAIController* Controller)
{
	if (Controller)
	{
		Controller->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_FollowPlayerTask::HandleMoveFinished);
	}
}