// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"


#include "Modules/ModuleManager.h"

#include "UnrealScenario.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(UnrealScenario, All, All);


class FUnrealScenario : public IModuleInterface
{
public:

	// This will get called when the editor loads the module
	virtual void StartupModule() override;

	// This will get called when the editor unloads the module
	virtual void ShutdownModule() override;
};


// forward declarations

class UConnectionElementComponent;

USTRUCT(BlueprintType, Category = "Util")
/**
List of materials used to store primitive materials.
*/
struct FMaterialList
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite, Category = "Util")
		TArray<UMaterialInterface*> SubMaterials;
};


UENUM(BlueprintType, Category = "Unreal Scenario")
/**
Flags used to highlight components in editing mode.
*/
enum class EEditFlags : uint8
{
	None = 0x00,
	Overlapping = 0x01,
	Connecting = 0x02,
	Selected = 0x04,
};

ENUM_CLASS_FLAGS(EEditFlags)


UENUM(BlueprintType, Category = "Unreal Scenario")
/**
Asset source type.
*/
enum class EModuleArrangement : uint8
{
	Linear = 0,
	Manhattan,
	Grid,
	Radial
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Information about socket used for physical (and logical) connections between elements.
*/
struct FSocketInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the socket.
        */
		FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the socket type (used to check compatibility).
        */
		FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        List of identifiers of compatible socket types.
        */
		TArray<FString> Compatibility;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        Scene component representing the socket.
        */
		USceneComponent* Component = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        True if the socket must be hidden (even if active).
        */
		bool bHidden = false;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Information for anchoring points.
*/
struct FConnectionAnchor
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the element to witch this anchor is connected.
        */
		FString ElementId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the socket to witch this anchor is connected.
        */
		FString SocketId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Offset of the anchor point relative to the connected socket.
        */
		FVector Offset;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        Scene component representing the anchor.
        */
		USceneComponent* Component = nullptr;
};


UENUM(BlueprintType, Category = "Unreal Scenario")
/**
Asset source type.
*/
enum class EAssetSource : uint8
{
	Scene = 0,
	Project,
	External,
	Undefined
};


USTRUCT(BlueprintType, Category = "Unreal Scenario|Catalog")
/**
Catalog item data.
*/
struct FCatalogItem
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Unique identifier of this item in the catalog.
        */
		FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Type of this itemù.
        */
		FString TypeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Category of this item, used to group items in the catalog.
        */
		FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Human readable name, displayed in the catalog.
        */
		FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Identifier of the icon representing this item.
        */
		FString Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Tooltip shown when the pointer is over this item.
        */
		FString ToolTip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Type of source containing the asset referred by this item.
        */
		EAssetSource AssetSource = EAssetSource::Undefined;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Identifier of the asset referred by this item.
        */
		FString AssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Type of the asset referred by this item.
        */
		FString AssetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        True if this item must be hidden in the catalog.
        */
		bool bHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        True if this item must be added only when no other item is present.
        */
		bool bFirstOne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
        /**
        Number of maximum instances allowed for this item (default = -1 = no limit).
        */
		int AllowedInstances = -1;

};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for agent statistics.
*/
struct FAgentStats
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        Number of completed episodes.
        */
		int EpisodesCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        Number of successful episodes.
        */
		int SuccessCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        Number of failed episodes.
        */
		int FailureCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario")
        /**
        Number of episodes come to a deadlock.
        */
		int DeadlockCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario")
		/**
		Rate of successful episodes on completed episodes.
		*/
		int SuccessRate = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario")
		/**
		Rate of successful episodes on completed episodes.
		*/
		int BestPerformance = 0;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for agent Learning configuration.
*/
struct FRLConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Value used to set initial values.
		*/
		float InitialValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Use the sample-average method for step size (learning rate) calculation.
		*/
		bool SampleAverage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario", meta = (UIMin = 0, UIMax = 1.0))
		/**
		Step-size parameter (learning rate).
		*/
		float FixedStepSize = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario", meta = (UIMin = 0, UIMax = 1.0))
		/**
		Discount rate.
		*/
		float DiscountRate = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario", meta = (UIMin = 0, UIMax = 1.0))
		/**
		Probability of taking a random action in an epsilon-greedy policy.
		*/
		float EpsilonGreedy = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		If set to a positive value (0..1) the Epsilon in epsilon-greedy policy
		is gradually reduced for each state by this factor,
		from 1 to the specified EpsilonGreedy value,
		if 1 (default) or negative no reduction is applied.
		*/
		float EpsilonReduction = -1.0f;
};


UENUM(BlueprintType)
/**
Specifier for set of entity states.
*/
enum class EEntitySpecifier : uint8
{
	Invalid = 0,
	Any,
	All
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Selector for set of entity states.
*/
struct FEntitySelector
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		EEntitySpecifier Specifier = EEntitySpecifier::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		FString TypeName;
};


UENUM(BlueprintType)
/**
Comparison operator selector.
*/
enum class EPropertyComparisonOperator : uint8
{
	Equal = 0,
	Different,
	Greater,
	GreaterEqual,
	Lesser,
	LesserEqual,
	Defined
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Base data structure for condition evaluation.
*/
struct FEntityConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the entity state to be checked.
        */
		FString EntityId;
        
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Type of entity state, used when ANY od ALL is specified as EntityId.
        */
		FString TypeName;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for property condition evaluation.
*/
struct FPropertyCondition : public FEntityConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the property to be checked.
        */
		FString PropertyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Value checked for the specified property.
        */
		FString PropertyValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Operator used to compare the given property with the given value.
        */
		EPropertyComparisonOperator ComparisonOperator;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for entity condition evaluation.
*/
struct FRelationshipCondition : public FEntityConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the link of the entity to be checked.
        */
		FString LinkId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the entity to be checked as related entity.
        */
		FString RelatedEntityId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Identifier of the link of the specified entity to be checked as related link.
        */
		FString RelatedLinkId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        If true invert the condition evaluation (true if not related).
        */
		bool Unrelated = false;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Base data structure for any condition reward.
*/
struct FConditionRewardBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		int Reward = 0;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for property condition reward.
*/
struct FPropertyConditionReward : public FConditionRewardBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		FPropertyCondition Condition;
};


USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for property condition reward.
*/
struct FRelationshipConditionReward : public FConditionRewardBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		FRelationshipCondition Condition;
};



USTRUCT(BlueprintType, Category = "Unreal Scenario")
/**
Data structure for scenario rewards.
*/
struct FScenarioRewards
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Reward for a successful episode.
        */
		int SuccessReward = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Reward for a failed episode.
        */
		int FailureReward = -100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Reward for an episode come to a deadlock.
        */
		int DeadlockReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Reward proportional to the number of actions  already taken.
        */
		int InProgressReward = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Rewards added each time a specified condition is satisfied.
        */
		TArray<FPropertyConditionReward> PropertyCountRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Rewards for some specified conditions on properties.
        */
		TArray<FPropertyConditionReward> EntityPropertyRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
        /**
        Rewards for some specified conditions on relationships.
        */
		TArray<FRelationshipConditionReward> EntityRelationshipRewards;
};


