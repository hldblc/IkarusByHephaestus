// BTService_UpdateTaskUtility.cpp
#include "CompanionAI/BTServices/BTService_UpdateTaskUtility.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"
#include "CompanionAI/Components/CompanionTaskComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateTaskUtility::UBTService_UpdateTaskUtility()
{
	NodeName = "Update Task Utility";
	Interval = 1.0f;
	RandomDeviation = 0.2f;
}

void UBTService_UpdateTaskUtility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
	// Get AI controller
	AAICompanionController* Controller = Cast<AAICompanionController>(OwnerComp.GetAIOwner());
	if (!Controller)
	{
		return;
	}
    
	// Get task component
	UCompanionTaskComponent* TaskComp = Controller->GetCompanionTaskComponent();
	if (!TaskComp)
	{
		return;
	}
    
	// Get blackboard component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}
    
	// Update utility scores for each task type
	BlackboardComp->SetValueAsFloat(FName(IdleScoreKey), TaskComp->CalculateTaskUtility(ECompanionTask::Idle));
	BlackboardComp->SetValueAsFloat(FName(FollowScoreKey), TaskComp->CalculateTaskUtility(ECompanionTask::Follow));
	BlackboardComp->SetValueAsFloat(FName(PatrolScoreKey), TaskComp->CalculateTaskUtility(ECompanionTask::Patrol));
	BlackboardComp->SetValueAsFloat(FName(GatherScoreKey), TaskComp->CalculateTaskUtility(ECompanionTask::Gather));
	BlackboardComp->SetValueAsFloat(FName(SearchScoreKey), TaskComp->CalculateTaskUtility(ECompanionTask::Search));
}