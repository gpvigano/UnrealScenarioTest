// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ElementDataComponent.h"
#include "ConnectionElementComponent.h"
#include "Logger.h"
#include "UnrealScenarioActor.h"
#include "Engine/World.h" // GetWorld()




#include "UScenUtil.generated.h"

UCLASS()
/**
 * Unreal Scenario Blueprint Utility Library
 */
class UNREALSCENARIO_API UUScenUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	Tag used to identify sockets in scene components.
	*/
	static FName SocketTag;

	/**
	Tag used to identify gizmos in scene components.
	*/
	static FName GizmoTag;

	/**
	Tag used to mark actors as overlapping.
	*/
	static FName OverlappingTag;



	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Look for an AUnrealScenarioActor in the given world, return nullptr if not found.
		*/
		static AUnrealScenarioActor* FindUnrealScenario(UWorld* World);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the tag used to denote all entities (defined in the Digital Scenario Framework).
		*/
		static FString GetAllEntitiesTag();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the tag used to denote any entity (defined in the Digital Scenario Framework).
		*/
		static FString GetAnyEntityTag();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Check if the given value is a special tag (all/any)
		*/
		static bool IsSpecialEntityTag(const FString& Tag);


	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Retrieve the entity state type from the identifier of an entity.
		*/
		static FString GetEntityStateType(FString EntityId);


	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get a list of all entity state types.
		*/
		static TArray<FString> GetEntityStateTypeNames();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get a list of all the names of properties defined in all the entity state types.
		*/
		static TArray<FString> GetAllDefinedPropertyNames();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get a list of the names of properties defined in the given entity state type.
		*/
		static TArray<FString> GetDefinedPropertyNames(const FString& TypeName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get a list of all the predefined values of the given property for all the entity state types.
		*/
		static TArray<FString> GetAllDefinedPropertyValues(const FString& PropertyName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get a list of predefined values of the given property for the given entity state type.
		*/
		static TArray<FString> GetDefinedPropertyValues(const FString& PropertyName, const FString& TypeName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get a list of link identifiers defined in the given entity state type.
		*/
		static TArray<FString> GetEntityTypeLinks(const FString& TypeName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Retrieve the list of link identifiers from the identifier of an entity.
		*/
		static TArray<FString> GetEntityStateLinks(const FString& EntityStateId);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Access the Digital Scenario Framework to compute the constant used by DigitalAssistant to calculate discounted returns.
		*/
		static float ComputeDiscountingOrGainConstant(int SingleStepReward, int EpisodeReward);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the entity data component of the given actor.
		*/
		static UEntityDataComponent* GetEntityData(AActor* EntityActor);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the element data component of the given actor.
		*/
		static UElementDataComponent* GetElementData(AActor* EntityActor);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the connection element component of the given actor.
		*/
		static UConnectionElementComponent* GetConnectionElementData(AActor* EntityActor);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the identifier of the entity data component of the given actor.
		*/
		static FString GetEntityIdentifier(AActor* EntityActor);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the actoror parent actor of the given actor with an entity data component.
		*/
		static AActor* GetEntityFromActor(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Compute the world position of the given anchor.
		*/
		static FVector GetAnchorWorldPosition(const FConnectionAnchor& Anchor);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Compute the position of the given anchor relative to the given scene component.
		*/
		static FVector GetAnchorLocalPosition(const FConnectionAnchor& Anchor, USceneComponent* ReferenceComponent);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Set the position of the given anchor relative to the given scene component.
		*/
		static void SetAnchorRelativePosition(FConnectionAnchor& Anchor, const FVector& Position, USceneComponent* ReferenceComponent);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Check if two sockets can be connected.
		*/
		static bool AreSocketsCompatible(const FSocketInfo& ConnectingSocket, const FSocketInfo& AcceptingSocket);

	/**
	Print a log message on screen and/or console.
	@param Level Level of severity (see ELogLevel).
	@param Message Message to be printed.
	@param bToConsole Print the message on console.
	@param bToScreen Print the message on screen.
	@param Key Optional unique key to overwrite the same message multiple times.
	*/
	static void Log(
		Logger::ELogLevel Level,
		FString Message,
		bool bToConsole = true,
		bool bToScreen = true,
		FString Key = "");


};
