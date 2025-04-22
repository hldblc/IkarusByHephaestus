// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICompanionController.generated.h"

class UBehaviorTreeComponent;
class UCompanionComponent;

UCLASS()
class IKARUSTHECOMPANION_API AAICompanionController : public AAIController
{
	GENERATED_BODY()

public:
	AAICompanionController();

protected:

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Overriding Possess to acquire reference to CompanionComponent
	virtual void OnPossess(APawn* InPawn) override;

		
		
private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBlackboardComponent> BlackboardComponent;
	
};
