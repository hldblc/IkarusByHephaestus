
#include "CompanionAI/BTTasks/SetMovementSpeed.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "CompanionAI/IkarusCharacter.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"

USetMovementSpeed::USetMovementSpeed(const FObjectInitializer& ObjectInitializer)
{
	NodeName = "Companion Movement Speed";

	CompanionProximityKey.AddFloatFilter(this,
		GET_MEMBER_NAME_CHECKED(USetMovementSpeed, CompanionProximityKey));
	DebugSpeedKey.AddFloatFilter(this,
		GET_MEMBER_NAME_CHECKED(USetMovementSpeed, DebugSpeedKey));
}

EBTNodeResult::Type USetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* Controller = Cast<AAICompanionController>(OwnerComp.GetAIOwner()))
	{
		if (auto* Companion = Cast<AIkarusCharacter>(Controller->GetPawn()))
		{
			ECompanionMovementSpeed DesiredSpeed = CompanionSpeed;

			if (bAutomaticFromDistance)
			{
				const float Prox =
					OwnerComp.GetBlackboardComponent()->GetValueAsFloat(
						CompanionProximityKey.SelectedKeyName);
				DesiredSpeed = ProximityToEnum(Prox);
			}

			float OutSpeed = 0.f;
			Companion->SetMovementSpeed(DesiredSpeed, OutSpeed);

			if (DebugSpeedKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsFloat(
					DebugSpeedKey.SelectedKeyName, OutSpeed);
			}

			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}

ECompanionMovementSpeed USetMovementSpeed::ProximityToEnum(float Proximity)
{
	if (Proximity > 0.75f) return ECompanionMovementSpeed::Idle;
	if (Proximity > 0.35f) return ECompanionMovementSpeed::Walking;
	if (Proximity > 0.15f) return ECompanionMovementSpeed::Running;
	return              ECompanionMovementSpeed::Sprinting;
	
}
