// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "EntityDataComponent.h"
#include "UnrealScenario.h"


#include "ElementDataComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = UnrealScenario, meta = (BlueprintSpawnableComponent))
/**
 * Component that holds data for a scenario element.
 */
class UNREALSCENARIO_API UElementDataComponent : public UEntityDataComponent
{
	GENERATED_BODY()

public:

	UElementDataComponent();

	UPROPERTY(AdvancedDisplay, EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Sockets used for connections.
		*/
		TArray< FSocketInfo > SocketComponents;


	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Scan sockets in scene components.
		*/
		void ScanSockets();

	/**
	Get the information of the socket with the given identifier.
	*/
	FSocketInfo* GetSocketInfo(FString Id);

	/**
	Get the primitive component representing the socket with the given identifier.
	*/
	UPrimitiveComponent* GetSocketComponent(FString Id);

	/**
	Find a socket identifier given a primitive component.
	*/
	FString FindSocketIdByComponent(USceneComponent* PrimComp);

	/**
	Get the information of the socket represented by the given primitive component.
	*/
	FSocketInfo* GetSocketInfoByComponent(UPrimitiveComponent* PrimComp);

	/**
	Set the given primitive component highlighted/normal with the given mode.
	*/
	void HighlightSocket(UPrimitiveComponent* PrimComp, bool Outline, EEditFlags Mode);

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
};
