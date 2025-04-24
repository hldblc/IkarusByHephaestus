// Fill out your copyright notice in the Description page of Project Settings.

#include "CompanionCore/CoreStructs/CompanionCoreStructs.h"

FFollowPlayerMemory::FFollowPlayerMemory()
	: LastTargetLocation(FVector::ZeroVector)
	, LastPathUpdateTime(0.0f)
	, LastStuckCheckTime(0.0f)
	, LastCheckedPawnLocation(FVector::ZeroVector)
	, bIsFollowing(false)
{
	CurrentRequestID = FAIRequestID::InvalidRequest;
}