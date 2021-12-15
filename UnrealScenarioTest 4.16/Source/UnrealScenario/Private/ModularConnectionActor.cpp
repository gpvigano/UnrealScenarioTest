// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "ModularConnectionActor.h"

#include "UEUtil.h"
#include "UScenUtil.h"
#include "ModularActor.h"

#include "Engine/World.h" // GetWorld()


AModularConnectionActor::AModularConnectionActor()
{
	bSnapToAxis = true;
}

#if WITH_EDITOR

void AModularConnectionActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FString propName;
	if (PropertyChangedEvent.Property)
	{
		propName = PropertyChangedEvent.Property->GetName();
		if (propName == TEXT("SectionTemplate"))
		{
			if (SectionTemplate && !SectionTemplate->IsA(AModularActor::StaticClass()))
			{
				UUScenUtil::Log(Logger::ERROR, TEXT("Section Template must be a Modular Actor."));
				return;
			}
		}
		if (propName == TEXT("CornerModuleBlueprint"))
		{
			UpdateSections();
		}
		//UUScenUtil::Log(Logger::LOG, propName + TEXT(" changed."));
		if (propName == TEXT("X") || propName == TEXT("Y") || propName == TEXT("Z"))
		{
			UpdateSections();
		}

	}
}

#endif


void AModularConnectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


bool AModularConnectionActor::CheckSectionTemplate()
{
	if (!Super::CheckSectionTemplate())
	{
		return false;
	}
	AModularActor* modActor = Cast<AModularActor>(SectionTemplate);
	if (!modActor)
	{
		UUScenUtil::Log(Logger::ERROR, "The section template is not a Modular Actor.");
		DestroySectionsImpl();
		SectionTemplate = nullptr;
		SectionBlueprint = nullptr;
		return false;
	}
	return true;
}


void AModularConnectionActor::UpdateSectionsImpl()
{
	int32 numSections = ConnectionSections.Num();
	UStaticMeshComponent* currentMesh = nullptr;
	CornerFlags.Reset();

	if (ConnectionPath.Num() < 2)
	{
		return;
	}

	FVector currPos = GetAnchorLocalPosition(0);
	FVector prevPos = currPos;
	AModularActor* prevSectionActor = nullptr;
	int cornersCount = 0;
	for (int i = 0; i < numSections && i < ConnectionPath.Num() - 1; i++)
	{
		FVector nextPos = GetAnchorLocalPosition(i + 1);
		AModularActor* sectionActor = Cast<AModularActor>(ConnectionSections[i]);
		float offset = sectionActor->GetModuleLength() / 2.0f;

		bool isCorner = false;
		// if a corner is defined add it to this actor and adjust sections positions
		if (CornerModuleBlueprint && i > 0)
		{
			FVector prevDir = currPos - prevPos;
			prevDir.Normalize();
			FVector pivot = currPos + prevDir * offset;
			FVector currDir = nextPos - pivot;
			currDir.Normalize();
			float dotProd = FVector::DotProduct(currDir, prevDir);

			// add it if the angle is enough (with this arbitrary threshold, for now)
			if (dotProd < 0.75f)
			{
				isCorner = true;
				FActorSpawnParameters spawnParams;
				AActor* newModuleActor = GetOrAddCorner(cornersCount);
				cornersCount++;

				// TODO: exit axis for the corner module must be parametrized

				FRotator cornerRot = prevDir.ToOrientationRotator();
				FVector rotatedRight = cornerRot.RotateVector(FVector::RightVector);
				FVector rotatedUp = cornerRot.RotateVector(FVector::UpVector);
				FVector projCurrDir = FVector::VectorPlaneProject(currDir, prevDir);
				float rDotCurr = FVector::DotProduct(rotatedRight, projCurrDir);
				float roll = FMath::RadiansToDegrees(FMath::Acos(rDotCurr));
				float upDotCurr = FVector::DotProduct(rotatedUp, projCurrDir);

				// note: in a right-handed system this would be (upDotCurr < 0.0f)
				cornerRot.Roll = (upDotCurr > 0.0f) ? -roll : roll;
				newModuleActor->SetActorRelativeLocation(currPos, false, nullptr, ETeleportType::TeleportPhysics);
				newModuleActor->SetActorRelativeRotation(cornerRot, false, nullptr, ETeleportType::TeleportPhysics);

				// TODO: exit point for the corner module must be parametrized
				FVector outVec = FVector(offset, offset, 0.0f);
				outVec = cornerRot.RotateVector(outVec);
				currPos += outVec;
			}
		}
		CornerFlags.Add(isCorner);
		sectionActor->SetActorRelativeLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
		sectionActor->SetActorRelativeRotation(FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
		sectionActor->StartPosition = currPos;
		sectionActor->EndPosition = nextPos;
		sectionActor->UpdateModules();
		nextPos = sectionActor->GetModulesEndPosition();
		FVector dir = nextPos - currPos;
		float dist = dir.Size();
		dir.Normalize();
		prevPos = currPos;
		currPos = nextPos;
		prevSectionActor = sectionActor;
	}
	RemoveCornersFrom(cornersCount);
}


void AModularConnectionActor::DestroySectionsImpl()
{
	Super::DestroySectionsImpl();
	UUEUtil::DestroyActorsArray(CornerModules);
}


void AModularConnectionActor::OnEditConfirm()
{
	AActor* movingHandle = MovingHandle;
	Super::OnEditConfirm();

	bool pathDefined = ConnectionPath.Num() > 0;
	bool firstPoint = ActiveAnchorIndex == 0 && pathDefined;
	bool lastPoint = ActiveAnchorIndex == ConnectionPath.Num() - 1 && pathDefined;
	bool middlePoint = ActiveAnchorIndex > 0 && ActiveAnchorIndex < ConnectionPath.Num() - 1 && pathDefined;

	if ((middlePoint && CornerFlags[ActiveAnchorIndex])
		|| (lastPoint && CornerModuleBlueprint != nullptr))
	{
		AModularActor* sectionActor = Cast<AModularActor>(ConnectionSections[ActiveAnchorIndex - 1]);
		float offset = sectionActor->GetModuleLength() / 2.0f;
		FVector dir = sectionActor->GetModulesEndPosition() - sectionActor->StartPosition;
		dir.Normalize();
		FVector pos = sectionActor->GetModulesEndPosition() + offset*dir;
		SetAnchorLocalPosition(ActiveAnchorIndex, pos);
		movingHandle->SetActorLocation(pos);
	}
	else if (firstPoint)
	{
		AModularActor* sectionActor = Cast<AModularActor>(ConnectionSections[0]);
		FVector pos = sectionActor->StartPosition;
		SetAnchorLocalPosition(ActiveAnchorIndex, pos);
		movingHandle->SetActorLocation(pos);
	}
	else if (ActiveAnchorIndex - 1 < ConnectionSections.Num())
	{
		AModularActor* sectionActor = Cast<AModularActor>(ConnectionSections[ActiveAnchorIndex - 1]);
		FVector pos = sectionActor->GetModulesEndPosition();
		SetAnchorLocalPosition(ActiveAnchorIndex, pos);
		movingHandle->SetActorLocation(pos);
	}
}


AActor* AModularConnectionActor::GetOrAddCorner(int32 Index)
{
	while (Index >= CornerModules.Num())
	{
		AActor* newModuleActor = nullptr;
		newModuleActor = GetWorld()->SpawnActor(CornerModuleBlueprint);
		newModuleActor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		CornerModules.Add(newModuleActor);
	}
	return CornerModules[Index];
}


void AModularConnectionActor::RemoveCornersFrom(int32 Index)
{
	while (CornerModules.Num() > Index)
	{
		AActor* toBeDestroyed = CornerModules.Last();
		CornerModules.Remove(toBeDestroyed);
		toBeDestroyed->Destroy();
	}
}
