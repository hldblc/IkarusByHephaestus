#pragma once

#include "CoreMinimal.h"
#include "CompanionEnums.generated.h"

UENUM(BlueprintType)
enum class ECompanionMovementSpeed : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Climbing UMETA(DisplayName = "Climbing"),
	Jumping UMETA(DisplayName = "Jumping"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Swimming UMETA(DisplayName = "Swimming"),
	Flying UMETA(DisplayName = "Flying"),
	Teleporting UMETA(DisplayName = "Teleporting")
	
};

/**
 * Optimized companion system enums for advanced AI behavior
 * All enums use uint8 for maximum memory efficiency
 */

/** Current activity or behavior state of the companion */
UENUM(BlueprintType)
enum class ECompanionTask : uint8
{
	None    UMETA(DisplayName = "None"),
	Idle    UMETA(DisplayName = "Idle"),
	Follow  UMETA(DisplayName = "Follow Player"),
	Patrol  UMETA(DisplayName = "Patrol Area"),
	Gather  UMETA(DisplayName = "Gather Resources"),
	Search  UMETA(DisplayName = "Search Area"),
	Combat  UMETA(DisplayName = "Combat"),
	Healing UMETA(DisplayName = "Healing"),
	Exploring UMETA(DisplayName = "Exploring"),
	UsingObject UMETA(DisplayName = "Using Object"),
	Custom  UMETA(DisplayName = "Custom Task")
};

/** Current emotional state of the companion that influences behavior and interactions */
UENUM(BlueprintType)
enum class ECompanionMood : uint8
{
	Neutral	 UMETA(DisplayName = "Neutral"),
	Cheerful UMETA(DisplayName = "Cheerful"),
	Concerned UMETA(DisplayName = "Concerned"),
	Angry	 UMETA(DisplayName = "Angry"),
	Exhausted UMETA(DisplayName = "Exhausted"),
	Afraid   UMETA(DisplayName = "Afraid"),
	Surprised UMETA(DisplayName = "Surprised"),
	Confused UMETA(DisplayName = "Confused"),
	Curious  UMETA(DisplayName = "Curious"),
	Determined UMETA(DisplayName = "Determined")
};

/** Base class or role that defines companion's primary function and abilities */
UENUM(BlueprintType)
enum class ECompanionType : uint8
{
	Pet		 UMETA(DisplayName = "Pet"),
	Guardian UMETA(DisplayName = "Guardian"),
	Worker	 UMETA(DisplayName = "Worker"), 
	Scout	 UMETA(DisplayName = "Scout"),
	Healer	 UMETA(DisplayName = "Healer"),
	Fighter	 UMETA(DisplayName = "Fighter")
};

/** Core attributes that determine companion capabilities and performance */
UENUM(BlueprintType)
enum class ECompanionStat : uint8
{
	Health	  UMETA(DisplayName = "Health"),
	Stamina	 UMETA(DisplayName = "Stamina"),
	Loyalty	 UMETA(DisplayName = "Loyalty"),
	Intelligence UMETA(DisplayName = "Intelligence"),
	Strength	UMETA(DisplayName = "Strength"),
	Speed	   UMETA(DisplayName = "Speed")
};

/** Level of bond between companion and player that affects loyalty and available interactions */
UENUM(BlueprintType)
enum class ECompanionRelationship : uint8
{
	Stranger	UMETA(DisplayName = "Stranger"),
	Acquaintance UMETA(DisplayName = "Acquaintance"),
	Friend	  UMETA(DisplayName = "Friend"),
	Loyal	   UMETA(DisplayName = "Loyal"),
	Bonded	  UMETA(DisplayName = "Bonded")
};

/** Personality characteristics that influence companion behavior and decision making */
UENUM(BlueprintType)
enum class ECompanionTrait : uint8
{
	Brave	   UMETA(DisplayName = "Brave"),
	Cautious	UMETA(DisplayName = "Cautious"),
	Resourceful UMETA(DisplayName = "Resourceful"),
	Protective  UMETA(DisplayName = "Protective"),
	Curious	 UMETA(DisplayName = "Curious"),
	Aggressive  UMETA(DisplayName = "Aggressive"),
	Docile	  UMETA(DisplayName = "Docile")
};

/** Outcome of interactions between companion and other entities */
UENUM(BlueprintType)
enum class EInteractionResult : uint8
{
	Success	 UMETA(DisplayName = "Success"),
	Failure	 UMETA(DisplayName = "Failure"),
	Neutral	 UMETA(DisplayName = "Neutral"),
	Exceptional UMETA(DisplayName = "Exceptional")
};

/** Positioning preference relative to a target (typically the player) */
UENUM(BlueprintType)
enum class EPositioningPreference : uint8
{
	NoPreference UMETA(DisplayName = "No Preference"),
	InFront UMETA(DisplayName = "In Front"),
	Behind UMETA(DisplayName = "Behind"),
	ToLeft UMETA(DisplayName = "To Left"),
	ToRight UMETA(DisplayName = "To Right"),
	Above UMETA(DisplayName = "Above"),
	Below UMETA(DisplayName = "Below")
};

/** Types of awareness for companion perception */
UENUM(BlueprintType)
enum class EAwarenessType : uint8
{
	None UMETA(DisplayName = "No Awareness"),
	Curious UMETA(DisplayName = "Curious"),
	Alert UMETA(DisplayName = "Alert"),
	Threatened UMETA(DisplayName = "Threatened"),
	Danger UMETA(DisplayName = "Immediate Danger"),
	Resource UMETA(DisplayName = "Resource Available"),
	PlayerNeeds UMETA(DisplayName = "Player Needs")
};

/** Priority levels for companion decision making */
UENUM(BlueprintType)
enum class ECompanionPriority : uint8
{
	Trivial = 0 UMETA(DisplayName = "Trivial"),
	Low = 25 UMETA(DisplayName = "Low"),
	Medium = 50 UMETA(DisplayName = "Medium"),
	High = 75 UMETA(DisplayName = "High"),
	Critical = 100 UMETA(DisplayName = "Critical")
};

/** Distance zones for companion positioning relative to player or targets */
UENUM(BlueprintType)
enum class EDistanceZone : uint8
{
	Intimate UMETA(DisplayName = "Intimate (0-1m)"),
	Personal UMETA(DisplayName = "Personal (1-2m)"),
	Social UMETA(DisplayName = "Social (2-5m)"),
	Public UMETA(DisplayName = "Public (5-10m)"),
	Distant UMETA(DisplayName = "Distant (10m+)"),
	Custom UMETA(DisplayName = "Custom Range")
};

/** Traversal methods available to the companion AI */
UENUM(BlueprintType)
enum class ETraversalMethod : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Climbing UMETA(DisplayName = "Climbing"),
	Jumping UMETA(DisplayName = "Jumping"),
	Swimming UMETA(DisplayName = "Swimming"),
	Flying UMETA(DisplayName = "Flying"),
	Teleporting UMETA(DisplayName = "Teleporting")
};

/** Item preferences for inventory management and gathering */
UENUM(BlueprintType)
enum class EItemPreference : uint8
{
	Ignore UMETA(DisplayName = "Ignore"),
	Low UMETA(DisplayName = "Low Priority"),
	Medium UMETA(DisplayName = "Medium Priority"),
	High UMETA(DisplayName = "High Priority"), 
	Critical UMETA(DisplayName = "Critical Priority")
};

/** Types of interactions between companion and environment/player */
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None UMETA(DisplayName = "No Interaction"),
	Examine UMETA(DisplayName = "Examine"),
	Pickup UMETA(DisplayName = "Pick Up"),
	Use UMETA(DisplayName = "Use"),
	Give UMETA(DisplayName = "Give To Player"),
	Speak UMETA(DisplayName = "Speak About"),
	Point UMETA(DisplayName = "Point At"),
	Avoid UMETA(DisplayName = "Avoid")
};