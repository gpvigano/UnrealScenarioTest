// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)


#include "UEUtil.h"
#include "Engine/GameEngine.h"
#include "Misc/FileHelper.h"
#include "Misc/OutputDeviceNull.h"
#include "Misc/OutputDeviceDebug.h"
#include "HAL/PlatformFilemanager.h"
#include "UnrealClient.h" // FScreenshotRequest
#include "GameFramework/PlayerController.h"
#include "Components/MaterialBillboardComponent.h"

#include "Runtime/Launch/Resources/Version.h" // ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION
#if ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 25
#define FProperty UProperty
#define FStructProperty UStructProperty
#define FObjectProperty UObjectProperty
#endif



Logger UUEUtil::Log;

void UUEUtil::GetActorComponents(AActor* Actor, TArray<UActorComponent*>& ActorComponents, TSubclassOf<UActorComponent> ActorComponentClass)
{
	ActorComponents.Empty();
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 23
	Actor->GetComponents(ActorComponentClass, ActorComponents);
#else
	ActorComponents = Actor->GetComponentsByClass(ActorComponentClass);
#endif
}


void UUEUtil::GetPrimitiveComponents(AActor* Actor, TArray<UPrimitiveComponent*>& PrimComponents)
{
	TArray<UActorComponent*> actorComponents;
	GetActorComponents(Actor, actorComponents, UPrimitiveComponent::StaticClass());
	PrimComponents.Empty();
	for (int i = 0; i < actorComponents.Num(); i++)
	{
		UPrimitiveComponent* primComp = CastChecked<UPrimitiveComponent>(actorComponents[i]);
		PrimComponents.Add(primComp);
	}
}


void UUEUtil::GetMeshComponents(AActor* Actor, TArray<UMeshComponent*>& MeshComponents)
{
	TArray<UActorComponent*> actorComponents;
	GetActorComponents(Actor, actorComponents, UMeshComponent::StaticClass());

	MeshComponents.Empty();
	for (int i = 0; i < actorComponents.Num(); i++)
	{
		UMeshComponent* primComp = CastChecked<UMeshComponent>(actorComponents[i]);
		MeshComponents.Add(primComp);
	}
}


void UUEUtil::GetSceneComponents(AActor* Actor, TArray<USceneComponent*>& SceneComponents)
{
	TArray<UActorComponent*> actorComponents;
	GetActorComponents(Actor, actorComponents, USceneComponent::StaticClass());

	SceneComponents.Empty();
	for (int i = 0; i < actorComponents.Num(); i++)
	{
		USceneComponent* SceneComponent = CastChecked<USceneComponent>(actorComponents[i]);
		if (SceneComponent != nullptr)
		{
			SceneComponents.Add(SceneComponent);
		}
	}
}


USceneComponent* UUEUtil::FindSceneComponent(AActor* Actor, FString Name)
{
	if (!Actor)
	{
		return nullptr;
	}
	TArray<UActorComponent*> actorComponents;
	GetActorComponents(Actor, actorComponents, USceneComponent::StaticClass());

	for (int i = 0; i < actorComponents.Num(); i++)
	{
		USceneComponent* SceneComponent = CastChecked<USceneComponent>(actorComponents[i]);
		if (SceneComponent != nullptr && SceneComponent->GetName() == Name)
		{
			return SceneComponent;
		}
	}
	return nullptr;
}


AActor* UUEUtil::CreateActorOfClass(UWorld* World, TSubclassOf<AActor> ActorClass, FName NewName, AActor* ParentActor)
{
	AActor* newActor = nullptr;
	FActorSpawnParameters spawnParams;
	spawnParams.Name = NewName;
	newActor = World->SpawnActor<AActor>(ActorClass, spawnParams);
	if (ParentActor)
	{
		newActor->AttachToActor(ParentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}
	return newActor;
}


AActor* UUEUtil::CreateCloneOfActor(AActor* ExistingActor, FString NewName, AActor* ParentActor)
{
	// TODO: static mesh component is not duplicated after the first time (possible UE4 bug?)

	UWorld* World = ExistingActor->GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Template = ExistingActor;
	//SpawnParams.Owner = ParentActor;
	////if (NewName == ExistingActor->GetName())
	////{
	////	Log.LogMessage({ Logger::WARNING, "Trying to spawn two objects with the same name." });
	////}
	////else if (!NewName.IsEmpty())
	////{
	////	SpawnParams.Name = FName(*NewName);
	////}
	AActor* newActor = World->SpawnActor<AActor>(ExistingActor->GetClass(), SpawnParams);
	if (ParentActor)
	{
		newActor->AttachToActor(ParentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}
	newActor->SetActorLocation(ExistingActor->GetActorLocation());
	newActor->SetActorRotation(ExistingActor->GetActorRotation());
	newActor->SetActorScale3D(ExistingActor->GetActorScale3D());
#if WITH_EDITOR
	newActor->SetActorLabel(*NewName);
#endif
	return newActor;
}


AActor* UUEUtil::CreateCloneOfActorAndPlace(AActor* ExistingActor, FString NewName, FVector SpawnLocation, FRotator SpawnRotation, AActor* ParentActor, bool Relative)
{
	AActor* newActor = CreateCloneOfActor(ExistingActor, NewName, ParentActor);
	// Using the version with location/rotation after ClearScene() the position seems to be reset, so here is restored
	if (Relative)
	{
		newActor->SetActorRelativeLocation(SpawnLocation);
		newActor->SetActorRelativeRotation(SpawnRotation);
	}
	newActor->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
	//newActor->Rename( *NewName );
	return newActor;
}


bool UUEUtil::ExistingObject(ULevel* OuterLevel, const FString& Name)
{
	UObject* existingObject = StaticFindObject(NULL, OuterLevel, *Name, true);
	return existingObject != nullptr;
}


bool UUEUtil::GetOrCreatePath(const FString& DirectoryPath, const FString& FileName, FString& OutAbsoluteFilePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.CreateDirectoryTree(*DirectoryPath))
	{
		OutAbsoluteFilePath = DirectoryPath;
		if (!OutAbsoluteFilePath.IsEmpty() && !OutAbsoluteFilePath.EndsWith("/"))
		{
			OutAbsoluteFilePath.AppendChar('/');
		}
		// Get absolute file path
		OutAbsoluteFilePath.Append(FileName);
		return true;
	}
	return false;
}


bool UUEUtil::FileExists(const FString& AbsoluteFilePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	return PlatformFile.FileExists(*AbsoluteFilePath);
}


bool UUEUtil::SaveTextFile(const FString& SaveDirectory, const FString& FileName, const FString& TextToSave, bool AllowOverwriting)
{
	FString AbsoluteFilePath;

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (GetOrCreatePath(SaveDirectory, FileName, AbsoluteFilePath))
	{
		// Get absolute file path

		// Allow overwriting or file doesn't already exist
		if (AllowOverwriting || !FileExists(AbsoluteFilePath))
		{
			return FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath);
		}
	}
	return false;
}


bool UUEUtil::LoadTextFile(const FString& DirectoryPath, const FString& FileName, FString& TextToLoad)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.DirectoryExists(*DirectoryPath))
	{
		// Get absolute file path
		FString AbsoluteFilePath = DirectoryPath + "/" + FileName;

		// Allow overwriting or file doesn't already exist
		if (PlatformFile.FileExists(*AbsoluteFilePath))
		{
			return FFileHelper::LoadFileToString(TextToLoad, *AbsoluteFilePath);
		}
	}
	return false;
}


bool UUEUtil::SetActorActive(AActor* ExistingActor, bool Active, bool AffectChildren)
{
	return SetActorActive(ExistingActor, Active, true, true, true, AffectChildren);
}


bool UUEUtil::SetActorVectorProperty(AActor* ExistingActor, FName PropertyName, FVector VectorValue)
{
	FProperty *posProp = ExistingActor->GetClass()->FindPropertyByName(PropertyName);
	void* ValuePtr = posProp->ContainerPtrToValuePtr<void>(ExistingActor);

	// Ensure this property is a struct
#if ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 25
	UStructProperty* posStructProp = Cast<UStructProperty>(posProp);
#else
	FStructProperty* posStructProp = CastField<FStructProperty>(posProp);
#endif
	if (posStructProp)
	{
		// Ensure this struct is a Vector
		if (posStructProp->Struct->GetName() == "Vector")
		{
			// Set Vector to value
			posStructProp->CopyCompleteValue(ValuePtr, &VectorValue);
			return true;
		}
	}
	return false;
}


bool UUEUtil::SetActorSceneCompProperty(AActor* ExistingActor, FName PropertyName, USceneComponent* SceneComponent)
{
	FProperty *posProp = ExistingActor->GetClass()->FindPropertyByName(PropertyName);
	void* ValuePtr = posProp->ContainerPtrToValuePtr<void>(ExistingActor);

#if ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 25
	FObjectProperty* sceneCompProp = Cast<FObjectProperty>(posProp);
#else
	FObjectProperty* sceneCompProp = CastField<FObjectProperty>(posProp);
#endif
	if (sceneCompProp)
	{
		sceneCompProp->SetObjectPropertyValue(ValuePtr, SceneComponent);
		return true;
	}
	return false;
}


USceneComponent* UUEUtil::GetActorSceneCompProperty(AActor* ExistingActor, FName PropertyName)
{
	//FProperty *posProp = ExistingActor->GetClass()->FindPropertyByName(PropertyName);
	//void* ValuePtr = posProp->ContainerPtrToValuePtr<void>(ExistingActor);

#if ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 25
	UObjectProperty* sceneCompProp = FindField<UObjectProperty>(ExistingActor->GetClass(), PropertyName);
	//UObjectProperty* sceneCompProp = Cast<UObjectProperty>(posProp);
#else
	FObjectProperty* sceneCompProp = FindFProperty<FObjectProperty>(ExistingActor->GetClass(), PropertyName);
	//FObjectProperty* sceneCompProp = CastField<FObjectProperty>(posProp);
#endif
	if (sceneCompProp)
	{
		USceneComponent* sceneComponent = Cast<USceneComponent>(sceneCompProp->GetPropertyValue_InContainer(ExistingActor));
		//UObject* objProp = sceneCompProp->GetObjectPropertyValue(ValuePtr);
		//USceneComponent* sceneComponent = Cast<USceneComponent>(objProp);
		return sceneComponent;
	}
	return nullptr;
}


bool UUEUtil::CallActorFunctionWithArguments(AActor* ExistingActor, const FString Command)
{
	FOutputDeviceDebug debug;
	return ExistingActor->CallFunctionByNameWithArguments(*Command, debug, NULL, true);
}


bool UUEUtil::SetActorVisible(AActor* ExistingActor, bool Visible, bool AffectChildren)
{
	if (ExistingActor == nullptr)
	{
		return false;
	}
	TArray<UActorComponent*> actorComponents;
	GetActorComponents(ExistingActor, actorComponents, UPrimitiveComponent::StaticClass());
	for (int i = 0; i < actorComponents.Num(); i++)
	{
		UPrimitiveComponent* primComp = CastChecked<UPrimitiveComponent>(actorComponents[i],ECastCheckedType::NullAllowed);
		if (primComp)
		{
#if ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 25
			primComp->bVisible = Visible;
#else
			primComp->SetVisibility(Visible);
#endif
		}
	}
	if (AffectChildren)
	{
		TArray<AActor*> childActors;
		ExistingActor->GetAllChildActors(childActors);
		for (AActor* childActor : childActors)
		{
			SetActorVisible(childActor, Visible, false);
		}
	}

	return true;
}


bool UUEUtil::SetActorActive(AActor* ExistingActor, bool Active, bool AffectRendering, bool AffectCollision, bool AffectTick, bool AffectChildren)
{
	if (ExistingActor == nullptr)
	{
		return false;
	}

	// see https://docs.unrealengine.com/en-US/GettingStarted/FromUnity/index.html

	if (AffectRendering)
	{
		// Hides visible components
		ExistingActor->SetActorHiddenInGame(!Active);
	}

	if (AffectCollision)
	{
		// Disables collision components
		ExistingActor->SetActorEnableCollision(Active);
	}

	if (AffectTick)
	{
		// Stops the Actor from ticking
		ExistingActor->SetActorTickEnabled(Active);
	}
	if (AffectChildren)
	{
		TArray<AActor*> childActors;
		ExistingActor->GetAllChildActors(childActors);
		for (AActor* childActor : childActors)
		{
			SetActorActive(childActor, Active, AffectRendering, AffectCollision, AffectTick, false);
		}
	}

	return true;
}


bool UUEUtil::GetActorVisible(AActor* ExistingActor)
{
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 23
	return !ExistingActor->IsHidden();// && ...HiddenInGame?
#else
	return !ExistingActor->bHidden;
#endif
}


void UUEUtil::DestroyActorsArray(TArray<AActor*>& Actors, bool ClearOverlaps, bool DestroyChildren)
{
	for (AActor* actor : Actors)
	{
		if (::IsValid(actor))
		{
			if (ClearOverlaps)
			{
				actor->ClearComponentOverlaps();
			}
			if(DestroyChildren)
			{
				TArray<AActor*> attachedActors;
				actor->GetAttachedActors(attachedActors);
				DestroyActorsArray(attachedActors);
			}
			actor->Destroy();
		}
	}
	Actors.Reset();
}


AActor* UUEUtil::PickActorUnderCursor(APlayerController* PlayerController, float& OutDistance)
{
	FHitResult traceResult(ForceInit);
	if (!PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, traceResult))
	{
		return nullptr;
	}

#if ENGINE_MAJOR_VERSION > 4
	FVector actorPos = traceResult.GetActor()->GetActorLocation();
#else
	FVector actorPos = traceResult.Actor->GetActorLocation();
#endif
	OutDistance = FVector::Distance(actorPos, traceResult.TraceStart);
#if ENGINE_MAJOR_VERSION > 4
	return traceResult.GetActor();
#else
	return traceResult.Actor.Get();
#endif
}


float UUEUtil::GetDistanceFromEyesViewPoint(AActor* TargetActor)
{
	APlayerController* playerController = TargetActor->GetWorld()->GetFirstPlayerController();
	float dist = 0.0f;
	FVector viewPos;
	FRotator viewRot;
	playerController->GetActorEyesViewPoint(viewPos, viewRot);
	FVector pickedHandlePos = TargetActor->GetActorLocation();
	dist = FVector::Distance(pickedHandlePos, viewPos);
	return dist;
}


void UUEUtil::SetDistanceFromEyesViewPoint(USceneComponent* SceneComponent, float Distance)
{
	APlayerController* playerController = SceneComponent->GetWorld()->GetFirstPlayerController();
	FVector actorPos = SceneComponent->GetComponentLocation();
	FVector pos;
	FRotator rot;
	playerController->GetPlayerViewPoint(pos, rot);

	FVector dir = actorPos - pos;
	if (FVector::DotProduct(dir, rot.Vector()) < 0)
	{
		dir = -dir;
	}
	dir.Normalize();
	FVector newPos = pos + dir*Distance;
	SceneComponent->SetWorldLocation(newPos);
}


void UUEUtil::MouseDragActor(AActor* DraggedActor, float Distance)
{
	if (DraggedActor)
	{
		APlayerController* playerController = DraggedActor->GetWorld()->GetFirstPlayerController();
		FVector pos;
		if (MouseDragLocation(playerController, Distance, pos))
		{
			DraggedActor->SetActorLocation(pos);
		}
	}
}


bool UUEUtil::MouseDragLocation(APlayerController* PlayerController, float Distance, FVector& Location)
{
	if (PlayerController)
	{
		FVector2D pickedMousePos;
		FVector pickedWorldPos;
		FVector pickedWorldDir;
		if (PlayerController->GetMousePosition(pickedMousePos.X, pickedMousePos.Y))
		{
			if (PlayerController->DeprojectScreenPositionToWorld(pickedMousePos.X, pickedMousePos.Y, pickedWorldPos, pickedWorldDir))
			{
				Location = pickedWorldPos + pickedWorldDir * Distance;
				return true;
			}
		}
	}
	return false;
}


void UUEUtil::RotateAlongVerticalAxis(AActor* Actor, float Degrees)
{
	FVector euler = Actor->GetActorRotation().Euler();
	euler.Z += Degrees;
	FRotator newRot = FRotator::MakeFromEuler(euler);
	Actor->SetActorRotation(newRot);
}


void UUEUtil::RotateWithPlayer(
	APlayerController* PlayerController,
	USceneComponent* SceneComponent,
	bool AlignWithViewPoint)
{
	FVector playerPos;
	FRotator playerRot;
	PlayerController->GetPlayerViewPoint(playerPos, playerRot);
	if (AlignWithViewPoint)
	{
		FVector forward = SceneComponent->GetComponentLocation() - playerPos;
		FVector right = playerRot.Quaternion().GetRightVector();
		FMatrix mat = FRotationMatrix::MakeFromXY(forward, right);
		playerRot = mat.ToQuat().Rotator();
	}
	SceneComponent->SetWorldRotation(playerRot, false, nullptr, ETeleportType::TeleportPhysics);
}


bool UUEUtil::SaveScreenshot(const FString& DirectoryPath, const FString& FileName, bool AppendSuffix)
{
	FString filePath;
	if (!GetOrCreatePath(DirectoryPath, FileName, filePath))
	{
		return false;
	}
	//FString filePath("C:/path/to/folder/screenshot.png");
	FScreenshotRequest::RequestScreenshot(filePath, false, AppendSuffix);
	return false;
}

FVector UUEUtil::FindClosestVectorDirection(FVector ReferenceVector, TArray<FVector> AllowedDirections)
{
	if (AllowedDirections.Num() == 0)
	{
		Log.LogMessage(Logger::WARNING, "Empty direction vector in FindClosestVectorDirection()","Util");
		return FVector::ZeroVector;
	}
	AllowedDirections.Sort([&ReferenceVector](const FVector& vec1, const FVector& vec2)
	{
		return fabs(FQuat::FindBetween(ReferenceVector, vec1).Rotator().Yaw)
			< fabs(FQuat::FindBetween(ReferenceVector, vec2).Rotator().Yaw);
	});

	return AllowedDirections[0];
}


FVector UUEUtil::FindClosestAxisDirectionXY(FVector ReferenceVector)
{
	TArray<FVector> dirs;
	dirs.Add(FVector::ForwardVector);
	dirs.Add(-FVector::ForwardVector);
	dirs.Add(FVector::RightVector);
	dirs.Add(-FVector::RightVector);

	return FindClosestVectorDirection(ReferenceVector, dirs);
}


FVector UUEUtil::FindClosestAxisDirectionXYZ(FVector ReferenceVector)
{
	TArray<FVector> dirs;
	dirs.Add(FVector::ForwardVector);
	dirs.Add(-FVector::ForwardVector);
	dirs.Add(FVector::RightVector);
	dirs.Add(-FVector::RightVector);
	dirs.Add(FVector::UpVector);
	dirs.Add(-FVector::UpVector);

	return FindClosestVectorDirection(ReferenceVector, dirs);

	//dirs.Sort([&dir](const FVector& vec1, const FVector& vec2)
	//{
	//	return fabs(FQuat::FindBetween(dir, vec1).Rotator().Yaw)
	//		< fabs(FQuat::FindBetween(dir, vec2).Rotator().Yaw);
	//});

	//return dirs[0];
}


float UUEUtil::ManhattanDistanceXY(FVector Position1, FVector Position2)
{
	float dist = 0.0f;
	dist += fabs(Position2.X - Position1.X);
	dist += fabs(Position2.Y - Position1.Y);
	return dist;
}


float UUEUtil::ManhattanDistanceXYZ(FVector Position1, FVector Position2)
{
	float dist = 0.0f;
	dist += fabs(Position2.X - Position1.X);
	dist += fabs(Position2.Y - Position1.Y);
	dist += fabs(Position2.Z - Position1.Z);
	return dist;
}


void UUEUtil::ToggleActorOutline(AActor* Actor, bool Enabled, EEditFlags EditFlags, FName SkipTag)
{
	if (!Actor)
	{
		return;
	}
	TArray<UActorComponent*> actorComponents;
	GetActorComponents(Actor, actorComponents, UPrimitiveComponent::StaticClass());
	bool checkTag = !SkipTag.IsNone();
	for (int i = 0; i < actorComponents.Num(); i++)
	{
		UPrimitiveComponent* primComp = CastChecked<UPrimitiveComponent>(actorComponents[i]);
		if (!checkTag || !primComp->ComponentHasTag(SkipTag))
		{
			TogglePrimitiveOutline(primComp, Enabled, EditFlags);
		}
	}
	TArray<AActor*> attachedActors;
	Actor->GetAttachedActors(attachedActors);
	for (AActor* childActor : attachedActors)
	{
		ToggleActorOutline(childActor, Enabled, EditFlags, SkipTag);
	}
}


void UUEUtil::TogglePrimitiveOutline(UPrimitiveComponent* PrimitiveComponent, bool Enabled, EEditFlags EditFlags)
{
	int32 StencilValue = (int32)EditFlags;
	if (!PrimitiveComponent)
	{
		return;
	}
	if (Enabled)
	{
		int32 stencilVal = PrimitiveComponent->CustomDepthStencilValue | StencilValue;
		PrimitiveComponent->SetRenderCustomDepth(true);
		PrimitiveComponent->SetCustomDepthStencilValue(stencilVal);
	}
	else
	{
		int32 stencilVal = PrimitiveComponent->CustomDepthStencilValue&~StencilValue;
		PrimitiveComponent->SetRenderCustomDepth(stencilVal > 0);
		PrimitiveComponent->SetCustomDepthStencilValue(stencilVal);
	}
}


void UUEUtil::SaveActorMaterials(AActor* Actor, TMap<UPrimitiveComponent*, FMaterialList>& Materials)
{
	Materials.Reset();
	SaveActorMaterialsInternal(Actor, Materials);
}


void UUEUtil::SetActorMaterials(AActor* Actor, const TMap<UPrimitiveComponent*, FMaterialList>& Materials)
{
	TArray<UPrimitiveComponent*> primComponents;
	GetPrimitiveComponents(Actor, primComponents);
	int32 numPrimComponents = primComponents.Num();
	for (int i = 0; i < numPrimComponents; i++)
	{
		UPrimitiveComponent* primComp = primComponents[i];
		if (primComp && Materials.Contains(primComp))
		{
			SetPrimMaterials(primComp, Materials[primComp].SubMaterials);
		}
	}
	TArray < AActor* > children;
	Actor->GetAttachedActors(children);
	int32 numChildren = children.Num();
	for (int i = 0; i < numChildren; i++)
	{
		SetActorMaterials(children[i], Materials);
	}
}


void UUEUtil::SetActorMaterial(AActor* Actor, UMaterialInterface* NewMaterial, FName SkipTag)
{
	TArray<UPrimitiveComponent*> primComponents;
	GetPrimitiveComponents(Actor, primComponents);
	int32 numPrimComponents = primComponents.Num();
	bool checkTag = !SkipTag.IsNone();
	for (int i = 0; i < numPrimComponents; i++)
	{
		UPrimitiveComponent* primComp = primComponents[i];
		if (!checkTag || !primComp->ComponentHasTag(SkipTag))
		{
			SetPrimMaterial(primComp, NewMaterial);
		}
	}
	TArray < AActor* > children;
	Actor->GetAttachedActors(children);
	int32 numChildren = children.Num();
	for (int i = 0; i < numChildren; i++)
	{
		SetActorMaterial(children[i], NewMaterial);
	}
}


void UUEUtil::SaveActorMaterialsInternal(AActor* Actor, TMap<UPrimitiveComponent*, FMaterialList>& Materials)
{
	TArray<UPrimitiveComponent*> primComponents;
	GetPrimitiveComponents(Actor, primComponents);
	int32 numPrimComponents = primComponents.Num();
	for (int i = 0; i < numPrimComponents; i++)
	{
		UPrimitiveComponent* primComp = primComponents[i];
		TArray<UMaterialInterface*> meshMats;
		SavePrimMaterials(primComp, meshMats);
		Materials.Add(primComp, { meshMats });
	}
	TArray < AActor* > children;
	Actor->GetAttachedActors(children);
	int32 numChildren = children.Num();
	for (int i = 0; i < numChildren; i++)
	{
		SaveActorMaterialsInternal(children[i], Materials);
	}
}



void UUEUtil::SavePrimMaterials(UPrimitiveComponent* PrimitiveComponent, TArray<UMaterialInterface*>& Materials)
{
	int32 numMeshMats = PrimitiveComponent->GetNumMaterials();
	for (int i = 0; i < numMeshMats; i++)
	{
		Materials.Add(PrimitiveComponent->GetMaterial(i));
	}
}


void UUEUtil::SetPrimMaterials(UPrimitiveComponent* PrimitiveComponent, const TArray<UMaterialInterface*>& Materials)
{
	int32 numMeshMats = PrimitiveComponent->GetNumMaterials();
	if (Materials.Num() == numMeshMats)
	{
		for (int i = 0; i < numMeshMats; i++)
		{
			PrimitiveComponent->SetMaterial(i, Materials[i]);
		}
	}
}


void UUEUtil::SetPrimMaterial(UPrimitiveComponent* PrimitiveComponent, UMaterialInterface * Material)
{
	UMaterialBillboardComponent* billboard = Cast<UMaterialBillboardComponent>(PrimitiveComponent);
	// TODO: should UBillboardComponent be used instead?
	if (billboard)
	{
		int32 numElems = billboard->Elements.Num();
		for (int i = 0; i < numElems; i++)
		{
			billboard->SetMaterial(i, Material);
		}
	}
	else
	{
		int32 numMeshMats = PrimitiveComponent->GetNumMaterials();
		for (int i = 0; i < numMeshMats; i++)
		{
			PrimitiveComponent->SetMaterial(i, Material);
		}
	}
}

