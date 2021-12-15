// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "EditableConnectionActor.h"

#include "UEUtil.h"
#include "UScenUtil.h"
#include "ElementDataComponent.h"

#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h" // GetWorld()
#include "Runtime/Launch/Resources/Version.h" // ENGINE_MINOR_VERSION

#include <algorithm>

AEditableConnectionActor::AEditableConnectionActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bSnapToAxis = false;
}


// Called every frame
void AEditableConnectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	if (playerController)
	{
		bool ctrlPressed = (playerController->IsInputKeyDown(EKeys::LeftControl) || playerController->IsInputKeyDown(EKeys::RightControl));
		bool altPressed = (playerController->IsInputKeyDown(EKeys::LeftAlt) || playerController->IsInputKeyDown(EKeys::RightAlt));
		bool shiftPressed = (playerController->IsInputKeyDown(EKeys::LeftShift) || playerController->IsInputKeyDown(EKeys::RightShift));
		bool confirmAction = playerController->WasInputKeyJustPressed(EKeys::LeftMouseButton);

		bool cancelAction = playerController->WasInputKeyJustPressed(EKeys::Z) && ctrlPressed;
		cancelAction = cancelAction || playerController->WasInputKeyJustPressed(EKeys::RightMouseButton);
		bool addSectionAction = playerController->WasInputKeyJustPressed(EKeys::Insert) || playerController->WasInputKeyJustPressed(EKeys::MiddleMouseButton);

		// confirm
		if (MovingHandle && confirmAction && IsValidAnchorIndex(ActiveAnchorIndex))
		{
			OnEditConfirm();
			if (ctrlPressed)
			{
				if (altPressed)
				{
					OnEditAddBefore();
				}
				else
				{
					OnEditAddAfter();
				}
			}
			else
			{
				SelectAnchor(-1);
			}
			ClickedComponent = nullptr;
			return;
		}

		if (ClickedComponent)
		{
			AActor* handle = ClickedComponent->GetOwner();
			ClickedComponent = nullptr;
			//if (MovingHandle && IsValidAnchorIndex(ActiveAnchorIndex))
			//{
			//	MovingHandle->SetActorLocation(ConnectionPath[ActiveAnchorIndex]);
			//	UpdateSections();
			//	SetModified(true);
			//	MovingHandle = nullptr;
			//	SelectAnchor(-1);
			//	return;
			//}
			AActor* prevHandle = IsValidAnchorIndex(ActiveAnchorIndex) ? AnchorHandles[ActiveAnchorIndex] : nullptr;
			////if (prevHandle != handle)
			////{
			////	// if a new handle is selected just change selection
			////	return;
			////}
			// else start dragging it
			//else
			//{
			//	MovingHandle = nullptr;
			//}
			if (ctrlPressed && AnchorHandles.Find(handle, ActiveAnchorIndex))
			{
				if (altPressed)
				{
					OnEditAddBefore();
				}
				else
				{
					OnEditAddAfter();
				}
			}
			else
			{
				SavedConnectionPath = ConnectionPath;
				DragHandle(handle);
			}
			PrintAnchorInfo();
			return;
		}

		// cancel
		if (MovingHandle && cancelAction)
		{
			OnEditCancel();
			return;
		}


		// add anchor
		if (MovingHandle && addSectionAction)
		{
			OnEditAddBefore();
			return;
		}

		if (bMouseInteractionEnabled)
		{
			UpdateMouseInteraction();
			bChangeSnapToAxis = shiftPressed;
		}
		else
		{
			UpdateVrInteraction();
		}
	}

	if (UpdateDelay <= 0.0f)
	{
		bUpdateNeeded = true;
	}

	UpdateOnlyIfNeeded();

	if (!TimerHandle.IsValid())
	{
		StartUpdateTimer();
	}
}


bool AEditableConnectionActor::ShouldTickIfViewportsOnly() const
{
	return true;
}




bool AEditableConnectionActor::IsValidAnchorIndex(int32 PointIndex)
{
	return PointIndex >= 0 && PointIndex < ConnectionPath.Num();
}


void AEditableConnectionActor::SelectAnchor(int32 PointIndex)
{
	if (AnchorHandles.Num() > 0)
	{
		bool validIndex = IsValidAnchorIndex(PointIndex);
		if (validIndex)
		{
			ActiveAnchorHandle = AnchorHandles[PointIndex];
		}
		else
		{
			ActiveAnchorHandle = nullptr;
		}
		HighlightHandle(ActiveAnchorHandle);
	}
	else
	{
		ActiveAnchorHandle = nullptr;
	}
	ActiveAnchorIndex = PointIndex;
}


void AEditableConnectionActor::HighlightHandle(AActor* PointHandle)
{
	bool result = AnchorHandles.Find(PointHandle) >= 0;
	if (PointHandle != HighlightedHandle)
	{
		if (HighlightedHandle)
		{
			UUEUtil::SetActorActive(HighlightedHandle, false, true, false, false);
		}
		HighlightedHandle = PointHandle;
	}
	if (HighlightedHandle)
	{
		UUEUtil::SetActorActive(HighlightedHandle, true, true, false, false);
	}
	////if (AnchorHandleTemplate && HighlightedHandle)
	////{
	////	HighlightedHandle->SetActorScale3D(AnchorHandleTemplate->GetActorScale3D());
	////}
	////if (AnchorHandleTemplate && handle)
	////{
	////	handle->SetActorScale3D(AnchorHandleTemplate->GetActorScale3D()*1.1f);
	////	HighlightedHandle = handle;
	////}
}


int32 AEditableConnectionActor::InsertSectionBefore()
{
	int32 index = ActiveAnchorIndex;
	InsertSection(GetAnchorLocalPosition(ActiveAnchorIndex), index);
	SelectAnchor(index);
	return index;
}


int32 AEditableConnectionActor::InsertSectionAfter()
{
	int32 index = ActiveAnchorIndex + 1;
	InsertSection(GetAnchorLocalPosition(ActiveAnchorIndex), index);
	SelectAnchor(index);
	return index;
}


void AEditableConnectionActor::RebuildConnectionSections()
{
	DestroyConnectionSections();
	BuildConnectionSections();
}


void AEditableConnectionActor::AddSection()
{
	int32 index = 0;
	if (ConnectionPath.Num() > 0)
	{
		index = ConnectionPath.Num() - 1;
	}
	FVector location = FVector::ZeroVector;
	if (ActiveAnchorIndex >= 0)
	{
		location = GetAnchorLocalPosition(ActiveAnchorIndex);
	}
	if (ConnectionPath.Num() == 0)
	{
		InsertSection(location, index);
		index = 1;
	}
	InsertSection(location, index);
	ActiveAnchorIndex = ConnectionPath.Num() - 1;
}


void AEditableConnectionActor::DeleteAnchor()
{
	if (!IsValidAnchorIndex(ActiveAnchorIndex))
	{
		return;
	}
	DeleteSection(ActiveAnchorIndex);
	ConnectionPath.RemoveAt(ActiveAnchorIndex);
	if (!IsValidAnchorIndex(ActiveAnchorIndex))
	{
		ActiveAnchorIndex = -1;
	}
	AnchorHandles.Last()->Destroy();
	AnchorHandles.RemoveAt(AnchorHandles.Num() - 1);
	UpdateSections();
	SelectAnchor(ActiveAnchorIndex);
}


void AEditableConnectionActor::ClearAll()
{
	DestroyConnectionSections();
	ConnectionPath.Reset();
}


void AEditableConnectionActor::UpdateSections()
{
	int32 numSections = ConnectionSections.Num();
	if (numSections < 1)
	{
		UUScenUtil::Log(Logger::VERBOSE, GetName() + TEXT(" - Empty connection path"), true, true, "INVALID_CONNECTION_PATH");
		return;
	}
	if (ConnectionPath.Num() < numSections + 1)
	{
		UUScenUtil::Log(Logger::ERROR, GetName() + TEXT(" - Not enough connection path points"), true, true, "INVALID_CONNECTION_PATH");
		return;
	}
	if (IsEditing())
	{
		FVector handleLocation = ComputeAnchorLocation(GetHandleLocalPosition(), bChangeSnapToAxis ? !bSnapToAxis : bSnapToAxis);
		SetAnchorLocalPosition(ActiveAnchorIndex, handleLocation);
	}
	UpdateSectionsImpl();
	UpdateAnchorHandles();
	if (ActiveAnchorHandle)
	{
		SelectAnchor(ActiveAnchorIndex);
	}
}


void AEditableConnectionActor::UpdateOnlyIfNeeded()
{
	if (!bUpdateNeeded)
	{
		return;
	}
	bool editing = IsEditing();
	if (editing)
	{
		FVector handlePos = GetHandleLocation();
		handlePos = GetHandleLocation();
		editing = !handlePos.Equals(PrevHandlePos);
		PrevHandlePos = handlePos;
	}
	bool moving = false;
	if (bAnchoredToWorld)
	{
		FVector pos = GetActorLocation();
		moving = !pos.Equals(PrevPos);
		PrevPos = pos;
	}
	bool anchorsChanged = false;
	if (bAnchoredToElements)
	{
		for (int i = 0; i < ConnectionPath.Num(); i++)
		{
			if (ConnectionPath[i].Component != nullptr)
			{
				if (!::IsValid(ConnectionPath[i].Component))
				{
					// if the connected element was deleted, set the anchor to the world
					LastAnchorPositions.Remove(ConnectionPath[i].Component);
					ConnectionPath[i].Offset = AnchorHandles[i]->GetActorLocation();
					ConnectionPath[i].Component = nullptr;
					anchorsChanged = true;
				}
				else if (ConnectionPath[i].Component != RootComponent)
				{
					if (!ConnectionPath[i].Component->GetComponentLocation().Equals(LastAnchorPositions[ConnectionPath[i].Component]))
					{
						LastAnchorPositions[ConnectionPath[i].Component] = ConnectionPath[i].Component->GetComponentLocation();
						anchorsChanged = true;
					}
				}
			}
		}
	}
	bUpdateNeeded = editing || moving || anchorsChanged;
	if (bUpdateNeeded)
	{
		UpdateSections();
	}
	bUpdateNeeded = false;
}


bool AEditableConnectionActor::IsEditing()
{
	return (HighlightedHandle && IsValidAnchorIndex(ActiveAnchorIndex));
}


void AEditableConnectionActor::ToggleActiveHandle(bool bShown)
{
	for (int i = 0; i < AnchorHandles.Num(); i++)
	{
		UUEUtil::SetActorActive(AnchorHandles[i], bShown && i == ActiveAnchorIndex, true, false, false);
	}
}


void AEditableConnectionActor::SetHandlesEnabled(bool bEnabled)
{
	for (int i = 0; i < AnchorHandles.Num(); i++)
	{
		UUEUtil::SetActorActive(AnchorHandles[i], bEnabled, !bEnabled, true, false);
	}
}


bool AEditableConnectionActor::SelectHandle(AActor* PointHandle)
{
	int32 index = -1;
	bool result = AnchorHandles.Find(PointHandle, index);
	SelectAnchor(index);
	return result;
}


void AEditableConnectionActor::PreviousAnchor()
{
	if (ActiveAnchorIndex < 0)
	{
		ActiveAnchorIndex = ConnectionPath.Num() - 1;
	}
	else
	{
		ActiveAnchorIndex--;
	}
	if (IsValidAnchorIndex(ActiveAnchorIndex))
	{
		SelectAnchor(ActiveAnchorIndex);
	}
	else
	{
		ActiveAnchorIndex = -1;
	}
}


void AEditableConnectionActor::NextAnchor()
{
	ActiveAnchorIndex++;
	if (IsValidAnchorIndex(ActiveAnchorIndex))
	{
		SelectAnchor(ActiveAnchorIndex);
	}
	else
	{
		ActiveAnchorIndex = -1;
	}
}


void AEditableConnectionActor::OnBeginMouseOverHandle(UPrimitiveComponent* TouchedComponent)
{
	if (MovingHandle)
	{
		return;
	}
	HighlightHandle(TouchedComponent->GetOwner());
	//UUEUtil::Log.LogMessage({Logger::LOG, "Mouse over " + TouchedComponent->GetName()});
}


void AEditableConnectionActor::OnEndMouseOverHandle(UPrimitiveComponent* TouchedComponent)
{
	if (MovingHandle)
	{
		return;
	}
	HighlightHandle(GetActiveAnchorHandle());
	//UUEUtil::Log.LogMessage({Logger::LOG, "Mouse off " + TouchedComponent->GetName()});
}


void AEditableConnectionActor::OnMouseClickedHandle(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	ClickedComponent = ClickedComp;
}


void AEditableConnectionActor::BeginPlay()
{
	Super::BeginPlay();

	CheckSectionTemplate();

	//ConnectionComponent = CastChecked<UConnectionElementComponent>( GetComponentByClass(UConnectionElementComponent::StaticClass()) );
	if (::IsValid(AnchorHandleTemplate))
	{
		UUEUtil::SetActorActive(AnchorHandleTemplate, false);
	}

	RebuildConnectionSections();
	if (ActiveAnchorHandle)
	{
		SelectAnchor(ActiveAnchorIndex);
	}
	SelectAnchor(-1);
	StartUpdateTimer();
}


void AEditableConnectionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ConnectionPath.Reset();
	ClearAll();
}


void AEditableConnectionActor::OnHandleOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// TODO: here the interaction with VR pointers can be managed

	if (IsEditing()
		&& OverlappedComp->GetOwner() == ActiveAnchorHandle)
	{
		UElementDataComponent* elementComponent = UUScenUtil::GetElementData(OtherActor);
		if (elementComponent)
		{
			if (CandidateSocketAnchor.Component)
			{
				HighlightCandidateAnchor(false);
			}
			if (OtherComp->ComponentHasTag(UUScenUtil::SocketTag))
			{
				const FString socketId = elementComponent->FindSocketIdByComponent(OtherComp);
				if (!socketId.IsEmpty())
				{
					CandidateSocketAnchor.SocketId = *socketId;
					CandidateSocketAnchor.ElementId = elementComponent->Identifier;
					CandidateSocketAnchor.Component = OtherComp;
					HighlightCandidateAnchor(true);
					UUEUtil::TogglePrimitiveOutline(OverlappedComp, true, EEditFlags::Connecting);
					bHandleOverlapped = true;
				}
			}
			else
			{
				CandidateSocketAnchor.SocketId.Reset();
				CandidateSocketAnchor.ElementId = elementComponent->Identifier;
				CandidateSocketAnchor.Component = OtherActor->GetRootComponent();
				HighlightCandidateAnchor(true);
			}
			PrintAnchorInfo();
		}
	}
}


void AEditableConnectionActor::OnHandleOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsEditing() && OverlappedComp->GetOwner() == ActiveAnchorHandle && OtherComp == CandidateSocketAnchor.Component)
	{
		bHandleOverlapped = false;
		//	UUEUtil::ToggleActorOutline(OtherActor, false, EEditFlags::Connecting);
		//	UUEUtil::ToggleActorOutline(ActiveAnchorHandle, false, EEditFlags::Connecting);
		//	CandidateSocketAnchor = FConnectionAnchor();
	}
}


bool AEditableConnectionActor::CheckSectionTemplate()
{
	if (!::IsValid(SectionBlueprint) && !::IsValid(SectionTemplate))
	{
		UUScenUtil::Log(Logger::ERROR, GetName() + " - The pipe template is not defined.");
		return false;
	}
	//if (::IsValid(SectionTemplate))
	//{
	//	UPrimitiveComponent* sectionTemplatePrim = CastChecked<UPrimitiveComponent>(SectionTemplate->GetComponentByClass(UPrimitiveComponent::StaticClass()), ECastCheckedType::NullAllowed);
	//	if (!sectionTemplatePrim)
	//	{
	//		UUScenUtil::Log(Logger::ERROR, GetName() + " - The section template has no primitive component.");
	//		return false;
	//	}
	//}
	return true;
	/*
	if (!::IsValid(SectionBlueprint) && ::IsValid(SectionTemplate) && SectionTemplate->GetAttachParentActor()!=this)
		{
			//FString sectionName = GetName() + "_S" + FString::FromInt(ConnectionSections.Num());
			//SectionTemplate = UUEUtil::CreateCloneOfActorAndPlace(SectionTemplate, sectionName, FVector::ZeroVector, FRotator::ZeroRotator, this);
			UUEUtil::SetActorActive(SectionTemplate, false);
			UUEUtil::SetActorVisible(SectionTemplate, false);
		}
		if (::IsValid(SectionBlueprint) && !::IsValid(SectionTemplate))
		{
			SectionTemplate = GetWorld()->SpawnActor(SectionBlueprint);
			UPrimitiveComponent* sectionTemplatePrim = CastChecked<UPrimitiveComponent>(SectionTemplate->GetComponentByClass(UPrimitiveComponent::StaticClass()), ECastCheckedType::NullAllowed);
			if (!sectionTemplatePrim)
			{
				UUScenUtil::Log(Logger::ERROR, GetName() + " - The section blueprint has no primitive component.", true, true, "SECTION_BP_MISSING_PRIM");
				SectionTemplate->Destroy();
				SectionBlueprint = nullptr;
				return false;
			}
			SectionTemplate->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
			sectionTemplatePrim->bVisible = false;
			UUEUtil::SetActorActive(SectionTemplate, false);
		}
		if (!::IsValid(SectionTemplate))
		{
			UUScenUtil::Log(Logger::ERROR, GetName() + " - The section template is not defined.");
			return false;
		}
		if (::IsValid(SectionTemplate))
		{
			UPrimitiveComponent* sectionTemplatePrim = CastChecked<UPrimitiveComponent>(SectionTemplate->GetComponentByClass(UPrimitiveComponent::StaticClass()), ECastCheckedType::NullAllowed);
			if (!sectionTemplatePrim)
			{
				UUScenUtil::Log(Logger::ERROR, GetName() + " - The section template has no primitive component.");
				return false;
			}
		}
		return true;
		*/
}


void AEditableConnectionActor::DestroyConnectionSections()
{
	DestroySectionsImpl();
	for (AActor* anchorHandle : AnchorHandles)
	{
		if (anchorHandle)
		{
			anchorHandle->Destroy();
		}
	}
	AnchorHandles.Reset();
}


void AEditableConnectionActor::BuildConnectionSections()
{
	if (!UUEUtil::GetActorVisible(this))
	{
		//UUEUtil::SetActorActive(this, false, true);
		DestroyConnectionSections();
		return;
	}
	bool selected = ActiveAnchorIndex >= 0;


	BuildSectionsImpl();
	ToggleActiveHandle(selected);
	SelectAnchor(ActiveAnchorIndex);
}



void AEditableConnectionActor::BuildSectionsImpl()
{
	int32 numAnchors = ConnectionPath.Num();

	int32 numSections = ConnectionSections.Num();
	// if the number of sections does not match the number of anchors 
	if (numSections != numAnchors - 1)
	{
		// rebuild the sections
		DestroyConnectionSections();
		for (int32 i = 0; i < numAnchors - 1; i++)
		{
			CreateSectionActor();
		}
	}

	UpdateSections();
}


void AEditableConnectionActor::DestroySectionsImpl()
{
	UUEUtil::DestroyActorsArray(ConnectionSections);
}


void AEditableConnectionActor::InsertSection(FVector location, int32 index)
{
	AActor* spawnedSection = CreateSectionActor();
	if (spawnedSection)
	{
		FConnectionAnchor anchor;
		anchor.Offset = location;
		anchor.Component = RootComponent;
		if (ConnectionPath.Num() < 2 && index > 0)
		{
			ConnectionPath.Insert(anchor, index - 1);
		}
		ConnectionPath.Insert(anchor, index);
		ToggleActiveHandle(UUEUtil::GetActorVisible(AnchorHandles[0]));
		UpdateSectionsImpl();
		UpdateAnchorHandles();
	}
}


void AEditableConnectionActor::DeleteSection(int32 pointIndex)
{
	if (pointIndex>=0 && pointIndex < ConnectionSections.Num())
	{
		AActor* connectionSection = ConnectionSections[pointIndex];
		ConnectionSections.RemoveAt(pointIndex);
		connectionSection->Destroy();
	}
}


AActor* AEditableConnectionActor::CreateSectionActor()
{
	AActor* spawnedSection = nullptr;
	UWorld* world = GetWorld();
	if (::IsValid(SectionBlueprint))
	{
		spawnedSection = world->SpawnActor(SectionBlueprint);
		UPrimitiveComponent* sectionTemplatePrim = CastChecked<UPrimitiveComponent>(spawnedSection->GetComponentByClass(UPrimitiveComponent::StaticClass()), ECastCheckedType::NullAllowed);
		if (!sectionTemplatePrim)
		{
			UUScenUtil::Log(Logger::ERROR, GetName() + " - The section blueprint has no primitive component.", true, true, "SECTION_BP_MISSING_PRIM");
			spawnedSection->Destroy();
			SectionBlueprint = nullptr;
			return nullptr;
		}
		spawnedSection->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		spawnedSection->SetActorRelativeLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
		spawnedSection->SetActorRelativeRotation(FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (!::IsValid(spawnedSection))
	{
		if (!::IsValid(SectionTemplate))
		{
			UUScenUtil::Log(Logger::ERROR, GetName() + " - The pipe template is not defined.");
			return nullptr;
		}
		//sectionTemplatePrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		FString sectionName = GetName() + "_s" + FString::FromInt(ConnectionSections.Num());
		spawnedSection = UUEUtil::CreateCloneOfActorAndPlace(SectionTemplate, sectionName, FVector::ZeroVector, FRotator::ZeroRotator, this);
	}
	TArray<UPrimitiveComponent*> sectionPrimitives;
	UUEUtil::GetPrimitiveComponents(this, sectionPrimitives);
	for (UPrimitiveComponent* sectionPrim : sectionPrimitives)
	{
		sectionPrim->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		//sectionPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//sectionPrim->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		sectionPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		sectionPrim->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		sectionPrim->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
#if ENGINE_MINOR_VERSION > 16
		sectionPrim->SetGenerateOverlapEvents(true);
#else
		sectionPrim->bGenerateOverlapEvents = true;
#endif
	}
	ConnectionSections.Add(spawnedSection);
	if (!UUEUtil::GetActorVisible(spawnedSection))
	{
		UUEUtil::SetActorActive(spawnedSection, true);
		UUEUtil::SetActorVisible(spawnedSection, true);
	}

	CreateAnchorHandleActor();
	return spawnedSection;
}


AActor* AEditableConnectionActor::CreateAnchorHandleActor()
{
	if (::IsValid(AnchorHandleTemplate))
	{
		AActor* newAnchorHandle = nullptr;
		FString handleNamePref = GetName() + "_h";
		if (AnchorHandles.Num() == 0)
		{
			newAnchorHandle = UUEUtil::CreateCloneOfActor(AnchorHandleTemplate, handleNamePref + "0", this);
			UUEUtil::SetActorActive(newAnchorHandle, true);
			UUEUtil::SetActorVisible(newAnchorHandle, true);
			SetupAnchorHandleActor(newAnchorHandle);
			AnchorHandles.Add(newAnchorHandle);
		}
		FString handleName = handleNamePref + FString::FromInt(AnchorHandles.Num());
		newAnchorHandle = UUEUtil::CreateCloneOfActor(AnchorHandleTemplate, handleName, this);
		UUEUtil::SetActorActive(newAnchorHandle, true);
		SetupAnchorHandleActor(newAnchorHandle);
		AnchorHandles.Add(newAnchorHandle);
		return newAnchorHandle;
	}
	else
	{
		UUScenUtil::Log(Logger::WARNING, "Pipe PointHandle not defined.", true, true, "MissingPipeHandle");
	}
	return nullptr;
}


void AEditableConnectionActor::SetupAnchorHandleActor(AActor* actor)
{
	actor->Tags.AddUnique("Selectable");
	actor->Tags.AddUnique("Movable");

	if (bMouseInteractionEnabled)
	{
		UStaticMeshComponent* meshComponent = CastChecked<UStaticMeshComponent>(actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		meshComponent->OnBeginCursorOver.AddDynamic(this, &AEditableConnectionActor::OnBeginMouseOverHandle);
		meshComponent->OnEndCursorOver.AddDynamic(this, &AEditableConnectionActor::OnEndMouseOverHandle);
		meshComponent->OnClicked.AddDynamic(this, &AEditableConnectionActor::OnMouseClickedHandle);
		meshComponent->ComponentTags.AddUnique(UUScenUtil::GizmoTag);
		if (bSocketConnectionEnabled)
		{
			if (!meshComponent->OnComponentHit.IsBound())
			{
				meshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
				meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				//meshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
				//meshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				meshComponent->OnComponentBeginOverlap.AddDynamic(this, &AEditableConnectionActor::OnHandleOverlapBegin);
				meshComponent->OnComponentEndOverlap.AddDynamic(this, &AEditableConnectionActor::OnHandleOverlapEnd);
#if ENGINE_MINOR_VERSION > 16
				meshComponent->SetGenerateOverlapEvents(true);
#else
				meshComponent->bGenerateOverlapEvents = true;
#endif
			}
		}
	}
}


void AEditableConnectionActor::UpdateAnchorHandles()
{
	bAnchoredToWorld = false;
	bAnchoredToElements = false;
	LastAnchorPositions.Reset();
	for (int i = 0; i < AnchorHandles.Num() && i < ConnectionPath.Num(); i++)
	{
		if (ConnectionPath[i].Component == nullptr)
		{
			bAnchoredToWorld = true;
		}
		else if (ConnectionPath[i].Component != RootComponent)
		{
			bAnchoredToElements = true;
			LastAnchorPositions.Add(ConnectionPath[i].Component, ConnectionPath[i].Component->GetComponentLocation());
		}
		// TODO: optimize this transformation
		FVector newLocation = GetActorTransform().TransformPosition(GetAnchorLocalPosition(i));
		if (AnchorHandles[i] != MovingHandle)
		{
			AnchorHandles[i]->SetActorLocation(newLocation, false, nullptr,
				// TODO: check correct UE4 version
#if ENGINE_MINOR_VERSION > 19
				ETeleportType::ResetPhysics
#else
				ETeleportType::TeleportPhysics
#endif
				);
		}
	}
}

FVector AEditableConnectionActor::ComputeAnchorLocation(FVector HandleLocalPosition, bool bSnapToAxisFlag)
{
	if (!bSnapToAxisFlag)
	{
		return HandleLocalPosition;
	}
	FVector handleLocation = HandleLocalPosition;
	// if the handle is moved parallel to the next section,
	// the also the following point is moved together along its section axis,
	// otherwise the handle can be moved along the axis closer
	// to the maximum distance component 

	if (ConnectionPath.Num() > 2 && ActiveAnchorIndex == 0)
	{
		FVector nextDir;
		bool projected = false;
		FVector pos0 = handleLocation;// GetAnchorLocalPosition(0);
		FVector pos1 = GetAnchorLocalPosition(1);
		FVector pos2 = GetAnchorLocalPosition(2);
		FVector vec01 = pos1 - pos0;
		FVector vec12 = pos2 - pos1;
		FVector currDir = vec01;
		currDir.Normalize();
		nextDir = vec12;
		nextDir.Normalize();
		projected = fabs(FVector::DotProduct(currDir, nextDir)) < 0.1f;
		if (projected)
		{
			FVector handleVec = handleLocation - pos1;
			FVector offset = handleVec.ProjectOnTo(vec12);
			handleLocation = pos0 + offset;
			SetAnchorLocalPosition(1, pos1 + offset);
			return handleLocation;
		}
	}

	FVector refLocation = GetAnchorLocalPosition(ActiveAnchorIndex);
	if (ActiveAnchorIndex > 0)
	{
		refLocation = GetAnchorLocalPosition(ActiveAnchorIndex - 1);
	}
	else if (ConnectionPath.Num() > 1)
	{
		refLocation = GetAnchorLocalPosition(1);
	}
	FVector handleOffset = handleLocation - refLocation;
	FVector handleAbsOffset(fabs(handleOffset.X), fabs(handleOffset.Y), fabs(handleOffset.Z));
	// change only the coordinate with the maximum offset
	float maxCoord = std::max(handleAbsOffset.X, handleAbsOffset.Y);
	maxCoord = std::max(maxCoord, handleAbsOffset.Z);
	if (maxCoord > 0)
	{
		bool maxIsX = handleAbsOffset.X > handleAbsOffset.Y && handleAbsOffset.X > handleAbsOffset.Z;
		bool maxIsY = handleAbsOffset.Y > handleAbsOffset.X && handleAbsOffset.Y > handleAbsOffset.Z;
		bool maxIsZ = handleAbsOffset.Z > handleAbsOffset.X && handleAbsOffset.Z > handleAbsOffset.Y;

		if (maxIsY || maxIsZ)
		{
			handleLocation.X = refLocation.X;
		}
		if (maxIsX || maxIsZ)
		{
			handleLocation.Y = refLocation.Y;
		}
		if (maxIsX || maxIsY)
		{
			handleLocation.Z = refLocation.Z;
		}
		// if it is a point in the middle adjust also the next point
		if (ActiveAnchorIndex > 0 && ActiveAnchorIndex < ConnectionPath.Num() - 1)
		{
			FVector dragPos = GetAnchorLocalPosition(ActiveAnchorIndex + 1);
			if (maxIsX)
			{
				dragPos.X = handleLocation.X;
			}
			if (maxIsY)
			{
				dragPos.Y = handleLocation.Y;
			}
			if (maxIsZ)
			{
				dragPos.Z = handleLocation.Z;
			}
			SetAnchorLocalPosition(ActiveAnchorIndex + 1, dragPos);
		}
		// if it is the first point adjust also the next point?
		if (ActiveAnchorIndex == 0 && ConnectionPath.Num() > 1)
		{
			FVector dragPos = GetAnchorLocalPosition(ActiveAnchorIndex + 1);
			if (maxIsX)
			{
				dragPos.Y = handleLocation.Y;
				dragPos.Z = handleLocation.Z;
			}
			if (maxIsY)
			{
				dragPos.X = handleLocation.X;
				dragPos.Z = handleLocation.Z;
			}
			if (maxIsZ)
			{
				dragPos.X = handleLocation.X;
				dragPos.Y = handleLocation.Y;
			}
			SetAnchorLocalPosition(ActiveAnchorIndex + 1, dragPos);
			//...?
		}
	}
	return handleLocation;
}


void AEditableConnectionActor::StartUpdateTimer()
{
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEditableConnectionActor::OnUpdateTimer, UpdateDelay, true, UpdateDelay);
}


void AEditableConnectionActor::OnUpdateTimer()
{
	bUpdateNeeded = true;
}


FVector AEditableConnectionActor::GetHandleLocalPosition()
{
	FVector handleLocation = FVector::ZeroVector;
	if (::IsValid(ActiveAnchorHandle))
	{
		handleLocation = ActiveAnchorHandle->GetActorLocation();
	}
	FVector handleLocalPos = GetTransform().InverseTransformPosition(handleLocation);

	return handleLocalPos;
}


FVector AEditableConnectionActor::GetHandleLocation()
{
	FVector handleLocation = FVector::ZeroVector;
	if (::IsValid(ActiveAnchorHandle))
	{
		handleLocation = ActiveAnchorHandle->GetActorLocation();
	}

	return handleLocation;
}


FVector AEditableConnectionActor::GetAnchorLocalPosition(int AnchorIndex)
{
	if (!IsValidAnchorIndex(AnchorIndex))
	{
		return FVector::ZeroVector;
	}
	const FConnectionAnchor& anchor = ConnectionPath[AnchorIndex];
	if (anchor.Component == RootComponent)
	{
		return anchor.Offset;
	}
	return UUScenUtil::GetAnchorLocalPosition(anchor, RootComponent);
}


void AEditableConnectionActor::SetAnchorLocalPosition(int AnchorIndex, const FVector& Position)
{
	if (!IsValidAnchorIndex(AnchorIndex))
	{
		return;
	}
	FConnectionAnchor& anchor = ConnectionPath[AnchorIndex];
	if (anchor.Component == RootComponent)
	{
		anchor.Offset = Position;
		return;
	}

	UUScenUtil::SetAnchorRelativePosition(anchor, Position, RootComponent);
}


bool AEditableConnectionActor::DragHandle(AActor* handle)
{
	if (SelectHandle(handle))
	{
		PickedHandleDist = UUEUtil::GetDistanceFromEyesViewPoint(handle);
		MovingHandle = handle;
		return true;
	}
	return false;
}


void AEditableConnectionActor::UpdateMovingHandle()
{
	if (MovingHandle)
	{
		MovingHandle->SetActorLocation(GetAnchorLocalPosition(ActiveAnchorIndex));
	}
}


void AEditableConnectionActor::UpdateMouseInteraction()
{
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	float mouseWheelUp = playerController->WasInputKeyJustPressed(EKeys::MouseScrollUp);//EKeys::Z);
	float mouseWheelDown = playerController->WasInputKeyJustPressed(EKeys::MouseScrollDown);//(EKeys::X);
	bool mouseWheel = mouseWheelUp != 0.0f || mouseWheelDown != 0.0f;
	if (mouseWheel)
	{
		PickedHandleDist += mouseWheelUp ? 15 : -15;
	}
	UUEUtil::MouseDragActor(MovingHandle, PickedHandleDist);
}


void AEditableConnectionActor::UpdateVrInteraction()
{
	// TODO: VR interaction to be reviewed and tested

	AActor* handle = nullptr;
	// Find the highlighetd handle
	for (int i = 0; i < AnchorHandles.Num(); i++)
	{
		if (CastChecked<UPrimitiveComponent>(AnchorHandles[i]->GetComponentByClass(UPrimitiveComponent::StaticClass()))->bRenderCustomDepth)
		{
			if (SelectHandle(AnchorHandles[i]))
			{
				handle = AnchorHandles[i];
			}
			break;
		}
	}
	if (MovingHandle && MovingHandle != handle)
	{
		MovingHandle->SetActorLocation(GetAnchorLocalPosition(ActiveAnchorIndex));
		if (handle == nullptr)
		{
			HighlightHandle(nullptr);
		}
	}
	if (handle && MovingHandle != handle)
	{
		MovingHandle = handle;
		SavedConnectionPath = ConnectionPath;
	}
}


void AEditableConnectionActor::OnEditConfirm()
{
	//MovingHandle->SetActorLocation(ConnectionPath[ActiveAnchorIndex]);
	ConnectionPath[ActiveAnchorIndex] = CandidateSocketAnchor;
	if (CandidateSocketAnchor.Component)
	{
		HighlightCandidateAnchor(false);
		if (MovingHandle)
		{
			UUEUtil::ToggleActorOutline(MovingHandle, false, EEditFlags::Connecting);
			if (bHandleOverlapped && !CandidateSocketAnchor.SocketId.IsEmpty())
			{
				USceneComponent* socketComponent = CandidateSocketAnchor.Component;
				MovingHandle->SetActorLocation(socketComponent->GetComponentLocation());
				//TODO: reorient also the pipe section according to the socket orientation?
			}
		}
	}
	CandidateSocketAnchor = FConnectionAnchor();
	PrintAnchorInfo();

	UpdateSections();
	SetModified(true);
	MovingHandle = nullptr;
	bHandleOverlapped = false;
}


void AEditableConnectionActor::OnEditAddBefore()
{
	CandidateSocketAnchor = FConnectionAnchor();
	UpdateMovingHandle();
	SavedConnectionPath = ConnectionPath;
	int32 index = InsertSectionBefore();
	SelectAnchor(index);
	DragHandle(ActiveAnchorHandle);
	UpdateSections();
}


void AEditableConnectionActor::OnEditAddAfter()
{
	CandidateSocketAnchor = FConnectionAnchor();
	UpdateMovingHandle();
	//ActiveAnchorHandle = AnchorHandles.Last();
	//int32 index = AnchorHandles.Num();
	//if (IsValidAnchorIndex(ActiveAnchorIndex))
	//{
	//	index = ActiveAnchorIndex + 1;
	//}
	SavedConnectionPath = ConnectionPath;
	//	AddSection();
	int32 index = InsertSectionAfter();

	SelectAnchor(index);
	DragHandle(AnchorHandles[index]);
	UpdateSections();
}


void AEditableConnectionActor::OnEditCancel()
{
	ConnectionPath = SavedConnectionPath;
	int32 index = ActiveAnchorIndex < ConnectionPath.Num() ? ActiveAnchorIndex : ConnectionPath.Num() - 1;
	if (index >= 0)
	{
		// TODO: consider ElementId and AnchorId
		MovingHandle->SetActorLocation(ConnectionPath[index].Offset);
	}
	if (bMouseInteractionEnabled)
	{
		SelectAnchor(-1);
	}
	else
	{
		CastChecked<UPrimitiveComponent>(MovingHandle->GetComponentByClass(UPrimitiveComponent::StaticClass()))->bRenderCustomDepth = false;
		HighlightHandle(nullptr);
	}
	MovingHandle = nullptr;
	bHandleOverlapped = false;
	if (CandidateSocketAnchor.Component)
	{
		UUEUtil::ToggleActorOutline(CandidateSocketAnchor.Component->GetOwner(), false, EEditFlags::Connecting);
	}
	CandidateSocketAnchor = FConnectionAnchor();
	BuildConnectionSections();
}


void AEditableConnectionActor::PrintAnchorInfo()
{
	if (IsValidAnchorIndex(ActiveAnchorIndex))
	{
		FString msg = FString::Printf(TEXT("Anchor %d"), ActiveAnchorIndex);
		if (!ConnectionPath[ActiveAnchorIndex].ElementId.IsEmpty())
		{
			msg += FString::Printf(TEXT("-%s.%s"), *(ConnectionPath[ActiveAnchorIndex].ElementId), *(ConnectionPath[ActiveAnchorIndex].SocketId));
		}
		UUScenUtil::Log(Logger::DEBUG, msg, false, true, TEXT("PipelineAnchorInfo"));
	}
}


void AEditableConnectionActor::SetModified(bool bModified)
{
	UActorComponent* actorComponent = GetComponentByClass(UEntityDataComponent::StaticClass());
	if (actorComponent)
	{
		UEntityDataComponent* entityComponent = CastChecked<UEntityDataComponent>(actorComponent);
		entityComponent->bModified = bModified;
	}
}


void AEditableConnectionActor::HighlightCandidateAnchor(bool bHighlighted)
{
	if (CandidateSocketAnchor.Component)
	{
		USceneComponent* socketComponent = CandidateSocketAnchor.Component;
		if (!CandidateSocketAnchor.SocketId.IsEmpty())
		{
			UPrimitiveComponent* primComp = Cast<UPrimitiveComponent>(socketComponent);
			UElementDataComponent* elemComp = UUScenUtil::GetElementData(primComp->GetOwner());
			elemComp->HighlightSocket(primComp, bHighlighted, EEditFlags::Connecting);
		}
		else
		{
			UUEUtil::ToggleActorOutline(socketComponent->GetOwner(), bHighlighted, EEditFlags::Connecting);
		}
	}
}
