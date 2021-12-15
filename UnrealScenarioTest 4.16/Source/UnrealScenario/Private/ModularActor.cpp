// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "ModularActor.h"

#include "UEUtil.h"
#include "UScenUtil.h"

#include "Engine/World.h" // GetWorld()
#include "DrawDebugHelpers.h"

AModularActor::AModularActor()
{

}



#if WITH_EDITOR

void AModularActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FString propName;
	if (PropertyChangedEvent.Property)
	{
		propName = PropertyChangedEvent.Property->GetName();
		if (propName == TEXT("ModuleBlueprint"))
		{
			Initialize(true);
		}
		//UUScenUtil::Log(Logger::LOG, propName + TEXT(" changed."));
		if (propName == TEXT("X") || propName == TEXT("Y") || propName == TEXT("Z"))
		{
			UpdateModules();
			DrawDebugSolidBox(GetWorld(), GetActorTransform().TransformPosition(EndPosition), FVector(20, 20, 20), FColor::Blue);
		}

	}
}

#endif


void AModularActor::BeginPlay()
{
	Super::BeginPlay();
	Initialize(true);
	UpdateModules();
}


void AModularActor::UpdateModules()
{
	if (!Initialize(false))
	{
		if (!Initialize(true))
		{
			return;
		}
	}
	FVector targetVector = EndPosition - StartPosition;
	ModulesEndRotation = FQuat::FindBetween(FVector::ForwardVector, targetVector);
	ModulesEndPosition = EndPosition;

	// TODO: Implement all types of EModuleArrangement

	switch (ModuleArrangement)
	{
	case EModuleArrangement::Linear:
	{
		FVector dir = targetVector;
		dir.Normalize();

		int32 numModules = (int32)floorf(targetVector.Size() / ModuleLength);
		SetNumModules(numModules);
		FVector pos = StartPosition;
		for (int32 i = 0; i < numModules; i++)
		{
			FVector offset = pos + dir*ModuleOffset.X;
			pos += dir*ModuleLength;
			ModulesEndPosition = pos;
			FQuat rotQuat = FQuat::FindBetween(ModuleAxisVector, dir);
			Modules[i]->SetActorRelativeLocation(offset, false, nullptr, ETeleportType::TeleportPhysics);
			Modules[i]->SetActorRelativeRotation(rotQuat, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	break;
	case EModuleArrangement::Manhattan:
	{
		//FVector dir = UUEUtil::FindClosestAxisDirectionXY(targetVector);
		//FVector cornerPos = targetVector.ProjectOnTo(dir);
		FVector cornerPos = targetVector.ProjectOnTo(FVector::ForwardVector);
		FVector startPosToCorner = cornerPos - StartPosition;
		FVector cornerToEndPos = EndPosition - cornerPos;
		FVector dir = startPosToCorner;
		dir.Normalize();

		int32 modulesToCorner = (int32)roundf(startPosToCorner.Size() / ModuleLength);
		int32 modulesCornerToEnd = (int32)roundf(cornerToEndPos.Size() / ModuleLength);
		//int32 modulesToCorner = (int32)floorf(startPosToCorner.Size() / ModuleLength);
		//int32 modulesCornerToEnd = (int32)floorf(cornerToEndPos.Size() / ModuleLength);
		//float dist = UUEUtil::ManhattanDistanceXY(StartPosition, EndPosition);
		//float dist = startPosToCorner.Size() + cornerToEndPos.Size();
		//int32 numModules = (int32)ceilf(dist / ModuleLength);
		//int32 numModules = (int32)roundf(dist / ModuleLength);
		int32 numModules = modulesToCorner + modulesCornerToEnd;
		SetNumModules(numModules);

		FVector pos = StartPosition;

		bool changedDir = false;
		for (int32 i = 0; i < numModules; i++)
		{
			if (!changedDir && i >= modulesToCorner)
			{
				dir = cornerToEndPos;
				dir.Normalize();
				ModulesEndRotation = FQuat::FindBetween(FVector::ForwardVector, targetVector);
				changedDir = true;
				//pos += ModuleSideVector * FVector::DotProduct(ModuleSideVector, ModuleOffset);
			}
			FVector offset = pos + ModuleOffset;
			pos += dir*ModuleLength;
			ModulesEndPosition = pos;
			FQuat rotQuat = FQuat::FindBetween(ModuleAxisVector, dir);

			Modules[i]->SetActorRelativeLocation(offset, false, nullptr, ETeleportType::TeleportPhysics);
			Modules[i]->SetActorRelativeRotation(rotQuat, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	break;
	}
}


bool AModularActor::Initialize(bool bRebuild)
{
	if (!::IsValid(ModuleBlueprint))
	{
		DestroyModules();
		bInitialized = false;
		return false;
	}
	if (!bRebuild && bInitialized)
	{
		return true;
	}
	DestroyModules();
	AActor* newModuleActor = nullptr;
	newModuleActor = GetWorld()->SpawnActor(ModuleBlueprint);
	UPrimitiveComponent* moduleTemplatePrim = CastChecked<UPrimitiveComponent>(newModuleActor->GetComponentByClass(UPrimitiveComponent::StaticClass()), ECastCheckedType::NullAllowed);
	if (!moduleTemplatePrim)
	{
		UUScenUtil::Log(Logger::ERROR, GetName() + " - The section blueprint has no primitive component.", true, true, "SECTION_BP_MISSING_PRIM");
	}
	else
	{
		//FBoxSphereBounds bounds = moduleTemplatePrim->Bounds;
	}
	EAxis::Type axis = ModuleAxis;
	FBox bBox = newModuleActor->CalculateComponentsBoundingBoxInLocalSpace();
	FVector bBoxSize = bBox.GetSize();
	if (axis == EAxis::None)
	{
		if (bBoxSize.X > bBoxSize.Y && bBoxSize.X > bBoxSize.Z) axis = EAxis::X;
		else if (bBoxSize.Y > bBoxSize.X && bBoxSize.Y > bBoxSize.Z) axis = EAxis::Y;
		else axis = EAxis::Y;
	}

	switch (axis)
	{
	case EAxis::X:
		ModuleOffset.X = -bBox.Min.X;
		ModuleLength = bBoxSize.X;
		ModuleWidth = bBoxSize.Y;
		ModuleAxisVector = FVector::ForwardVector;
		ModuleSideVector = FVector::RightVector;
		break;
	case EAxis::Y:
		ModuleOffset.Y = -bBox.Min.Y;
		ModuleLength = bBoxSize.Y;
		ModuleWidth = bBoxSize.X;
		ModuleAxisVector = FVector::RightVector;
		ModuleSideVector = FVector::ForwardVector;
		break;
	case EAxis::Z:
		ModuleOffset.Z = -bBox.Min.Z;
		ModuleLength = bBoxSize.Z;
		ModuleWidth = bBoxSize.X;
		ModuleAxisVector = FVector::UpVector;
		ModuleSideVector = FVector::RightVector;
		break;
	}
	newModuleActor->Destroy();
	//Modules.Add(newModuleActor);
	bInitialized = true;

	return true;
}


AActor* AModularActor::CreateModule()
{
	AActor* newModuleActor = nullptr;
	newModuleActor = GetWorld()->SpawnActor(ModuleBlueprint);
	newModuleActor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	Modules.Add(newModuleActor);
	return newModuleActor;
}

void AModularActor::SetNumModules(int32 NumModules)
{
	int32 prevNumModules = Modules.Num();
	if (prevNumModules < NumModules)
	{
		for (int32 i = prevNumModules; i < NumModules; i++)
		{
			CreateModule();
		}
		NumModules = Modules.Num();
	}
	if (prevNumModules > NumModules)
	{
		for (int32 i = prevNumModules - 1; i >= NumModules; i--)
		{
			Modules[i]->Destroy();
		}
		if (Modules.Num() > NumModules)
		{
			Modules.RemoveAt(NumModules, Modules.Num() - NumModules, true);
		}
	}
}


void AModularActor::DestroyModules()
{
	UUEUtil::DestroyActorsArray(Modules);
}

