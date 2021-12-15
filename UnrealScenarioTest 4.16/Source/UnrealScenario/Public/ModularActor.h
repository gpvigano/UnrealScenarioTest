// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "MovableActor.h"
#include "UnrealScenario.h"

#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"

#include "ModularActor.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
 * A modular actor composed by replicated parts.
 */
class UNREALSCENARIO_API AModularActor : public AMovableActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Position at which the module replication starts.
        */
		FVector StartPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Position at which the module replication ends.
        */
		FVector EndPosition = FVector(0.f,100.f,0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Template blueprint used to create modules.
        */
		TSubclassOf<AActor> ModuleBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Forward axis for the template blueprint used to create modules (default: None=autodetect).
        */
		TEnumAsByte<EAxis::Type> ModuleAxis = EAxis::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Arrangement of the modules.
		@note Only Linear and Manhattan are currently implemented.
		@todo Implement all types of EModuleArrangement.
        */
	EModuleArrangement ModuleArrangement = EModuleArrangement::Linear;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
		/**
        Replicated modules.
        */
		TArray<AActor*> Modules;



	AModularActor();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
        Update modules according to start and end positions.
        */
		void UpdateModules();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
        Rebuild modules according to the given template and start/end positions.
        */
		void RebuildModules()
	{
		Initialize(true);
		UpdateModules();
	}

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
        Get the legth of the a module along its main axis.
        */
		float GetModuleLength()
	{
		return ModuleLength;
	}

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
        Get the position after the last module.
        */
		FVector GetModulesEndPosition()
	{
		return ModulesEndPosition;
	}

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
        Get the rotation after the last module.
        */
		FQuat GetModulesEndRotation()
	{
		return ModulesEndRotation;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	bool bInitialized = false;
	float ModuleLength = 0.0f;
	float ModuleWidth = 0.0f;
	FVector ModuleOffset;
	FVector ModuleAxisVector;
	FVector ModuleSideVector;

	FVector ModulesEndPosition;
	FQuat ModulesEndRotation;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool Initialize(bool bRebuild);

	AActor* CreateModule();
	void SetNumModules(int32 NumModules);
	void DestroyModules();
};
