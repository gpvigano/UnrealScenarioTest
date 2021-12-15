// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "AssetIndexComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"

#include "UEUtil.h"
#include "UScenUtil.h"


// Sets default values for this component's properties
UAssetIndexComponent::UAssetIndexComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;//true;

	// ...
}


// Called when the game starts
void UAssetIndexComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TPair<FString, AActor*>& entityActor : SceneActors)
	{
		if(entityActor.Value)
		{
			UUEUtil::SetActorActive(entityActor.Value,false);
		}
		else
		{
			UUEUtil::Log.LogMessage(Logger::ERROR, TEXT("Actor not defined: ") + entityActor.Key, TEXT("AssetIndex"));
		}
	}

}


// Called every frame
void UAssetIndexComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


AActor* UAssetIndexComponent::CreateActorFromSceneActor(const FString& AssetId, FString actorName, AActor* parentActor)
{
	if (!SceneActors.Contains(AssetId))
	{
		UUEUtil::Log.LogMessage(Logger::WARNING, TEXT("No actor found for ") + AssetId, TEXT("AssetIndex"));
		return nullptr;
	}
	AActor* sceneActor = SceneActors[AssetId];
	if (sceneActor == nullptr)
	{
		UUEUtil::Log.LogMessage(Logger::WARNING, TEXT("Null actor found for ") + AssetId, TEXT("AssetIndex"));
		return nullptr;
	}
	UWorld* world = GetWorld();
	AActor* newActor = nullptr;
	FActorSpawnParameters spawnParams;
	spawnParams.Template = sceneActor;
	////////if (entityName == sceneActor->GetName())
	////if (StaticFindObjectFast(nullptr, world, UUEUtil::ToFName( entityName)))
	////{
	////	UUEUtil::Log.LogMessage({ Logger::WARNING, "Trying to spawn two objects with the same name.", "UnrealScenario" });
	////}
	////else if (!entityName.IsEmpty())
	////{
	////	spawnParams.Name = FName(*entityName);
	////}
	newActor = world->SpawnActor<AActor>(sceneActor->GetClass(), spawnParams);
	//newActor = CastChecked<AEntityActor>( UUEUtil::CreateCloneOfActor(sceneActor, entityName, parentActor) );
	if (parentActor)
	{
		newActor->AttachToActor(parentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}
#if WITH_EDITOR
	if(!actorName.IsEmpty())
	{
		newActor->SetActorLabel(*actorName);
	}
#endif
	UUEUtil::SetActorActive(newActor, true);
	return newActor;
}

AActor* UAssetIndexComponent::GetSceneActorById(const FString& AssetId)
{
	if (!SceneActors.Contains(AssetId))
	{
		return nullptr;
	}
	return SceneActors[AssetId];
}


AActor* UAssetIndexComponent::CreateActorFromAsset(const FString& AssetId, FString actorName, AActor* parentActor)
{
	if (!ActorBlueprints.Contains(AssetId))
	{
		UUEUtil::Log.LogMessage(Logger::WARNING, TEXT("No asset found for ") + AssetId, TEXT("AssetIndex"));
		return nullptr;
	}

	TSubclassOf<AActor> actorClass = ActorBlueprints[AssetId];
	if (!actorClass)
	{
		UUEUtil::Log.LogMessage(Logger::WARNING, TEXT("Null asset found for ") + AssetId, TEXT("AssetIndex"));
		return nullptr;
	}
	UWorld* world = GetWorld();
	AActor* newActor = nullptr;
	if (!actorName.IsEmpty())
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Name = FName(*actorName);
		newActor = world->SpawnActor<AActor>(actorClass, spawnParams);
#if WITH_EDITOR
		newActor->SetActorLabel(*actorName);
#endif
	}
	else
	{
		newActor = world->SpawnActor<AActor>(actorClass);
	}
	if (parentActor)
	{
		newActor->AttachToActor(parentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}
	UUEUtil::SetActorActive(newActor, true);
	return newActor;
}


bool UAssetIndexComponent::IsSceneActorDefined(const FString& ActorId) const
{
	if (!SceneActors.Contains(ActorId))
	{
#if WITH_EDITOR
		// TODO: workaround for a bug: when changing a key in the Editor the map hash function is not updated and the new name of the key is not found
		// search the key in the map
		for (const auto& actorItem : SceneActors)
		{
			if(actorItem.Key == ActorId)
				return actorItem.Value != nullptr;
		}
#endif
		return false;
	}
	return SceneActors[ActorId] != nullptr;
}


bool UAssetIndexComponent::IsActorBlueprintDefined(const FString& AssetId) const
{
	if (!ActorBlueprints.Contains(AssetId))
	{
#if WITH_EDITOR
		// TODO: workaround for a bug (see above)
		// search the key in the map
		for (const auto& bpItem : ActorBlueprints)
		{
			if(bpItem.Key == AssetId)
				return bpItem.Value != nullptr;
		}
#endif
		return false;
	}
	return ActorBlueprints[AssetId] != nullptr;
}


UTexture2D* UAssetIndexComponent::GetTextureById(const FString& AssetId)
{
	if (!Textures.Contains(AssetId))
	{
		return nullptr;
	}
	return Textures[AssetId];
}


UMaterialInterface* UAssetIndexComponent::GetMaterialById(const FString& AssetId)
{
	if (!Materials.Contains(AssetId))
	{
		return nullptr;
	}
	return Materials[AssetId];
}

