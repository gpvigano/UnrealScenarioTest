// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "PipelineActor.h"
#include "UEUtil.h"
#include "UScenUtil.h"
#include "ElementDataComponent.h"

#include "Components/SplineMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h" // GetWorld()
#include "Runtime/Launch/Resources/Version.h" // ENGINE_MINOR_VERSION

#include <algorithm>


APipelineActor::APipelineActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurveSmoothing = 1.0f;
}


void APipelineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool APipelineActor::CheckSectionTemplate()
{
	if (!Super::CheckSectionTemplate())
	{
		return false;
	}
	if (!::IsValid(SectionBlueprint))
	{
		if (!::IsValid(SectionTemplate))
		{
			UUScenUtil::Log(Logger::ERROR, "The pipe template is not defined.");
			return false;
		}
		USplineMeshComponent* pipeTemplateSplineMesh = CastChecked<USplineMeshComponent>(SectionTemplate->GetComponentByClass(USplineMeshComponent::StaticClass()), ECastCheckedType::NullAllowed);
		if (!pipeTemplateSplineMesh)
		{
			UUScenUtil::Log(Logger::ERROR, "The pipe template has no spline mesh component.");
			return false;
		}
	}
	return true;

/*
if (!Super::CheckSectionTemplate())
	{
		return false;
	}
	USplineMeshComponent* pipeTemplateSplineMesh = CastChecked<USplineMeshComponent>(SectionTemplate->GetComponentByClass(USplineMeshComponent::StaticClass()), ECastCheckedType::NullAllowed);
	if (!pipeTemplateSplineMesh)
	{
		UUScenUtil::Log(Logger::ERROR, "The pipe template has no spline mesh component.");
		return false;
	}
	return true;
	*/
}



void APipelineActor::UpdateSectionsImpl()
{
	int32 numSections = ConnectionSections.Num();
	USplineMeshComponent* prevSplineMesh = nullptr;
	USplineMeshComponent* currentSplineMesh = nullptr;
	for (int i = 0; i < numSections && i < ConnectionPath.Num() - 1; i++)
	{
		prevSplineMesh = currentSplineMesh;
		currentSplineMesh = CastChecked<USplineMeshComponent>(ConnectionSections[i]->GetComponentByClass(USplineMeshComponent::StaticClass()), ECastCheckedType::NullAllowed);
		if (::IsValid(currentSplineMesh))
		{
			currentSplineMesh->SetStartPosition(GetAnchorLocalPosition(i), false);
			currentSplineMesh->SetEndPosition(GetAnchorLocalPosition(i + 1), false);
		}
	}
	CalculateTangents();
	CalculateUpDirs();
	for (int i = 0; i < numSections; i++)
	{
		FVector upDir = FVector::UpVector;
		currentSplineMesh = CastChecked<USplineMeshComponent>(ConnectionSections[i]->GetComponentByClass(USplineMeshComponent::StaticClass()), ECastCheckedType::NullAllowed);
		if (::IsValid(currentSplineMesh))
		{
			currentSplineMesh->UpdateMesh();
			currentSplineMesh->UpdateRenderStateAndCollision();
			currentSplineMesh->UpdateOverlaps();
		}
	}
}


void APipelineActor::CalculateTangents()
{
	USplineMeshComponent* prevSplineMesh = nullptr;
	USplineMeshComponent* currentSplineMesh = nullptr;
	int32 numSections = ConnectionSections.Num();
	int32 numPoints = ConnectionPath.Num();
	if (numPoints < numSections + 1)
	{
		UUScenUtil::Log(Logger::ERROR, "Not enough pipeline path points");
		return;
	}
	TArray<FVector> tangents;
	for (int i = 0; i < numPoints; i++)
	{
		FVector tangentVector;
		if (i == 0)
		{
			tangentVector = GetAnchorLocalPosition(i + 1) - GetAnchorLocalPosition(i);
		}
		else
			if (i < numPoints - 1)
			{
				FVector prevTangentVector = GetAnchorLocalPosition(i) - GetAnchorLocalPosition(i - 1);
				FVector nextTangentVector = GetAnchorLocalPosition(i + 1) - GetAnchorLocalPosition(i);
				tangentVector = (nextTangentVector + prevTangentVector)*0.5f;
			}
			else // i == numPoints-1, last iteration
			{
				tangentVector = GetAnchorLocalPosition(i) - GetAnchorLocalPosition(i - 1);
			}
		tangentVector.Normalize();
		tangents.Add(tangentVector*CurveSmoothing);
	}

	// weld the sections
	for (int i = 0; i < numSections; i++)
	{
		FVector tangentVector = tangents[i];
		prevSplineMesh = currentSplineMesh;
		currentSplineMesh = CastChecked<USplineMeshComponent>(ConnectionSections[i]->GetComponentByClass(USplineMeshComponent::StaticClass()), ECastCheckedType::NullAllowed);
		if (::IsValid(currentSplineMesh))
		{
			currentSplineMesh->SetStartTangent(tangents[i], false);
			currentSplineMesh->SetEndTangent(tangents[i + 1], false);
		}
	}
}


void APipelineActor::CalculateUpDirs()
{
	// TODO: optimize to avoid calls to GetAnchorLocalPosition()

	int32 numPoints = ConnectionPath.Num();
	int32 numSections = ConnectionSections.Num();
	USplineMeshComponent* currentSplineMesh = nullptr;
	for (int i = 0; i < numSections; i++)
	{
		FVector upDir = FVector::UpVector;
		currentSplineMesh = CastChecked<USplineMeshComponent>(ConnectionSections[i]->GetComponentByClass(USplineMeshComponent::StaticClass()), ECastCheckedType::NullAllowed);
		if (::IsValid(currentSplineMesh))
		{
			FVector v1 = GetAnchorLocalPosition(i + 1) - GetAnchorLocalPosition(i);
			FVector v2;
			if (i + 2 < numPoints)
			{
				v2 = GetAnchorLocalPosition(i + 2) - GetAnchorLocalPosition(i + 1);
			}
			else if (i > 1)
			{
				v2 = GetAnchorLocalPosition(i - 1) - GetAnchorLocalPosition(i);
			}
			upDir = FVector::CrossProduct(v1, v2);
			if (upDir.IsNearlyZero())
			{
				float upDot = FVector::DotProduct(v1, FVector::UpVector);
				float rightDot = FVector::DotProduct(v1, FVector::RightVector);
				upDir = upDot < rightDot ? FVector::UpVector : FVector::RightVector;
			}
			upDir.Normalize();
			currentSplineMesh->SetSplineUpDir(upDir, false);
		}
	}
}

