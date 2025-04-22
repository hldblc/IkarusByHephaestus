// Copyright Epic Games, Inc. All Rights Reserved.

#include "IkarusTheCompanion/Public/CompanionAI/CompanionControllers/AICompanionController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

// Sets default values
AAICompanionController::AAICompanionController()
{
	PrimaryActorTick.bCanEverTick = true;
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

// Called when the game starts or when spawned
void AAICompanionController::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(BehaviorTree))
	{
		
	}
}






// When possessing a pawn, try to find and cache the CompanionComponent
void AAICompanionController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	
}

// Called every frame
void AAICompanionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


