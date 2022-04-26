// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UnrealScenarioActor.h"

#include "Misc/Paths.h" // FPaths
#include "Runtime/Launch/Resources/Version.h" // ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION
#include "Runtime/Engine/Public/UnrealClient.h" // FScreenshotRequest
#include "Runtime/Engine/Public/EngineUtils.h" // TActorIterator
#include "Runtime/Engine/Classes/Engine/PostProcessVolume.h"
#include "Engine/World.h" // GetWorld()
#include "GameFramework/PlayerController.h"
#if WITH_EDITOR
#include "Engine/Selection.h" // USelection
#endif

#include "UEUtil.h"
#include "UScenUtil.h"
#include "EntityStateComponent.h"
#include "EntityActor.h"
#include "ElementActor.h"
#include "EditableConnectionActor.h"

#include "discenfw/DigitalScenarioFramework.h"
#include "discenfw/scen/ScenarioManager.h"
#include "discenfw/scen/Catalog.h"
#include "discenfw/sim/SimulationManager.h"
#include "discenfw/ve/VirtualEnvironmentAPI.h"
#include "discenfw/ve/VeManager.h"
#include "discenfw/util/MessageLog.h"


AUnrealScenarioActor::AUnrealScenarioActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//DontHighlightTag = TEXT("DontHighlight");

	AssetIndexComponent = CreateDefaultSubobject<UAssetIndexComponent>(TEXT("AssetIndex"));
	AssetIndexComponent->CreationMethod = EComponentCreationMethod::Native;
	AddOwnedComponent(AssetIndexComponent);

	CatalogComponent = CreateDefaultSubobject<UCatalogComponent>(TEXT("Catalog"));
	CatalogComponent->CreationMethod = EComponentCreationMethod::Native;
	AddOwnedComponent(CatalogComponent);

	CatalogComponent->Name = TEXT("DefaultCatalog");
}


void AUnrealScenarioActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UUEUtil::Log.FlushMessages();

	using namespace discenfw;

#if WITH_EDITOR

	// Shortcut to add to the scenario an actor in the scene (Editor only)

	if (EntityActorToAdd)
	{
		UEntityDataComponent* origEntityComponent = UUScenUtil::GetEntityData(EntityActorToAdd);
		if (!origEntityComponent)
		{
			UUScenUtil::Log(Logger::ERROR, TEXT("The chosen element has not an Entity Component: ") + EntityActorToAdd->GetActorLabel());
		}
		else
		{
			FString assetId = EntityActorToAdd->GetActorLabel();
			AssetIndexComponent->SceneActors.Add(assetId, EntityActorToAdd);

			// TODO: add it also to the catalog

			AActor* newActor = CreateEntityFromActor(assetId, assetId, ScenarioActor);
			UUEUtil::SetActorActive(EntityActorToAdd, false);
			EntityActorToAdd = nullptr;

			SetActorSelectable(newActor);

			//EntityActors.Add(FName(*entityIdStr), newActor);
			EntityActors.Add(newActor);
			UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(newActor);
			if (entityComponent)
			{
				std::string entityClass = "Entity";
				if (Cast<UElementDataComponent>(entityComponent)) entityClass = "Element";
				if (Cast<UConnectionElementComponent>(entityComponent)) entityClass = "ConnectionElement";

				// const std::string& catalogId, const std::string& assetType, const std::string& uri, SourceType assetSource
				discenfw::AssetReference assetRef(
					UUEUtil::ToStdString(CatalogComponent->Name),
					"Actor",
					UUEUtil::ToStdString(assetId),
					discenfw::AssetReference::SourceType::SCENE
				);
				std::shared_ptr<Entity> newEntity = DiScenFw()->CreateEntity(
					entityClass,
					EntityIdentity(
						UUEUtil::ToStdString(entityComponent->Identifier),
						UUEUtil::ToStdString(entityComponent->Type),
						UUEUtil::ToStdString(entityComponent->Category),
						UUEUtil::ToStdString(entityComponent->Description)
					)
				);
				//newEntity->SetConfiguration(UUEUtil::ToStdString(entityComponent->Configuration));
				DiScenFw()->ConfigureEntity(newEntity->GetIdentifier(), UUEUtil::ToStdString(entityComponent->Configuration));
				newEntity->SetAssetReference(assetRef);
				//discenfw::DiScenFw()->CreateEntity(
				//	entityClass,
				//	{
				//	UUEUtil::ToStdString(entityComponent->Identifier),
				//	UUEUtil::ToStdString(entityComponent->Type),
				//	UUEUtil::ToStdString(entityComponent->Category),
				//	UUEUtil::ToStdString(entityComponent->Configuration),
				//	UUEUtil::ToStdString(entityComponent->Description),
				//	assetRef
				//	});
			}

		}
	}

#endif

	UpdateSimulation(DeltaTime);

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	bool ctrlPressed = playerController->IsInputKeyDown(EKeys::LeftControl);

	// cancel
	if (playerController->WasInputKeyJustPressed(EKeys::RightMouseButton))
	{
		// TODO: hide context menu
		OnCancel();
		return;
	}

	// undo
	if (ctrlPressed && playerController->WasInputKeyJustPressed(EKeys::Z))
	{
		Undo();
		return;
	}

	// confirm
	if (MovingEntityActor && playerController->WasInputKeyJustReleased(EKeys::LeftMouseButton))
	{
		bool entityMoved = MovingEntityStartPos != MovingEntityActor->GetActorLocation()
			|| MovingEntityStartRot != MovingEntityActor->GetActorRotation();
		if (entityMoved)
		{
			if (SocketPrimComp && OtherSocketPrimComp)
			{
				AActor* entityActor = GetEntityFromActor(SocketPrimComp->GetOwner());
				AActor* otherEntityActor = GetEntityFromActor(OtherSocketPrimComp->GetOwner());
				if (MovingEntityActor == otherEntityActor)
				{
					Swap(entityActor, otherEntityActor);
				}
				if (OtherSocketPrimComp->GetOwner() == entityActor)
				{
					Swap(SocketPrimComp, OtherSocketPrimComp);
				}
				// Save references before they can be reset by an overlapping callback
				UPrimitiveComponent* entityPrimitive = SocketPrimComp;
				UPrimitiveComponent* otherPrimitive = OtherSocketPrimComp;
				UElementDataComponent* entityComponent = UUScenUtil::GetElementData(entityActor);
				UElementDataComponent* otherEntityComponent = UUScenUtil::GetElementData(otherEntityActor);
				FSocketInfo* thisSocket = entityComponent->GetSocketInfoByComponent(entityPrimitive);
				FSocketInfo* otherSocket = otherEntityComponent->GetSocketInfoByComponent(otherPrimitive);
				bool compatible = UUScenUtil::AreSocketsCompatible(*thisSocket, *otherSocket);
				if (compatible)
				{
					MovingEntityActor->SetActorRotation(otherPrimitive->GetComponentRotation(), ETeleportType::TeleportPhysics);
					// physically connect elements
					FVector offset = otherPrimitive->GetComponentLocation() - entityPrimitive->GetComponentLocation();
					MovingEntityActor->SetActorLocation(MovingEntityActor->GetActorLocation() + offset, false, nullptr, ETeleportType::TeleportPhysics);
				}
				entityComponent->HighlightSocket(entityPrimitive, false, EEditFlags::Connecting | EEditFlags::Overlapping);
				otherEntityComponent->HighlightSocket(otherPrimitive, false, EEditFlags::Connecting | EEditFlags::Overlapping);
				//UUEUtil::TogglePrimitiveOutline(SocketPrimComp, false, EEditFlags::Connecting | EEditFlags::Overlapping);
				//UUEUtil::TogglePrimitiveOutline(OtherSocketPrimComp, false, EEditFlags::Connecting | EEditFlags::Overlapping);
			}
			SocketPrimComp = nullptr;
			OtherSocketPrimComp = nullptr;
			SaveEntityPosition();
			return;
		}
		else
		{
			// TODO: show context menu
		}
	}

	if (SelectedEntityActor && playerController->WasInputKeyJustPressed(EKeys::W) && ctrlPressed)
	{
		AActor* prevSelectedEntityActor = SelectEntityActor(nullptr);
		AActor* newElement = DuplicateElement(prevSelectedEntityActor);
		if (newElement)
		{
			newElement->SetActorLocation(prevSelectedEntityActor->GetActorLocation());
			newElement->SetActorRotation(prevSelectedEntityActor->GetActorRotation());
			SelectEntityActor(newElement);
			DragSelectedEntity();
		}
		return;
	}

	if (SelectedEntityActor && playerController->WasInputKeyJustPressed(EKeys::Delete))
	{
		AActor* prevSelectedEntityActor = SelectEntityActor(nullptr);
		DeleteElement(prevSelectedEntityActor);
		return;
	}

	if (HighlighedEntityActor == nullptr && playerController->WasInputKeyJustReleased(EKeys::LeftMouseButton))
	{
		if (!bTemporaryLockedSelection)
		{
			SelectEntityActor(nullptr);
			return;
		}
		bTemporaryLockedSelection = false;
	}

	if (IsEditing())
	{
		UpdateEditing(DeltaTime);
	}
}


#if WITH_EDITOR

void AUnrealScenarioActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!ScenarioActor)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Scenario Actor not defined"));
	}
}

#endif


TArray<FString> AUnrealScenarioActor::GetEntityIdentifiers()
{
	using namespace discenfw;
	const auto& entities = DiScenFw()->GetEntities();
	TArray<FString> ids;
	for (unsigned i = 0; i < entities.size(); i++)
	{
		const std::shared_ptr< const discenfw::Entity > entity = entities[i];
		ids.Add(UUEUtil::ToFString(entity->GetIdentifier()));
	}
	return ids;
}


bool AUnrealScenarioActor::IsScenarioEmpty()
{
	return EntityActors.Num() == 0;
}


int AUnrealScenarioActor::CountEntitiesByCategory(FString CategoryId)
{
	int count = 0;
	std::string category = UUEUtil::ToStdString(CategoryId);
	using namespace discenfw;
	const auto& entities = DiScenFw()->GetEntities();
	for (unsigned i = 0; i < entities.size(); i++)
	{
		if (entities[i]->GetIdentity().Category == category)
		{
			count++;
		}
	}
	return count;
}


AActor* AUnrealScenarioActor::AddElementFromCatalog(FString ItemId, bool bMoveToNextSlot)
{
	const FCatalogItem* catalogItem = CatalogComponent->Items.FindByPredicate(
		[&ItemId](const FCatalogItem& item)
	{
		return item.ItemId == ItemId;
	});
	if (!catalogItem)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Item not found in catalog: ") + ItemId);
		return nullptr;
	}
	bool inProject = (catalogItem->AssetSource == EAssetSource::Project);
	if (catalogItem->AssetSource == EAssetSource::External)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Asset loading from external source not (yet) implemented: ") + ItemId);
		return nullptr;
	}
	if (catalogItem->AssetSource == EAssetSource::Undefined)
	{
		inProject = AssetIndexComponent->IsActorBlueprintDefined(catalogItem->AssetId);
	}
	AActor* entityActor = nullptr;
	if (inProject)
	{
		entityActor = AssetIndexComponent->CreateActorFromAsset(catalogItem->AssetId, "", ScenarioActor);
	}
	else
	{
		entityActor = AssetIndexComponent->CreateActorFromSceneActor(catalogItem->AssetId, "", ScenarioActor);
	}
	if (!entityActor)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to create element from: ") + catalogItem->AssetId);
		return nullptr;
	}
	UEntityDataComponent* entityComp = UUScenUtil::GetEntityData(entityActor);
	// TODO: automatic creation of UEntityDataComponent from entity data

	// TODO: review this automatic creation of identifier (based on type or category? what if they are empty? DiScenFw already implements this in a different way, see DuplicateEntity.)
	FString entityType = entityComp->Type;
	FString entityIdentifier = entityComp->Identifier;
	if (!catalogItem->Category.IsEmpty())
	{
		entityType = catalogItem->Category;
		if (CatalogComponent->CategoryAbbreviations.Contains(catalogItem->Category))
		{
			entityType = CatalogComponent->CategoryAbbreviations[catalogItem->Category];
		}
		entityIdentifier = entityType;
	}
	int count = 1;
	using namespace discenfw;
	do
	{
		entityIdentifier = FString::Printf(TEXT("%s%d"), *entityType, count);
		count++;
	} while (EntityActorExists(entityIdentifier) || UUEUtil::ExistingObject(GetLevel(), entityIdentifier));
	// TODO: check missing synchronization between DSF e UnrealScenario

	entityComp->Identifier = entityIdentifier;

	entityActor->Rename(*entityIdentifier);
#if WITH_EDITOR
	entityActor->SetActorLabel(*entityIdentifier);
#endif

	AssetDefinition::SourceType sourceType = inProject ? AssetReference::PROJECT : AssetReference::SCENE;

	EntityIdentity entityIdentity(
		UUEUtil::ToStdString(entityIdentifier),
		UUEUtil::ToStdString(entityComp->Type),
		UUEUtil::ToStdString(entityComp->Category),
		""
	);

	std::shared_ptr<Entity> newEntity = DiScenFw()->CreateEntity(
		"Element",
		entityIdentity
		);
	//newEntity->SetConfiguration(UUEUtil::ToStdString(entityComp->Configuration));
	newEntity->SetAssetReference(
		AssetReference(
			UUEUtil::ToStdString(CatalogComponent->Name),
			UUEUtil::ToStdString(catalogItem->AssetType),
			UUEUtil::ToStdString(catalogItem->AssetId),
			"",
			sourceType
		)
	);
	DiScenFw()->ConfigureEntity(newEntity->GetIdentifier(), UUEUtil::ToStdString(entityComp->Configuration));

	UpdateEntityData(entityActor);
	SetActorSelectable(entityActor);
	EntityActors.Add(entityActor);
	if (bMoveToNextSlot)
	{
		MoveActorToNextSlotLocation(entityActor);
	}

	OnEntityAdded();

	return entityActor;
}


AActor* AUnrealScenarioActor::DuplicateElement(AActor* ElementActor)
{
	if (!ElementActor)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Cannot duplicate null element."));
		return nullptr;
	}
	UEntityDataComponent* entityComp = UUScenUtil::GetEntityData(ElementActor);
	if (!entityComp)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Cannot duplicate an actor without entity data."));
		return nullptr;
	}
	FString entityId = entityComp->Identifier;
	if (!EntityActors.Contains(ElementActor))
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Element not in scenario: ") + entityId);
		return nullptr;
	}
	if (IsEditing())
	{
		SaveEntityPosition();
	}

	//OnCancel();
	using namespace discenfw;
	const std::shared_ptr<Entity> newEntity = DiScenFw()->DuplicateEntity(UUEUtil::ToStdString(entityComp->Identifier));
	if (!newEntity)
	{
		return nullptr;
	}
	AActor* newEntityActor = ImportEntity(newEntity, ScenarioActor);
	if (!newEntityActor)
	{
		return nullptr;
	}
	EntityActors.Add(newEntityActor);

	OnEntityAdded();
	SelectEntityActor(newEntityActor);
	DragSelectedEntity();

	UUScenUtil::Log(Logger::DEBUG, TEXT("Duplicated element ") + entityId);

	return newEntityActor;
}


bool AUnrealScenarioActor::DeleteElement(AActor* ElementActor)
{
	if (!ElementActor)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Cannot delete null element."));
		return false;
	}
	UEntityDataComponent* entityComp = UUScenUtil::GetEntityData(ElementActor);
	if (!entityComp)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Cannot delete an actor without entity data."));
		return false;
	}
	FString entityId = entityComp->Identifier;
	if (!EntityActors.Contains(ElementActor))
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Element not in scenario: ") + entityId);
		return false;
	}
	OnCancel();
	using namespace discenfw;
	DiScenFw()->DeleteEntity(UUEUtil::ToStdString(entityComp->Identifier));
	EntityActors.Remove(ElementActor);
	ElementActor->ClearComponentOverlaps();
	ElementActor->Destroy();

	OnEntityRemoved();

	UUScenUtil::Log(Logger::DEBUG, TEXT("Deleted element ") + entityId);

	return true;
}

void AUnrealScenarioActor::MoveActorToNextSlotLocation(AActor* ElementActor)
{
	ElementActor->SetActorLocation(ElementSpawnSlot);
}


void AUnrealScenarioActor::PopulateScene()
{
	if (EntityActors.Num() > 0)
	{
		ClearScene();
	}

	using namespace discenfw;
	const auto& entities = DiScenFw()->GetEntities();
	for (unsigned i = 0; i < entities.size(); i++)
	{
		ImportEntity(entities[i], ScenarioActor);
	}
	BuildConnectionElements();
}


void AUnrealScenarioActor::ClearScene()
{
	//for (const TPair< FName, AEntityActor* >& pair : EntityActors)
	//{
	//	AConnectionElementActor* connectionElement = Cast<AConnectionElementActor>(pair.Value);
	//	if (connectionElement)
	//	{
	//		connectionElement->ClearAll();
	//	}
	//	pair.Value->Destroy();
	//}
	for (AActor* entityActor : EntityActors)
	{
		entityActor->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
		//entityActor->DetachRootComponentFromParent(false);
		entityActor->Destroy();
	}
	EntityActors.Reset();
	//ConnectionElementActors.Reset();
}


void AUnrealScenarioActor::UpdateData()
{
	//for (const TPair<FName, AEntityActor* >& pair : EntityActors)
	//{
	//	AEntityActor* entityActor = pair.Value;
	//	UpdateEntityData(entityActor);
	//}
	for (AActor* entityActor : EntityActors)
	{
		UpdateEntityData(entityActor);
	}
}


void AUnrealScenarioActor::LoadScenarioData()
{
	// TODO: InitDigitalScenarioFramework() calls are needed only for "CallInEditor" methods, before BeginPlay()
	InitDigitalScenarioFramework();

	using namespace discenfw;

	if (DiScenFw()->LoadScenario(UUEUtil::ToStdString(ScenarioDir() + ScenarioFileName), true))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("Scenario loaded from") + ScenarioFileName, false, true);
		if (!HistoryFileName.IsEmpty())
		{
			if (DiScenFw()->LoadScenarioHistory(UUEUtil::ToStdString(ScenarioDir() + HistoryFileName)))
			{
				UUScenUtil::Log(Logger::LOG, TEXT("Scenario history loaded from ") + HistoryFileName, false, true);
			}
			else
			{
				UUScenUtil::Log(Logger::ERROR, TEXT("Failed to load scenario history from ") + ScenarioDir() + HistoryFileName, false, true);
			}
		}
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, "Failed to load scenario from " + ScenarioDir() + ScenarioFileName, false, true);
	}
	OnScenarioLoaded();
}


void AUnrealScenarioActor::SaveScenarioData()
{
	InitDigitalScenarioFramework();

	if (!ScenarioFileName.IsEmpty())
	{
		using namespace discenfw;
		if (DiScenFw()->SaveScenario(UUEUtil::ToStdString(ScenarioDir() + ScenarioFileName), true))
		{
			UUScenUtil::Log(Logger::LOG, TEXT("Scenario saved to") + ScenarioFileName);
			if (!HistoryFileName.IsEmpty())
			{
				if (DiScenFw()->SaveScenarioHistory(UUEUtil::ToStdString(ScenarioDir() + HistoryFileName)))
				{
					UUScenUtil::Log(Logger::LOG, TEXT("Scenario history saved to") + HistoryFileName);
				}
				else
				{
					UUScenUtil::Log(Logger::ERROR, TEXT("Failed to save scenario history to ") + ScenarioDir() + HistoryFileName);
				}
			}
		}
		else
		{
			UUScenUtil::Log(Logger::ERROR, TEXT("Failed to save scenario to ") + ScenarioDir() + ScenarioFileName);
		}
	}
}


void AUnrealScenarioActor::ClearScenario()
{
	InitDigitalScenarioFramework();

	using namespace discenfw;
	DiScenFw()->ResetScenario();
	ClearScene();
	OnScenarioLoaded();
}


void AUnrealScenarioActor::Screenshot()
{
	if (UUEUtil::SaveScreenshot(ScenarioDir(), ScenarioFileName + TEXT(".png")))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("Screenshot save in ") + ScenarioDir());
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to save screenshot in ") + ScenarioDir());
	}
}


void AUnrealScenarioActor::UpdatePositioning()
{
	if (MovingEntityActor)
	{
		FVector dragPos;
		FVector prevActorLoc = MovingEntityActor->GetActorLocation();
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		if (UUEUtil::MouseDragLocation(playerController, MovingEntityDistance, dragPos))
		{
			float snapToGrid = 1.0f;
			dragPos = dragPos.GridSnap(snapToGrid);
			if (playerController->IsInputKeyDown(EKeys::LeftShift) || playerController->IsInputKeyDown(EKeys::RightShift))
			{
				dragPos.Z = prevActorLoc.Z - MovingEntityOffset.Z;
			}
			MovingEntityActor->SetActorLocation(dragPos + MovingEntityOffset, bOverlapOnDragging);
		}
	}
}


void AUnrealScenarioActor::CancelPositioning()
{
	MovingEntityActor->SetActorLocation(MovingEntityStartPos);
	MovingEntityActor->SetActorRotation(MovingEntityStartRot);
	if (OrigMaterialMap.Num() > 0)
	{
		UUEUtil::SetActorMaterials(MovingEntityActor, OrigMaterialMap);
		OrigMaterialMap.Empty();
	}

	MovingEntityActor = nullptr;
	//UUEUtil::ToggleActorOutline(SelectedEntityActor, false);
	//SelectedEntityActor = nullptr;
}


void AUnrealScenarioActor::Undo()
{
	// TODO: implement command chronology (see TQueue and GIOVE\SRC\include\giove\sys\gvruntime.h)
	UUScenUtil::Log(Logger::WARNING, TEXT("Undo is not yet implemented, sorry..."), false, true, "NOT_IMPLEMENTED");
}


FString AUnrealScenarioActor::ScenarioDir()
{
	//#if WITH_EDITOR

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 20
	FString projectPath = FPaths::ProjectDir() + "Data/";
#else
	FString projectPath = FPaths::GameDir() + "Data/";
#endif

	//#else
	//	FString projectPath = "../../";
	//#endif
	return projectPath;
}


void AUnrealScenarioActor::PlaySimulation()
{
	// TODO: this is needed only for "CallInEditor" methods
	if (!HasActorBegunPlay())
	{
		return;
	}

	using namespace discenfw;
	auto simulationController = DiScenFw()->ScenarioSimulationController();
	if (simulationController->ValidSimulation())
	{
		simulationController->PlaySimulation();
		SimulationProgress = (float)simulationController->ComputeSimulationProgress()*100.0;
	}
}


void AUnrealScenarioActor::PauseSimulation()
{
	if (!HasActorBegunPlay())
	{
		return;
	}

	using namespace discenfw;
	auto simulationController = DiScenFw()->ScenarioSimulationController();
	if (simulationController->ValidSimulation())
	{
		simulationController->PauseSimulation();
	}
}


void AUnrealScenarioActor::StopSimulation()
{
	if (!HasActorBegunPlay())
	{
		return;
	}

	using namespace discenfw;
	auto simulationController = DiScenFw()->ScenarioSimulationController();
	if (simulationController->ValidSimulation())
	{
		simulationController->StopSimulation();
		SimulationProgress = (float)simulationController->ComputeSimulationProgress()*100.0;
	}
}



void AUnrealScenarioActor::BeginPlay()
{
	Super::BeginPlay();

	//#if WITH_EDITOR
	UUEUtil::Log.bScreenDisabled = !bLogOnScreenEnabled;
	//#endif
		//UUEUtil::Log.CurrentLogLevel = CurrentLogLevel;

	bUseCustomDepth = false;
	for (TActorIterator<APostProcessVolume> actorItr(GetWorld()); actorItr; ++actorItr)
	{
		if (actorItr)
		{
			bUseCustomDepth = true;
			break;
		}
	}
	if (::IsValid(ScenarioActor) && ScenarioActor->IsRootComponentMovable())
	{
		ScenarioActor->SetActorLocation(FVector::ZeroVector);
	}

	InitDigitalScenarioFramework();

	// test scenario
	if (bAutoLoadScenario)
	{
		LoadScenarioData();
	}

	// test scenario simulation
	if (bAutoStartSimulation)
	{
		PlaySimulation();
	}
}


void AUnrealScenarioActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelEditing();
	Super::EndPlay(EndPlayReason);
	using namespace discenfw;
	DiScenFw()->ResetAll();
}


void AUnrealScenarioActor::InitDigitalScenarioFramework()
{
	if (bUnrealScenarioInitialized)
	{
		return;
	}
	using namespace discenfw;

	discenfw::SetLogDisplayFunction([](int severity,
		const std::string& message, const std::string& category,
		bool onConsole, bool onScreen, const std::string& msgTag)
	{
		UUEUtil::Log.PushMessage(
			Logger::MessageInfo(
				static_cast<Logger::ELogLevel>(severity),
				UUEUtil::ToFString(message),
				UUEUtil::ToFString(category),
				onConsole,
				onScreen,
				UUEUtil::ToFString(msgTag)
				));
	});

	std::shared_ptr<ve::VirtualEnvironmentAPI> ve = ve::VE();

	ve->ProjectDir = [&]() -> std::string
	{
		// TODO: allow customizable root path (currently = project path)
		return UUEUtil::ToStdString(ScenarioDir());
	};

	UUScenUtil::Log(Logger::LOG, TEXT("Project path: ") + ScenarioDir(), true, false);
	//UUScenUtil::Log(Logger::LOG, TEXT("ProjectUserDir() : ") + FPaths::GameUserDir(), true, false);
	//UUScenUtil::Log(Logger::LOG, TEXT("GetProjectFilePath() : ") + FPaths::GetProjectFilePath(), true, false);

	ve->TakeScreenshot = [](const std::string& filePath) -> void
	{
		FScreenshotRequest::RequestScreenshot(FString(filePath.c_str()), false, false);
	};

	//ve->LoadScenario = [&](const std::string& path) -> void { ScenarioFileName = path.c_str(); LoadScenarioData(); };
	//ve->SaveScenario = [&](const std::string& path) -> void { ScenarioFileName = path.c_str(); SaveScenarioData(); };
	ve->SyncScene = [&]() -> void { PopulateScene(); };
	ve->SyncScenario = [&]() -> void { UpdateData(); };

	ve->SyncSceneObjectTransform = [&](const std::string& elemId) -> void
	{
		AActor* elemActor = GetElementFromId(UUEUtil::ToFString(elemId));
		std::shared_ptr< Element > elem = GetElementByName(UUEUtil::ToFString(elemId));
		if (elemActor && elem)
		{
			SetElementActorTransform(elemActor, elem->GetTransform());
		}
	};

	ve->SyncElementTransform = [&](const std::string& elemId) -> void
	{
		AActor* elemActor = GetElementFromId(UUEUtil::ToFString(elemId));
		std::shared_ptr< Element > elem = GetElementByName(UUEUtil::ToFString(elemId));
		if (elemActor && elem)
		{
			UpdateElementTransform(elemActor, elem);
		}
	};


	//ve->EntityImported = [&](const std::string& entityId) -> bool {
	//	FName entityIdStr(entityId.c_str());
	//	return EntityActors.Contains(entityIdStr);
	//};
	//ve->SetEntityActive = [&](const std::string& entityId, bool on) -> void {
	//	FName entityIdStr(entityId.c_str());
	//	AActor* entityActor = EntityActors[entityIdStr];
	//	UUEUtil::SetActorActive(entityActor, on, true, true, true);
	//};


	ve->MoveViewPointTo = [&](Vector3D position) -> void
	{
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		APawn* pawn = playerController->GetPawn();
		FVector actorPos = { position.Forward,position.Right,position.Up };
		pawn->TeleportTo(actorPos, FRotator::ZeroRotator, false, true);
	};

	ve->SetViewPointDir = [&](Vector3D direction) -> void
	{
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		FVector actorDir = { direction.Forward,direction.Right,direction.Up };
		FMatrix actorMat = FRotationMatrix::MakeFromX(actorDir);
		FRotator actorRot;
		actorRot = actorMat.Rotator();
		playerController->SetControlRotation(actorRot);
	};

	ve->SetViewPointCoordSys = [&](CoordSys3D coordSys) -> void
	{
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		APawn* pawn = playerController->GetPawn();
		FVector actorPos = { coordSys.Origin.Forward,coordSys.Origin.Right,coordSys.Origin.Up };
		pawn->TeleportTo(actorPos, FRotator::ZeroRotator, false, true);
		FVector xDir = { coordSys.ForwardAxis.Forward,coordSys.ForwardAxis.Right,coordSys.ForwardAxis.Up };
		FVector yDir = { coordSys.RightAxis.Forward,coordSys.RightAxis.Right,coordSys.RightAxis.Up };
		FMatrix actorMat = FRotationMatrix::MakeFromXY(xDir, yDir);
		FRotator actorRot;
		actorRot = actorMat.Rotator();
		playerController->SetControlRotation(actorRot);
	};

	ve->LookAt = [&](Vector3D position) -> void
	{
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		APawn* pawn = playerController->GetPawn();
		FVector targetPos = { position.Forward,position.Right,position.Up };
		FVector actorDir = targetPos - pawn->GetActorLocation();
		FMatrix actorMat = FRotationMatrix::MakeFromX(actorDir);
		FRotator actorRot;
		actorRot = actorMat.Rotator();
		playerController->SetControlRotation(actorRot);
	};

	ve->LerpElementTransform = [&](const std::string& elemId, LocalTransform transform1, LocalTransform transform2, float trim) -> void
	{
		AActor* elemActor = GetElementFromId(UUEUtil::ToFString(elemId));
		if (elemActor)
		{
			UUScenUtil::Log(Logger::DEBUG, TEXT("Lerp trim: ") + FString::SanitizeFloat(trim), false, true, "LerpTrim");
			LerpActorTransform(elemActor, transform1, transform2, trim);
		}
	};

	//ve->ReparentElement = [&](const std::string& elemId, const std::string& parentElemId) {
	//	AElementActor* elemActor = GetElementFromId(elemId);
	//	AElementActor* parentElemActor = GetElementFromId(parentElemId);
	//	if (elemActor && parentElemActor)
	//	{
	//		elemActor->AttachToActor(parentElemActor, FAttachmentTransformRules::KeepWorldTransform);
	//	}
	//};
	bUnrealScenarioInitialized = true;
}


AActor* AUnrealScenarioActor::CreateEntityFromActor(FString assetId, FString entityName, AActor * parentActor)
{
	AActor* entityTypeActor = AssetIndexComponent->GetSceneActorById(assetId);
	if (entityTypeActor == nullptr)
	{
		UUScenUtil::Log(Logger::WARNING, TEXT("Template actor not found: ") + assetId);
		return nullptr;
	}
	if (UUScenUtil::GetEntityData(entityTypeActor) == nullptr)
	{
		UUScenUtil::Log(Logger::WARNING, TEXT("Template actor ") + assetId + TEXT(" has no entity data."));
		return nullptr;
	}

	AActor* newEntityActor = AssetIndexComponent->CreateActorFromSceneActor(assetId, entityName, parentActor);
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(newEntityActor);
	entityComponent->Identifier = entityName;

	return newEntityActor;
}


AActor* AUnrealScenarioActor::CreateEntityFromAsset(FString assetId, FString entityName, AActor * parentActor)
{
	TSubclassOf<AActor> entityClass;
	if (!AssetIndexComponent->IsActorBlueprintDefined(assetId))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("No asset found for ") + assetId);
		return nullptr;
	}
	AActor* newEntityActor = AssetIndexComponent->CreateActorFromAsset(assetId);
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(newEntityActor);
	if (!entityComponent)
	{
		UUScenUtil::Log(Logger::WARNING, TEXT("Actor blueprint ") + assetId + TEXT(" has no entity data."));
		newEntityActor->Destroy();
		return nullptr;
	}
	entityComponent->Identifier = entityName;
	if (parentActor)
	{
		newEntityActor->AttachToActor(parentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}
#if WITH_EDITOR
	if (!entityName.IsEmpty())
	{
		newEntityActor->SetActorLabel(*entityName);
	}
#endif
	return newEntityActor;
}


void AUnrealScenarioActor::HighlightActor(AActor* Actor, bool Highlighted)
{
	if (bUseCustomDepth)
	{
		UUEUtil::ToggleActorOutline(Actor, Highlighted, EEditFlags::Selected);
	}
	else
	{
		if (Highlighted)
		{
			UUEUtil::SaveActorMaterials(Actor, OrigMaterialMap);
			UUEUtil::SetActorMaterial(Actor, HighlightMaterial, DontHighlightTag);
		}
		else if (OrigMaterialMap.Num() > 0)
		{
			UUEUtil::SetActorMaterials(Actor, OrigMaterialMap);
			OrigMaterialMap.Empty();
		}
	}
}


void AUnrealScenarioActor::HighlightComponent(UPrimitiveComponent* PrimitiveComponent, bool Highlighted)
{
	if (bUseCustomDepth)
	{
		UUEUtil::TogglePrimitiveOutline(PrimitiveComponent, Highlighted, EEditFlags::Selected);
	}
	else
	{
		if (Highlighted)
		{
			UUEUtil::SavePrimMaterials(PrimitiveComponent, OrigPrimMaterials);
			UUEUtil::SetPrimMaterial(PrimitiveComponent, HighlightMaterial);
		}
		else if (OrigPrimMaterials.Num() > 0)
		{
			UUEUtil::SetPrimMaterials(PrimitiveComponent, OrigPrimMaterials);
			OrigPrimMaterials.Empty();
		}
	}
}







AActor* AUnrealScenarioActor::GetEntityFromActor(AActor* selectedActor)
{
	AActor* selectedEntityActor = selectedActor;
	AActor* entityActor = nullptr;
	UEntityDataComponent* entityComponent = nullptr;
	if (selectedEntityActor)
	{
		entityActor = selectedActor;
		entityComponent = UUScenUtil::GetEntityData(selectedEntityActor);
		while (entityComponent == nullptr && selectedEntityActor->GetAttachParentActor())
		{
			selectedEntityActor = selectedEntityActor->GetAttachParentActor();
			entityComponent = UUScenUtil::GetEntityData(selectedEntityActor);
			entityActor = selectedEntityActor;
		}
	}
	return entityActor;
}


AActor* AUnrealScenarioActor::SelectEntityFromActor(AActor* selectedActor)
{
	//AConnectionElementActor* connectionElementActor = nullptr;
	AActor* entityActor = GetEntityFromActor(selectedActor);
	if (entityActor)
	{
		//connectionElementActor = Cast<AConnectionElementActor>(entityActor);
		//if (connectionElementActor)
		//{
		//	int32 index = -1;
		//	if (ConnectionElementActors.Find(connectionElementActor, index))
		//	{
		//		connectionElementActor->ToggleActiveHandle(true);
		//		if (!connectionElementActor->SelectHandle(selectedActor))
		//		{
		//			connectionElementActor->SelectAnchor(0);
		//		}
		//	}
		//}
		if (EntityActors.Find(entityActor) != INDEX_NONE)
		{
			UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(entityActor);
			UUScenUtil::Log(
				Logger::LOG,
				"Selected: " + entityComponent->Identifier,
				true,
				true,
				"ENTITY_SELECTION");
			// TODO: store somewhere the selected entity actor
			//SelectEntity(pair.Key);
		}
	}
	//else
	//{
	//	SelectEntity(FName());
	//	ConnectionElementActors.Remove(nullptr);
	//}


	// TODO: always active handles?
	//for (int i = 0; i < ConnectionElementActors.Num(); i++)
	//{
	//	ConnectionElementActors[i]->ToggleActiveHandle(ConnectionElementActors[i] == connectionElementActor);
	//}

	return entityActor;
}


AActor* AUnrealScenarioActor::SelectEntityActor(AActor* selectedActor)
{
	AActor* prevSelectedEntityActor = SelectedEntityActor;
	if (prevSelectedEntityActor)
	{
		if (bUseCustomDepth)
		{
			UUEUtil::ToggleActorOutline(prevSelectedEntityActor, false, EEditFlags::Selected | EEditFlags::Connecting);
		}
		else if (SelectedOrigMaterialMap.Num() > 0)
		{
			UUEUtil::SetActorMaterials(prevSelectedEntityActor, SelectedOrigMaterialMap);
			SelectedOrigMaterialMap.Empty();
		}

		AEditableConnectionActor* connActor = Cast<AEditableConnectionActor>(prevSelectedEntityActor);
		if (connActor)
		{
			connActor->SetHandlesEnabled(false);
		}
	}

	SelectedEntityActor = selectedActor;
	if (SelectedEntityActor)
	{
		if (bUseCustomDepth)
		{
			UUEUtil::ToggleActorOutline(SelectedEntityActor, true, EEditFlags::Selected);
		}
		else
		{
			if (OrigMaterialMap.Num() > 0)
			{
				SelectedOrigMaterialMap = OrigMaterialMap;
				OrigMaterialMap.Empty();
			}
			else
			{
				if (HighlighedPrimComp && (HighlighedPrimComp->GetOwner()) == SelectedEntityActor)
				{
					// before saving original materials,
					// they must be restored if a primitive was already highlighted
					UUEUtil::SetPrimMaterials(HighlighedPrimComp, OrigPrimMaterials);
					OrigPrimMaterials.Empty();
				}
				UUEUtil::SaveActorMaterials(SelectedEntityActor, SelectedOrigMaterialMap);
				UUEUtil::SetActorMaterial(SelectedEntityActor, HighlightMaterial, DontHighlightTag);
			}
		}
		AEditableConnectionActor* connActor = Cast<AEditableConnectionActor>(SelectedEntityActor);
		if (connActor)
		{
			connActor->SetHandlesEnabled(true);
		}
	}
	return prevSelectedEntityActor;
}


AActor* AUnrealScenarioActor::GetEntityActorFromId(const FString& entityId)
{
	AActor** entityActorPtr = EntityActors.FindByPredicate(
		[&entityId](AActor* entityActor)
	{
		UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(entityActor);
		if (entityComponent)
		{
			return entityComponent->Identifier == entityId;
		}
		return false;
	});
	if (entityActorPtr == nullptr)
	{
		return nullptr;
	}
	return *entityActorPtr;
}


AActor* AUnrealScenarioActor::GetEntityFromId(const FString& entityId)
{
	AActor* entityActor = GetEntityActorFromId(entityId);
	if (entityActor == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Entity not found: ") + entityId);
		return nullptr;
	}
	return entityActor;
}


bool AUnrealScenarioActor::EntityActorExists(const FString& entityId)
{
	AActor* entityActor = GetEntityActorFromId(entityId);
	return entityActor != nullptr;
}


AActor* AUnrealScenarioActor::GetElementFromId(const FString& entityId)
{
	AActor* elemActor = GetEntityFromId(entityId);
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(elemActor);
	FString entityIdTag = entityId + "Missing";
	if (!entityComponent)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Entity not found: ") + entityId, true, true, entityIdTag);
		return nullptr;
	}
	UElementDataComponent* elementComponent = UUScenUtil::GetElementData(elemActor);
	if (!elementComponent)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Entity is not an element: ") + entityId, true, true, entityIdTag);
		return nullptr;
	}
	return elemActor;
}



void AUnrealScenarioActor::SetActorSelectable(AActor* actor, bool includeAttachedActors)
{
	actor->SetActorEnableCollision(true);
	actor->Tags.AddUnique("Selectable");
	//if (!actor->OnActorBeginOverlap.IsBound())
	//{
	//	actor->OnActorBeginOverlap.AddDynamic(this, &AUnrealScenarioActor::OnActorOverlapBegin);
	//	actor->OnActorHit.AddDynamic(this, &AUnrealScenarioActor::OnActorHit);
	//}

	// TODO: Find a way to activate an entity from blueprint
	//actor->OnClicked.AddDynamic(this, &AUnrealScenarioActor::OnActorClicked);

	// tag elements as movable
	if (Cast<AElementActor>(actor) != nullptr)
	{
		actor->Tags.AddUnique("Movable");
	}
	//if (bMouseInteractionEnabled)
	{
		TArray<UPrimitiveComponent*> primComponents;
		UUEUtil::GetPrimitiveComponents(actor, primComponents);
		for (int i = 0; i < primComponents.Num(); i++)
		{
			UPrimitiveComponent* primComp = primComponents[i];
			if (primComp)
			{
				if (bMouseInteractionEnabled)
				{
					if (!primComp->OnBeginCursorOver.IsBound()
						&& !primComp->ComponentHasTag(DontHighlightTag))
					{
						primComp->OnBeginCursorOver.AddDynamic(this, &AUnrealScenarioActor::OnBeginCursorOverComponent);
						primComp->OnEndCursorOver.AddDynamic(this, &AUnrealScenarioActor::OnEndCursorOverComponent);
						primComp->OnClicked.AddDynamic(this, &AUnrealScenarioActor::OnClickedComponent);
					}
				}
				//else
				if (bOverlappingEnabled)
				{
					if (!primComp->OnComponentHit.IsBound())
					{
						primComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
						primComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						primComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
						primComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
						primComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
						primComp->OnComponentHit.AddDynamic(this, &AUnrealScenarioActor::OnHit);
						primComp->OnComponentBeginOverlap.AddDynamic(this, &AUnrealScenarioActor::OnOverlapBegin);
						primComp->OnComponentEndOverlap.AddDynamic(this, &AUnrealScenarioActor::OnOverlapEnd);
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 16
						primComp->SetGenerateOverlapEvents(true);
#else
						primComp->bGenerateOverlapEvents = true;
#endif
					}
				}
			}
		}
	}

	if (includeAttachedActors)
	{
		TArray<AActor*> attachedActors;
		actor->GetAttachedActors(attachedActors);
		for (AActor* attachedActor : attachedActors)
		{
			SetActorSelectable(attachedActor);
		}
	}
}


void AUnrealScenarioActor::UpdateEntityData(AActor* entityActor)
{
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(entityActor);
	if (entityComponent)
	{
		using namespace discenfw;
		std::shared_ptr< discenfw::Entity > entity = DiScenFw()->UpdateEntityData(
			EntityIdentity(
				UUEUtil::ToStdString(entityComponent->Identifier),
				UUEUtil::ToStdString(entityComponent->Type),
				UUEUtil::ToStdString(entityComponent->Category),
				UUEUtil::ToStdString(entityComponent->Description)
			),
			true);
		entity->SetConfiguration(UUEUtil::ToStdString(entityComponent->Configuration));

		std::shared_ptr< discenfw::Element > elem = entity->IsA("Element") ? std::static_pointer_cast<discenfw::Element>(entity) : nullptr;
		if (elem)
		{
			UpdateElementData(entityActor, elem);
		}
	}
}


void AUnrealScenarioActor::UpdateScenarioCamera()
{
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	FVector pos;
	FVector dir;
	GetViewPoint(pos, dir);
	//ScenarioAPI.Logic->Interaction.CameraPosition = { pos.Y,pos.X,pos.Z };
	//ScenarioAPI.Logic->Interaction.CameraDirection = { dir.Y,dir.X,dir.Z };
}


void AUnrealScenarioActor::GetViewPoint(FVector & pos, FVector & dir)
{
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	FRotator rot;
	playerController->GetPlayerViewPoint(pos, rot);
	dir = rot.Vector();
}


void AUnrealScenarioActor::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	UUScenUtil::Log(Logger::DEBUG, HitComp->GetOwner()->GetName() + TEXT(" hit by ") + OtherComp->GetName());
	//OnBeginCursorOverEntity(HitComp);
}


void AUnrealScenarioActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OverlappedComp->ComponentHasTag(UUScenUtil::GizmoTag) || OtherComp->ComponentHasTag(UUScenUtil::GizmoTag))
	{
		return;
	}
	AActor* entityActor = GetEntityFromActor(OverlappedComp->GetOwner());
	//AConnectionElementActor* connectionElementActor = Cast<AConnectionElementActor>(entityActor);
	AActor* otherEntityActor = GetEntityFromActor(OtherActor);
	//AConnectionElementActor* otherConnectionElementActor = Cast<AConnectionElementActor>(otherEntityActor);
	UElementDataComponent* entityComponent = UUScenUtil::GetElementData(entityActor);
	UElementDataComponent* otherEntityComponent = UUScenUtil::GetElementData(otherEntityActor);
	if (entityComponent && otherEntityComponent
		&& OverlappedComp->ComponentHasTag(UUScenUtil::SocketTag)
		&& OtherComp->ComponentHasTag(UUScenUtil::SocketTag))
	{
		FSocketInfo* thisSocket = entityComponent->GetSocketInfoByComponent(OverlappedComp);
		FSocketInfo* otherSocket = otherEntityComponent->GetSocketInfoByComponent(OtherComp);
		if (thisSocket && otherSocket)
		{
			OverlappedComp->bHiddenInGame = false;
			OtherComp->bHiddenInGame = false;
			bool compatible = UUScenUtil::AreSocketsCompatible(*thisSocket, *otherSocket);
			if (compatible)
			{
				entityComponent->HighlightSocket(OverlappedComp, true, EEditFlags::Connecting);
				otherEntityComponent->HighlightSocket(OtherComp, true, EEditFlags::Connecting);
				//UUEUtil::TogglePrimitiveOutline(OverlappedComp, true, EEditFlags::Connecting);
				//UUEUtil::TogglePrimitiveOutline(OtherComp, true, EEditFlags::Connecting);
			}
			else
			{
				entityComponent->HighlightSocket(OverlappedComp, true, EEditFlags::Overlapping);
				otherEntityComponent->HighlightSocket(OtherComp, true, EEditFlags::Overlapping);
				//UUEUtil::TogglePrimitiveOutline(OverlappedComp, true, EEditFlags::Overlapping);
				//UUEUtil::TogglePrimitiveOutline(OtherComp, true, EEditFlags::Overlapping);
			}
			SocketPrimComp = OverlappedComp;
			OtherSocketPrimComp = OtherComp;
			return;
		}
	}
	if (OverlappedComp->ComponentHasTag(UUScenUtil::SocketTag) || OtherComp->ComponentHasTag(UUScenUtil::SocketTag))
	{
		return;
	}
	//OnBeginCursorOverEntity(OverlappedComp);
	//if (connectionElementActor && connectionElementActor != otherEntityActor && (otherConnectionElementActor == nullptr || !otherConnectionElementActor->AnchorHandles.Find(OtherActor)))
	if (entityComponent && otherEntityComponent && entityActor != otherEntityActor
		&& UUEUtil::GetActorVisible(entityActor) && UUEUtil::GetActorVisible(otherEntityActor))
	{
		//USplineMeshComponent* connectionElementSection = Cast<USplineMeshComponent>(OverlappedComp);
		//USplineMeshComponent* otherConnectionElementSection = Cast<USplineMeshComponent>(OtherComp);
		//if (connectionElementSection)
		//{
		UUScenUtil::Log(Logger::DEBUG,
			entityComponent->Identifier + TEXT(" overlapping with ") + otherEntityComponent->Identifier, true, true, TEXT("OverlappingEntity"));

		UUEUtil::TogglePrimitiveOutline(OverlappedComp, true, EEditFlags::Overlapping);
		UUEUtil::TogglePrimitiveOutline(OtherComp, true, EEditFlags::Overlapping);
		entityActor->Tags.AddUnique(UUScenUtil::OverlappingTag);
		otherEntityActor->Tags.AddUnique(UUScenUtil::OverlappingTag);
		//}
		//ScenarioAPI.Logic->Interaction.OverlappingEntities.insert(GetEntityByName(entityActor->Identifier));
	}
}


void AUnrealScenarioActor::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OverlappedComp->ComponentHasTag(UUScenUtil::GizmoTag) || OtherComp->ComponentHasTag(UUScenUtil::GizmoTag))
	{
		return;
	}
	AActor* entityActor = GetEntityFromActor(OverlappedComp->GetOwner());
	//AConnectionElementActor* connectionElementActor = Cast<AConnectionElementActor>(entityActor);
	AActor* otherEntityActor = GetEntityFromActor(OtherActor);
	//AConnectionElementActor* otherConnectionElementActor = Cast<AConnectionElementActor>(otherEntityActor);
	UElementDataComponent* entityComponent = UUScenUtil::GetElementData(entityActor);
	UElementDataComponent* otherEntityComponent = UUScenUtil::GetElementData(otherEntityActor);
	if (entityComponent && otherEntityComponent
		&& OverlappedComp->ComponentHasTag(UUScenUtil::SocketTag)
		&& OtherComp->ComponentHasTag(UUScenUtil::SocketTag))
	{
		entityComponent->HighlightSocket(OverlappedComp, false, EEditFlags::Connecting | EEditFlags::Overlapping);
		otherEntityComponent->HighlightSocket(OtherComp, false, EEditFlags::Connecting | EEditFlags::Overlapping);
		SocketPrimComp = nullptr;
		OtherSocketPrimComp = nullptr;
		return;
	}
	if (OverlappedComp->ComponentHasTag(UUScenUtil::SocketTag) || OtherComp->ComponentHasTag(UUScenUtil::SocketTag))
	{
		return;
	}

	if (entityComponent && otherEntityComponent && entityActor != otherEntityActor)
	{
		//USplineMeshComponent* connectionElementSection = Cast<USplineMeshComponent>(OverlappedComp);
		//USplineMeshComponent* otherConnectionElementSection = Cast<USplineMeshComponent>(OtherComp);
		//if (connectionElementSection)
		//{
		UUScenUtil::Log(Logger::DEBUG, entityComponent->Identifier + TEXT(" no more overlapping with ") + otherEntityComponent->Identifier, true, true, TEXT("OverlappingEntity"));
		if (entityActor->ActorHasTag(TEXT("moved")))
		{
			UUEUtil::TogglePrimitiveOutline(OverlappedComp, true, EEditFlags::Overlapping);
		}
		else
		{
			UUEUtil::TogglePrimitiveOutline(OverlappedComp, false, EEditFlags::Overlapping);
		}
		if (otherEntityActor->ActorHasTag(TEXT("moved")))
		{
			UUEUtil::TogglePrimitiveOutline(OtherComp, true, EEditFlags::Overlapping);
		}
		else
		{
			UUEUtil::TogglePrimitiveOutline(OtherComp, false, EEditFlags::Overlapping);
		}
		entityActor->Tags.Remove(UUScenUtil::OverlappingTag);
		otherEntityActor->Tags.Remove(UUScenUtil::OverlappingTag);
		//}
		//ScenarioAPI.Logic->Interaction.OverlappingEntities.erase(GetEntityByName(entityActor->Identifier));
	}
}


//void AUnrealScenarioActor::OnActorOverlapBegin(AActor* ThisActor, AActor* OtherActor)
//{
//	UUScenUtil::Log(Logger::DEBUG, ThisActor->GetName() + TEXT(" overlapping with ") + OtherActor->GetName());
//}


//void AUnrealScenarioActor::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
//{
//	UUScenUtil::Log(Logger::DEBUG, SelfActor->GetName() + TEXT(" hit by ") + OtherActor->GetName());
//}


//void AUnrealScenarioActor::OnActorClicked(AActor* TouchedActor, FKey ButtonPressed)
//{
//	// TODO: Find a way to send a selection event from blueprint
//	// TODO: clear selection if nothing is selected
//	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(TouchedActor);
//	if (entityComponent && ButtonPressed == EKeys::LeftMouseButton)
//	{
//		UUScenUtil::Log(Logger::DEBUG, TEXT("Clicked: ") + entityComponent->Identifier);
//		//SelectEntityFromActor(TouchedActor);
//	}
//}


void AUnrealScenarioActor::OnBeginCursorOverComponent(UPrimitiveComponent* TouchedComponent)
{
	if (MovingEntityActor == nullptr)
	{
		OnBeginCursorOverEntity(TouchedComponent);
	}
}


void AUnrealScenarioActor::OnEndCursorOverComponent(UPrimitiveComponent* TouchedComponent)
{
	if (MovingEntityActor == nullptr)
	{
		OnEndCursorOverEntity(TouchedComponent);
	}
}


void AUnrealScenarioActor::OnClickedComponent(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	AActor* selectedEntityActor = SelectEntityFromActor(ClickedComp->GetOwner());
	if (selectedEntityActor != nullptr && MovingEntityActor == nullptr)
	{
		OnEntityClicked(selectedEntityActor, ClickedComp);
		return;
	}
	else if (SelectedEntityActor == selectedEntityActor && MovingEntityActor != nullptr)
	{
		//AConnectionElementActor* selConnectionElementActor = Cast<AConnectionElementActor>(SelectedEntityActor);
		//if (selConnectionElementActor)
		//{
		//	selConnectionElementActor->OnMouseClickedHandle(ClickedComp, ButtonPressed);
		//}
		SaveEntityPosition();
		SelectEntityActor(nullptr);
		return;
	}
	else if (selectedEntityActor == nullptr && MovingEntityActor == nullptr)
	{
		//AConnectionElementActor* selConnectionElementActor = Cast<AConnectionElementActor>(SelectedEntityActor);
		//if (selConnectionElementActor)
		//{
		//	selConnectionElementActor->OnMouseClickedHandle(ClickedComp, ButtonPressed);
		//	selConnectionElementActor->ToggleActiveHandle(false);
		//}
		SelectEntityActor(nullptr);
	}
	SelectEntityActor(selectedEntityActor);
}


void AUnrealScenarioActor::OnBeginCursorOverEntity(UPrimitiveComponent* TouchedComponent)
{
	AActor* entityActor = GetEntityFromActor(TouchedComponent->GetOwner());
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(entityActor);
	if (entityComponent)
	{
		UUScenUtil::Log(Logger::DEBUG, TEXT("Mouse over ") + entityComponent->Identifier + TEXT(",") + TouchedComponent->GetName(), false, true, TEXT("MOUSE_OVER"));
		if (TouchedComponent->ComponentHasTag("Interactive"))
		{
			HighlighedPrimComp = TouchedComponent;
			// do not highlight the selected entity, it is already highlighted
			if (entityActor != SelectedEntityActor)
			{
				HighlightComponent(HighlighedPrimComp, true);
			}
		}
		else
		{
			// do not highlight the selected entity, it is already highlighted
			if (entityActor != SelectedEntityActor)
			{
				HighlightActor(entityActor, true);
			}
		}
		//AConnectionElementActor* connectionElementActor = Cast<AConnectionElementActor>(entityActor);
	}
	HighlighedEntityActor = entityActor;
}


void AUnrealScenarioActor::OnEndCursorOverEntity(UPrimitiveComponent* TouchedComponent)
{
	AActor* entityActor = UUScenUtil::GetEntityFromActor(TouchedComponent->GetOwner());
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(entityActor);
	if (entityComponent)
	{
		if (HighlighedPrimComp)
		{
			// leave selected entity highlighted
			if (entityActor != SelectedEntityActor)
			{
				HighlightComponent(HighlighedPrimComp, false);
			}
			HighlighedPrimComp = nullptr;
		}
		else
		{
			// leave selected entity highlighted
			if (entityActor != SelectedEntityActor)
			{
				HighlightActor(entityActor, false);
			}
		}
	}
	HighlighedEntityActor = nullptr;
}


void AUnrealScenarioActor::OnEntityClicked(AActor* selectedEntityActor, UPrimitiveComponent* ClickedComp)
{
	// if interacting with an interactive component do not affect its owner entity
	if (HighlighedPrimComp)
	{
		return;
	}

	//AConnectionElementActor* connectionElementActor = Cast<AConnectionElementActor>(selectedEntityActor);
	//if (connectionElementActor)
	//{
	//	connectionElementActor->bMouseInteractionEnabled = this->bMouseInteractionEnabled;
	//	connectionElementActor->OnMouseClickedHandle(ClickedComp, ButtonPressed);
	//}

	//if (SelectedEntityActor != nullptr && SelectedEntityActor != selectedEntityActor)
	if (SelectedEntityActor != selectedEntityActor)
	{
		SelectEntityActor(selectedEntityActor);
		//if (connectionElementActor)
		//{
		//	connectionElementActor->ToggleActiveHandle(true);
		//	connectionElementActor->bMouseInteractionEnabled = this->bMouseInteractionEnabled;
		//}
		// TODO: show context menu
		//return;
	}
	//else
	{
		DragSelectedEntity();
	}
}


void AUnrealScenarioActor::OnCancel()
{
	if (IsEditing())
	{
		CancelEditing();
	}
	else
	{
		SelectEntityActor(nullptr);
	}
}


void AUnrealScenarioActor::OnScenarioLoaded()
{
	CancelEditing();
}


void AUnrealScenarioActor::OnEntityAdded()
{
	//CancelEditing();
}


void AUnrealScenarioActor::OnEntityRemoved()
{
	CancelEditing();
}


bool AUnrealScenarioActor::IsEditing()
{
	return MovingEntityActor != nullptr;
}


void AUnrealScenarioActor::UpdateEditing(float DeltaTime)
{
	////UUEUtil::MouseDragActor(MovingEntityActor, MovingEntityDistance);
	if (MovingEntityActor)
	{
		UpdatePositioning();
		// rotate
		// TODO: customizable controls
		//float mouseWheelUp = playerController->GetInputAxisKeyValue(EKeys::MouseScrollUp);
		//float mouseWheelDown = playerController->GetInputAxisKeyValue(EKeys::MouseScrollDown);
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		float mouseWheelUp = playerController->WasInputKeyJustPressed(EKeys::MouseScrollUp);//EKeys::Z);
		float mouseWheelDown = playerController->WasInputKeyJustPressed(EKeys::MouseScrollDown);//(EKeys::X);
		bool mouseWheel = mouseWheelUp != 0.0f || mouseWheelDown != 0.0f;
		if (mouseWheel)
		{
			bool ctrlPressed = playerController->IsInputKeyDown(EKeys::LeftControl) || playerController->IsInputKeyDown(EKeys::RightControl);
			if (ctrlPressed)
			{
				UUEUtil::RotateAlongVerticalAxis(MovingEntityActor, mouseWheelUp ? 15 : -15);
			}
			else
			{
				MovingEntityDistance += mouseWheelUp ? 15 : -15;
			}
		}
		OnEntityMoved(MovingEntityActor);
	}
}

void AUnrealScenarioActor::UpdateSimulation(float DeltaTime)
{
	using namespace discenfw;
	auto simulationController = DiScenFw()->ScenarioSimulationController();
	//UpdateScenarioCamera();
	simulationController->SetSimulationTimeSpeed(SimulationSpeedMultiplier);
	simulationController->SetSimulationProgress(SimulationProgress / 100.0);
	if (simulationController->ValidSimulation() && simulationController->SimulationStarted())
	{
		DiScenFw()->ScenarioSimulation()->UpdateSimulation(DeltaTime);
		SimulationProgress = (float)simulationController->ComputeSimulationProgress()*100.0;
		discenfw::DateTime dt = simulationController->GetSimulationDateTime();

		// TODO: discenfw functions must return a string or a struct containing strings
		//dt = discenfw::StringToDateTime(discenfw::DateTimeToString(dt));
		//FString dtFStr(discenfw::DateTimeToString(dt));
		//std::string dtStr = discenfw::DateTimeToString(dt);
		//FString dtFStr = "Simulation time: "+UUEUtil::ToFString(dtStr);
		FString dtFStr = FString::Printf(TEXT("Simulation time: %4d-%02d-%02d %02d:%02d:%02d.%03d"),
			dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute, dt.Second, dt.Millisecond);
		UUEUtil::Log.LogMessage(Logger::DEBUG, dtFStr, "UnrealScenario", false, true, "SimTime");
	}
}


void AUnrealScenarioActor::CancelEditing()
{
	if (MovingEntityActor)
	{
		AActor* movedActor = MovingEntityActor;
		CancelPositioning();
		OnStopMovingEntity(movedActor);
	}
	if (SocketPrimComp && OtherSocketPrimComp)
	{
		AActor* entityActor = GetEntityFromActor(SocketPrimComp->GetOwner());
		AActor* otherEntityActor = GetEntityFromActor(OtherSocketPrimComp->GetOwner());
		UElementDataComponent* entityComponent = UUScenUtil::GetElementData(entityActor);
		UElementDataComponent* otherEntityComponent = UUScenUtil::GetElementData(otherEntityActor);
		// Save pointers before they can be reset by end-overlap callbacks
		UPrimitiveComponent* thisSocketComponent = SocketPrimComp;
		UPrimitiveComponent* otherSocketComponent = OtherSocketPrimComp;
		entityComponent->HighlightSocket(thisSocketComponent, false, EEditFlags::Connecting | EEditFlags::Overlapping);
		otherEntityComponent->HighlightSocket(otherSocketComponent, false, EEditFlags::Connecting | EEditFlags::Overlapping);
	}
	SocketPrimComp = nullptr;
	OtherSocketPrimComp = nullptr;
}


void AUnrealScenarioActor::SaveEntityPosition()
{
	if (MovingEntityActor)
	{
		// update position and stop
		UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(MovingEntityActor);
		entityComponent->bModified = true;
		OnStopMovingEntity(MovingEntityActor);
		MovingEntityActor = nullptr;
	}
}


void AUnrealScenarioActor::DragSelectedEntity()
{
	//if (connectionElementActor == nullptr)
	//{
	MovingEntityActor = SelectedEntityActor;
	MovingEntityStartPos = MovingEntityActor->GetActorLocation();
	MovingEntityStartRot = MovingEntityActor->GetActorRotation();
	MovingEntityDistance = UUEUtil::GetDistanceFromEyesViewPoint(MovingEntityActor);

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	FVector dragPos;
	if (UUEUtil::MouseDragLocation(playerController, MovingEntityDistance, dragPos))
	{
		MovingEntityOffset = MovingEntityStartPos - dragPos;
	}
	else
	{
		MovingEntityOffset = FVector::ZeroVector;
	}
	OnStartMovingEntity(MovingEntityActor);
	//}
	// TODO: hide context menu
}


//---------------------------------------------------------------------
// DiScenFw connector

std::shared_ptr< discenfw::Entity > AUnrealScenarioActor::GetEntityByName(const FString& name)
{
	std::string id(UUEUtil::ToStdString(name));
	using namespace discenfw;
	return DiScenFw()->GetEntity(id);
}


std::shared_ptr< discenfw::Element > AUnrealScenarioActor::GetElementByName(const FString& name)
{
	std::string id(UUEUtil::ToStdString(name));
	using namespace discenfw;
	return DiScenFw()->GetElement(id);
}


AActor* AUnrealScenarioActor::GetEntityActorFromId(const std::string& id)
{
	FString entityId = UUEUtil::ToFString(id);
	return GetEntityActorFromId(entityId);
}


FVector AUnrealScenarioActor::ImportVector(const discenfw::Vector3D& vec)
{
	return{ vec.Forward, vec.Right, vec.Up };
}


FTransform AUnrealScenarioActor::ImportCoordSysRot(const discenfw::CoordSys3D& coordSys)
{
	FTransform actorTransform;
	FVector actorForward = { coordSys.ForwardAxis.Forward, coordSys.ForwardAxis.Right, coordSys.ForwardAxis.Up };
	//FVector actorRight = { coordSys.RightAxis.Forward, coordSys.RightAxis.Right, coordSys.RightAxis.Up };
	FVector actorUp = { coordSys.UpAxis.Forward, coordSys.UpAxis.Right, coordSys.UpAxis.Up };
	FMatrix actorMat = FRotationMatrix::MakeFromXZ(actorForward, actorUp);
	actorTransform.SetFromMatrix(actorMat);
	return actorTransform;
}


void AUnrealScenarioActor::SetElementActorTransform(
	AActor* targetActor,
	const discenfw::LocalTransform& localTransform
)
{
	if (!localTransform.ParentId.empty())
	{
		AActor* parentActor = GetEntityFromId(UUEUtil::ToFString(localTransform.ParentId));
		if (parentActor && targetActor->GetAttachParentActor() != parentActor)
		{
			targetActor->AttachToActor(parentActor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	else
	{
		if (ScenarioActor && targetActor->GetAttachParentActor() != ScenarioActor)
		{
			targetActor->AttachToActor(ScenarioActor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	FVector elemLoc = ImportVector(localTransform.Location) * UnitScale;
	FVector elemEuler = ImportVector(localTransform.EulerAngles);
	FVector elemScale = ImportVector(localTransform.Scale);
	FRotator elemRot = FRotator::MakeFromEuler(elemEuler);

	FVector actorOrigin = ImportVector(localTransform.CoordSys.Origin) * UnitScale;

	if (localTransform.UseCoordSys)
	{
		FTransform actorTransform = ImportCoordSysRot(localTransform.CoordSys);
		targetActor->SetActorRelativeTransform(actorTransform);
		targetActor->SetActorRelativeLocation(actorOrigin);
		if (elemScale != targetActor->GetActorRelativeScale3D())
		{
			targetActor->SetActorRelativeScale3D(elemScale);
		}
	}
	else
	{
		targetActor->SetActorRelativeLocation(elemLoc);
		targetActor->SetActorRelativeRotation(elemRot);
		if (elemScale != targetActor->GetActorRelativeScale3D())
		{
			targetActor->SetActorRelativeScale3D(elemScale);
		}
	}
}


AActor* AUnrealScenarioActor::ImportEntity(const std::shared_ptr< const discenfw::Entity > entity, AActor* parentActor)
{
	std::string className = entity->GetClassName();
	FString entityClassStr(className.c_str());
	FString entityIdStr(entity->GetIdentifier().c_str());

	if (EntityActorExists(UUEUtil::ToFString(entity->GetIdentifier())))
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("More entities with the same identifier: ") + entityIdStr);
		return nullptr;
	}

	using namespace discenfw;

	AActor* newActor = nullptr;
	AssetReference::SourceType assetSource = entity->GetAssetReference().Source;
	switch (assetSource)
	{
	case AssetReference::SCENE:
		newActor = CreateEntityFromActor(UUEUtil::ToFString(entity->GetAssetReference().Uri), entityIdStr, parentActor);
		break;
	case AssetReference::PROJECT:
		newActor = CreateEntityFromAsset(UUEUtil::ToFString(entity->GetAssetReference().Uri), entityIdStr, parentActor);
		break;
	default:
		newActor = CreateNewEntity(className, entity->GetIdentity().Category, entity->GetIdentifier(), parentActor);
		break;
	}

	if (newActor == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to create entity: ") + entityClassStr);
		return nullptr;
	}

	// Set other Entity members
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(newActor);
	if (entityComponent)
	{
		entityComponent->Type = entity->GetIdentity().Type.c_str();
		entityComponent->Category = entity->GetIdentity().Category.c_str();
		entityComponent->Description = entity->GetIdentity().Description.c_str();
		entityComponent->Configuration = entity->GetConfiguration().c_str();
		entityComponent->EntityAssetId = entity->GetAssetReference().Uri.c_str();
		entityComponent->bModified = false;
	}

	//if (entity->IsA("Aggregate"))
	//{
	//	const std::shared_ptr< Aggregate > sys = std::static_pointer_cast<Aggregate>(entity);
	//	AAggregateActor* newAggregateActor = Cast<AAggregateActor>(newActor);
	//	if (newAggregateActor == nullptr)
	//	{
	//		newActor->Destroy();
	//		UUScenUtil::Log(Logger::ERROR, TEXT("Catalog class mismatch: ") + UUEUtil::ToFString(className) + TEXT(" <--> ") + UUEUtil::ToFString(entity->Asset.Uri));
	//		return nullptr;
	//	}
	//	ImportAggregate(sys, newAggregateActor);
	//}
	//else
	if (entity->IsA("Element"))
	{
		//UUScenUtil::Log(Logger::DEBUG, TEXT("Actor found for element type ") + entityTypeStr);
		const std::shared_ptr< const Element > elem = std::static_pointer_cast<const Element>(entity);
		ImportElement(elem, newActor);
	}

	SetActorSelectable(newActor);

	//EntityActors.Add(FName(*entityIdStr), newActor);
	EntityActors.Add(newActor);

	return newActor;
}


void AUnrealScenarioActor::ImportElement(
	const std::shared_ptr< const discenfw::Element > elem,
	AActor* newActor)
{
	SetElementActorTransform(newActor, elem->GetTransform());
	// Set other Entity members
	UElementDataComponent* elementComponent = UUScenUtil::GetElementData(newActor);
	if (elementComponent)
	{
		elementComponent->ScanSockets();

		// TODO: import type and compatibility

		for (const auto& socket : elem->GetSockets())
		{
			const discenfw::SocketInfo socketInfo = elem->GetSocket(socket.Id);
			//	UPrimitiveComponent* socketComp = elementComponent->GetSocketComponent(UUEUtil::ToFString(socket.first));
			//	if (socketComp)
			//	{
			//		FTransform socketTransform = ImportCoordSysRot(elem->Transform.CoordSys);
			//		socketComp->SetRelativeLocationAndRotation(socketTransform.GetLocation(), socketTransform.GetRotation());
			//	}
		}
	}

	if (elem->IsA("ConnectionElement"))
	{
		const std::shared_ptr< const discenfw::ConnectionElement > connectionElement =
			std::static_pointer_cast<const discenfw::ConnectionElement>(elem);

		ImportConnectionElement(connectionElement, newActor);
	}
}


/*

void AUnrealScenarioActor::ImportAggregate(const std::shared_ptr< discenfw::Aggregate > system, AAggregateActor* newActor)
{
	FString entityIdStr(system->Identifier.c_str());
	bool failedToImport = false;
	for (size_t i = 0; i < system->Components.size(); i++)
	{
		if (ImportEntity(system->Components[i], newActor) == nullptr)
		{
			failedToImport = true;
		}
	}
	if (failedToImport)
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Errors importing aggregate ") + entityIdStr + TEXT("One or more components could not be imported."));
	}
}
*/

void AUnrealScenarioActor::ImportConnectionElement(
	const std::shared_ptr< const discenfw::ConnectionElement > connectionElement,
	AActor* connectionElementActor
	)
{
	// Update connection element according to path
	UActorComponent* actorComponent = connectionElementActor->GetComponentByClass(UConnectionElementComponent::StaticClass());
	UConnectionElementComponent* connectionComponent = Cast<UConnectionElementComponent>(actorComponent);
	connectionComponent->ConnectionPath.Reset();
	for (size_t i = 0; i < connectionElement->GetNumAnchors(); i++)
	{
		const discenfw::Anchor& anchor = connectionElement->GetAnchor(i);
		FConnectionAnchor connAnchor;
		connAnchor.ElementId = UUEUtil::ToFString(anchor.ElementId);
		connAnchor.SocketId = UUEUtil::ToFString(anchor.SocketId);
		connAnchor.Offset = ImportVector(anchor.Offset);
		connectionComponent->ConnectionPath.Add(connAnchor);
	}
	PendingConnectionElements.Add(connectionComponent);
}




void AUnrealScenarioActor::LerpActorTransform(
	AActor* targetActor,
	discenfw::LocalTransform transform1,
	discenfw::LocalTransform transform2,
	float trim
	)
{
	AActor* parentActor1 = ScenarioActor;
	if (!transform1.ParentId.empty())
	{
		parentActor1 = GetEntityFromId(UUEUtil::ToFString(transform1.ParentId));
	}
	if (parentActor1 && targetActor->GetAttachParentActor() != parentActor1)
	{
		targetActor->AttachToActor(parentActor1, FAttachmentTransformRules::KeepWorldTransform);
	}
	FVector elemLoc1 = ImportVector(transform1.Location);
	FVector elemEuler1 = ImportVector(transform1.EulerAngles);
	FVector elemScale1 = ImportVector(transform1.Scale);
	FRotator elemRot1 = FRotator::MakeFromEuler(elemEuler1);
	if (transform1.UseCoordSys)
	{
		FTransform actorTransform = ImportCoordSysRot(transform1.CoordSys);
		elemLoc1 = ImportVector(transform1.CoordSys.Origin);
		elemRot1 = actorTransform.GetRotation().Rotator();
	}

	FVector elemLoc2 = ImportVector(transform2.Location);
	FVector elemEuler2 = ImportVector(transform2.EulerAngles);
	FVector elemScale2 = ImportVector(transform2.Scale);
	FRotator elemRot2 = FRotator::MakeFromEuler(elemEuler2);
	if (transform1.UseCoordSys)
	{
		FTransform actorTransform = ImportCoordSysRot(transform2.CoordSys);
		elemLoc2 = ImportVector(transform2.CoordSys.Origin);
		elemRot2 = actorTransform.GetRotation().Rotator();
	}
	bool differentParents = (transform1.ParentId != transform2.ParentId);
	if (differentParents)
	{
		AActor* parentActor2 = ScenarioActor;
		if (!transform2.ParentId.empty())
		{
			parentActor2 = GetEntityFromId(UUEUtil::ToFString(transform2.ParentId));
		}
		FTransform actorTransform1 = parentActor1 ? parentActor1->GetActorTransform() : ScenarioActor->GetActorTransform();
		FTransform actorTransform2 = parentActor2 ? parentActor2->GetActorTransform() : ScenarioActor->GetActorTransform();
		//#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 17
		//		elemRot1 = actorTransform1.TransformRotation(elemRot1.Quaternion()).Rotator();
		//		elemRot2 = actorTransform2.TransformRotation(elemRot2.Quaternion()).Rotator();
		//#else
		FQuat quat1 = actorTransform1.GetRotation()*elemRot1.Quaternion();
		FQuat quat2 = actorTransform2.GetRotation()*elemRot2.Quaternion();
		elemRot1 = quat1.Rotator();
		elemRot2 = quat2.Rotator();
		//#endif
		elemLoc1 = actorTransform1.TransformPosition(elemLoc1);
		elemLoc2 = actorTransform2.TransformPosition(elemLoc2);
		elemScale1 = actorTransform1.TransformVector(elemScale1);
		elemScale2 = actorTransform2.TransformVector(elemScale2);
	}
	FVector elemLoc = FMath::Lerp(elemLoc1, elemLoc2, trim);
	FVector elemScale = FMath::Lerp(elemScale1, elemScale2, trim);
	FRotator elemRot = FMath::Lerp(elemRot1, elemRot2, trim);

	if (differentParents)
	{
		targetActor->SetActorLocationAndRotation(elemLoc, elemRot);
	}
	else
	{
		targetActor->SetActorRelativeLocation(elemLoc);
		targetActor->SetActorRelativeRotation(elemRot);
	}
	if (elemScale != targetActor->GetActorRelativeScale3D())
	{
		targetActor->SetActorRelativeScale3D(elemScale);
	}
}







AActor* AUnrealScenarioActor::CreateNewEntity(
	const std::string& entityClass,
	const std::string& entityType,
	const std::string& entityName,
	AActor* parentActor
	)
{
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	if (!entityName.empty())
	{
		SpawnParams.Name = UUEUtil::ToFName(entityName);
	}
	AActor* newActor = nullptr;
	if (entityClass == "Entity")
	{
		newActor = World->SpawnActor<AEntityActor>(AEntityActor::StaticClass(), SpawnParams);
	}
	else if (entityClass == "Element")
	{
		newActor = World->SpawnActor<AElementActor>(AElementActor::StaticClass(), SpawnParams);
	}
	//else if (entityClass == "Aggregate")
	//{
	//	newActor = World->SpawnActor<AAggregateActor>(AAggregateActor::StaticClass(), SpawnParams);
	//}
	//else if (entityClass == "ConnectionElement")
	//{
	//	newActor = World->SpawnActor<AConnectionElementActor>(AConnectionElementActor::StaticClass(), SpawnParams);
	//}
	//else if (entityClass == "Assembly")
	//{
	//	newActor = World->SpawnActor<AAssemblyActor>(AAssemblyActor::StaticClass(), SpawnParams);
	//}

	if (newActor == nullptr)
	{
		UUEUtil::Log.LogMessage(Logger::ERROR, "No class found for entity type " + FString(entityType.c_str()), "UnrealScenario");
		return nullptr;
	}

	if (parentActor)
	{
		newActor->AttachToActor(parentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	}
	newActor->SetActorLocation(FVector::ZeroVector);
	newActor->SetActorRotation(FRotator::ZeroRotator);
	newActor->SetActorScale3D(FVector::OneVector);
#if WITH_EDITOR
	newActor->SetActorLabel(entityName.c_str());
#endif

	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(newActor);
	entityComponent->Identifier = UUEUtil::ToFString(entityName);
	entityComponent->Type = UUEUtil::ToFString(entityType);

	entityComponent->Identifier = UUEUtil::ToFString(entityName);
	entityComponent->Category = UUEUtil::ToFString(entityType);
	return newActor;
}


void AUnrealScenarioActor::UpdateElementData(AActor* elemActor, std::shared_ptr< discenfw::Element > elem)
{
	UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(elemActor);
	if (entityComponent)
	{
		FString entityName = entityComponent->Identifier;
		if (elem)
		{
			UpdateElementTransform(elemActor, elem);
			UElementDataComponent* elementComponent = UUScenUtil::GetElementData(elemActor);
			if (elementComponent)
			{
				elem->ClearSockets();
				elementComponent->ScanSockets();
				for (const FSocketInfo& socket : elementComponent->SocketComponents)
				{
					FString socketId = socket.Id;
					std::string id = UUEUtil::ToStdString(socketId);
					const UPrimitiveComponent* socketComp = Cast<UPrimitiveComponent>(socket.Component);
					FVector socketLoc = socketComp->GetComponentLocation();
					FRotator socketRot = socketComp->GetComponentRotation();
					discenfw::SocketInfo socketInfo = elem->GetSocket(id);
					socketInfo.Id = id; // initialize if not found
					socketInfo.Type = UUEUtil::ToStdString(socket.Type);
					socketInfo.Location = ExportCoordSys(socketLoc, socketRot);
					socketInfo.Compatibility.clear();
					for (const FString& type : socket.Compatibility)
					{
						socketInfo.Compatibility.push_back(UUEUtil::ToStdString(type));
					}
					elem->SetSocket(socketInfo);
				}
				if (elem->IsA("ConnectionElement"))
				{
					std::shared_ptr< discenfw::ConnectionElement > connectionElement = std::static_pointer_cast<discenfw::ConnectionElement>(elem);
					UConnectionElementComponent* connectionComponent = Cast<UConnectionElementComponent>(elementComponent);
					if (connectionComponent)
					{
						connectionElement->ClearConnectionPath();
						connectionComponent->ImportConnectionPath();
						for (FConnectionAnchor connAnchor : connectionComponent->ConnectionPath)
						{
							std::string elementId = UUEUtil::ToStdString(connAnchor.ElementId);
							std::string socketId = UUEUtil::ToStdString(connAnchor.SocketId);
							//if (connAnchor.Component != nullptr)
							//{
							//	AActor* actor = connAnchor.Component->GetOwner();
							//	UActorComponent* actorComponent = actor->GetComponentByClass(UElementDataComponent::StaticClass());
							//	UElementDataComponent* elementComponent = CastChecked<UElementDataComponent>(actorComponent);
							//	elementId = UUEUtil::ToStdString(elementComponent->Identifier);
							//	if (connAnchor.Component->ComponentHasTag(UUScenUtil::SocketTag))
							//	{
							//		socketId = UUEUtil::ToStdString(connAnchor.Component->GetName());
							//	}
							//}
							discenfw::Vector3D offset = ExportVector(connAnchor.Offset);
							connectionElement->AddAnchor(
							{
								offset,
								elementId,
								socketId
							});
						}
					}
				}
			}
		}

		//AConnectionElementActor* connectionElementActor = Cast<AConnectionElementActor>(elemActor);
		//if (connectionElement && connectionElementActor)
		//{
		//	connectionElement->Path.Points.clear();
		//	for (int32 i = 0; i < connectionElementActor->ConnectionElementPath.Num(); i++)
		//	{
		//		FVector point = connectionElementActor->ConnectionElementPath[i];
		//		connectionElement->Path.Points.push_back(Vector3D({ point.Y,point.X,point.Z }));
		//	}
		//}
	}
}


void AUnrealScenarioActor::UpdateElementTransform(AActor* elemActor, std::shared_ptr< discenfw::Element > elem)
{
	if (elem && elemActor)
	{
		// conversion between systems

		FVector actorLoc = elemActor->GetActorLocation() / UnitScale;
		elem->GetTransform3D().Location = ExportVector(actorLoc);
		// rotation
		FRotator actorRot = elemActor->GetActorRotation();
		FVector elemEuler = actorRot.Euler();
		//discenfw::Vector3D elemRotVec = ExportVector(elemEuler);
		discenfw::Vector3D elemRotVec = { (float)elemEuler.X, (float)elemEuler.Y, (float)elemEuler.Z };
		elem->GetTransform3D().EulerAngles = elemRotVec;
		// TODO: rotation matrix
		elem->GetTransform3D().UseCoordSys = true;
		//if (elem->Transform.UseCoordSys)
		{
			elem->GetTransform3D().CoordSys = ExportCoordSys(actorLoc, actorRot);
		}
		// scale
		FVector actorScale = elemActor->GetActorRelativeScale3D();
		discenfw::Vector3D elemScale = ExportVector(actorScale);
		elem->GetTransform3D().Scale = elemScale;
	}
}


discenfw::Vector3D AUnrealScenarioActor::ExportVector(const FVector& vec)
{
	return { (float)vec.Y, (float)vec.X, (float)vec.Z };
}


discenfw::CoordSys3D AUnrealScenarioActor::ExportCoordSys(const FVector& uePos, const FRotator& ueRot)
{
	discenfw::CoordSys3D coordSys;
	FQuat elemQuat = ueRot.Quaternion();
	// TODO: precision should be customizable
	const float precision = 0.00001f;
	FVector ueRt = elemQuat.GetRightVector().GridSnap(precision);
	FVector ueFwd = elemQuat.GetForwardVector().GridSnap(precision);
	FVector ueUp = elemQuat.GetUpVector().GridSnap(precision);
	coordSys.Origin = ExportVector(uePos);
	coordSys.RightAxis = ExportVector(ueRt);
	coordSys.ForwardAxis = ExportVector(ueFwd);
	coordSys.UpAxis = ExportVector(ueUp);

	return coordSys;
}


void AUnrealScenarioActor::BuildConnectionElement(UConnectionElementComponent* PendingConnectionComponent)
{
	for (auto& ref : PendingConnectionComponent->ConnectionPath)
	{
		if (ref.ElementId.IsEmpty())
		{
			ref.Component = nullptr;
			//if (ref.SocketId.IsEmpty())
			//{
			//	ref.Component = PendingConnectionComponent->GetOwner()->GetRootComponent();
			//}
			//else
			//{
			//	ref.Component = PendingConnectionComponent->GetSocketComponent(ref.SocketId);
			//}
		}
		else
		{
			AActor* entityActor = GetEntityActorFromId(ref.ElementId);
			UActorComponent* actorComponent = entityActor->GetComponentByClass(UElementDataComponent::StaticClass());
			UElementDataComponent* elementComponent = Cast<UElementDataComponent>(actorComponent);
			if (ref.SocketId.IsEmpty())
			{
				ref.Component = elementComponent->GetOwner()->GetRootComponent();
			}
			else
			{
				elementComponent->ScanSockets();
				ref.Component = elementComponent->GetSocketComponent(ref.SocketId);
			}
		}
		PendingConnectionComponent->ExportConnectionPath();
		SetActorSelectable(PendingConnectionComponent->GetOwner());
		AEditableConnectionActor* connActor = Cast<AEditableConnectionActor>(PendingConnectionComponent->GetOwner());
		if (connActor)
		{
			connActor->SetHandlesEnabled(false);
		}
	}
}


void AUnrealScenarioActor::BuildConnectionElements()
{
	for (UConnectionElementComponent* connComp : PendingConnectionElements)
	{
		BuildConnectionElement(connComp);
	}

	PendingConnectionElements.Reset();
}

