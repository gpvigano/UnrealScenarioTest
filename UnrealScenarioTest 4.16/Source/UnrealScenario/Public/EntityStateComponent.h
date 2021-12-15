// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EntityStateComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = UnrealScenario, meta = (BlueprintSpawnableComponent))
/**
Base component used to translate entity states into representations in the VE.
*/
class UNREALSCENARIO_API UEntityStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		List of properties with their value.
		*/
		TMap<FString, FString> Properties;

	// Sets default values for this component's properties
	UEntityStateComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the value of a property.
		*/
		FString GetProperty(FString PropertyName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Check if a property is defined and has the given value.
		*/
		bool CheckProperty(FString PropertyName, FString PropertyValue) const;

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Find a scene component in the owner actor.
		*/
		USceneComponent* FindSceneComponent(FString ChildName);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Apply the given property with its value.
		*/
		void ApplyProperty(FString PropertyName, FString PropertyValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Unreal Scenario")
		/**
		Apply the currently defined entity state.
		*/
		void ApplyState();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
