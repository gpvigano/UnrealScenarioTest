// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"



#include "AssetIndexComponent.generated.h"


UCLASS(ClassGroup = UnrealScenario, meta = (BlueprintSpawnableComponent))
/**
Component holding a list of asset references, indexed with identifiers.
*/
class UNREALSCENARIO_API UAssetIndexComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAssetIndexComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indices")
		/**
		Hide indexed scene actors.
		*/
		bool bHideSceneActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indices")
		/**
		Indexed scene actors.
		*/
		TMap<FString, AActor*> SceneActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indices")
		/**
		Indexed actor blueprints.
		*/
		TMap< FString, TSubclassOf<AActor> > ActorBlueprints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indices")
		/**
		Indexed textures.
		*/
		TMap<FString, UTexture2D*> Textures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indices")
		/**
		Indexed materials.
		*/
		TMap<FString, UMaterialInterface*> Materials;




	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Duplicate the given (named) actor in the scene
		*/
		AActor* CreateActorFromSceneActor(const FString& AssetId, FString ActorName = TEXT(""), AActor* parentActor = nullptr);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Get the given (named) actor in the scene
		*/
		AActor* GetSceneActorById(const FString& AssetId);

	UFUNCTION(BlueprintCallable, Category = "Util")
		/**
		Create a new actor from the given (named) asset
		*/
		AActor* CreateActorFromAsset(const FString& AssetId, FString ActorName = TEXT(""), AActor* parentActor = nullptr);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Check if the given (named) actor is defined and not null
		*/
		bool IsSceneActorDefined(const FString& ActorId) const;

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Check if the given (named) actor blueprint is defined and not null
		*/
		bool IsActorBlueprintDefined(const FString& AssetId) const;

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Get the given (named) texture
		*/
		UTexture2D* GetTextureById(const FString& AssetId);

	UFUNCTION(BlueprintPure, Category = "Util")
		/**
		Get the given (named) material
		*/
		UMaterialInterface* GetMaterialById(const FString& AssetId);
};
