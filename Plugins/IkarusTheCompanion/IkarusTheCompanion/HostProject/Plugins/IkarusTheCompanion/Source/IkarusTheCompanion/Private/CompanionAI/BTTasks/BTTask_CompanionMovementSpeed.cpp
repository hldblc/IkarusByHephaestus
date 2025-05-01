#include "CompanionAI/BTTasks/BTTask_CompanionMovementSpeed.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "CompanionAI/IkarusCharacter.h"
#include "CompanionAI/CompanionControllers/AICompanionController.h"

UBTTask_CompanionMovementSpeed::UBTTask_CompanionMovementSpeed(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NodeName = "Companion Movement Speed";

    CompanionProximityKey.AddFloatFilter(this,
        GET_MEMBER_NAME_CHECKED(UBTTask_CompanionMovementSpeed, CompanionProximityKey));
    DebugSpeedKey.AddFloatFilter(this,
        GET_MEMBER_NAME_CHECKED(UBTTask_CompanionMovementSpeed, DebugSpeedKey));
}

EBTNodeResult::Type UBTTask_CompanionMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                               uint8* /*NodeMemory*/)
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

ECompanionMovementSpeed UBTTask_CompanionMovementSpeed::ProximityToEnum(float P)
{
    if (P > 0.75f) return ECompanionMovementSpeed::Idle;
    if (P > 0.35f) return ECompanionMovementSpeed::Walking;
    if (P > 0.15f) return ECompanionMovementSpeed::Running;
    return              ECompanionMovementSpeed::Sprinting;
}
