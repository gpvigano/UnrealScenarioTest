// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "ElementDataComponent.h"
#include "UEUtil.h"
#include "UScenUtil.h"
#include "GameFramework/Actor.h"


UElementDataComponent::UElementDataComponent()
{
}


void UElementDataComponent::ScanSockets()
{
	AActor* owner = GetOwner();
	if (owner)
	{
		if (SocketComponents.Num() > 0)
		{
			bool invalid = false;
			for (FSocketInfo& socketInfo : SocketComponents)
			{
				if (::IsValid(socketInfo.Component)
					|| socketInfo.Component->GetOwner()!= owner
					|| socketInfo.Component->ComponentHasTag(UUScenUtil::SocketTag))
				{
					invalid = true;
					socketInfo.Component = nullptr;
				}
			}
		}
		USceneComponent* sceneComp = GetOwner()->GetRootComponent();
		if (sceneComp)
		{
			TArray<USceneComponent*> sceneComponents;
			sceneComp->GetChildrenComponents(true, sceneComponents);

			for (int i = 0; i < sceneComponents.Num(); i++)
			{
				USceneComponent* sceneComponent = sceneComponents[i];
				UPrimitiveComponent* primComp = Cast<UPrimitiveComponent>(sceneComponent);
				if (primComp != nullptr && primComp->ComponentHasTag(UUScenUtil::SocketTag))
				{
					FString compName = primComp->GetName();
					FSocketInfo* found = SocketComponents.FindByPredicate(
						[&compName](const FSocketInfo& info)
					{
						return info.Id == compName;
					});
					if (found)
					{
						found->Component = primComp;
						found->bHidden = primComp->bHiddenInGame;
					}
					else
					{
						FSocketInfo socketInfo;
						socketInfo.Id = compName;
						socketInfo.Component = primComp;
						socketInfo.bHidden = primComp->bHiddenInGame;
						SocketComponents.Add(socketInfo);
					}
				}
			}
		}
	}
}


FSocketInfo* UElementDataComponent::GetSocketInfo(FString Id)
{
	FSocketInfo* found = SocketComponents.FindByPredicate(
		[&Id](const FSocketInfo& info)
	{
		return info.Id == Id;
	});
	return found;
}


UPrimitiveComponent* UElementDataComponent::GetSocketComponent(FString Id)
{
	FSocketInfo* found = SocketComponents.FindByPredicate(
		[&Id](const FSocketInfo& info)
	{
		return info.Id == Id;
	});
	if (!found)
	{
		return nullptr;
	}
	return Cast<UPrimitiveComponent>(found->Component);
}


FString UElementDataComponent::FindSocketIdByComponent(USceneComponent* PrimComp)
{
	FSocketInfo* found = SocketComponents.FindByPredicate(
		[&PrimComp](const FSocketInfo& info)
	{
		return info.Component == PrimComp;
	});
	if (!found)
	{
		return FString();
	}
	return found->Id;
}


FSocketInfo* UElementDataComponent::GetSocketInfoByComponent(UPrimitiveComponent* PrimComp)
{
	FString Id = FindSocketIdByComponent(PrimComp);
	return GetSocketInfo(Id);
}


void UElementDataComponent::HighlightSocket(UPrimitiveComponent* PrimComp, bool Outline, EEditFlags Mode)
{
	FSocketInfo* thisSocket = GetSocketInfoByComponent(PrimComp);
	if (thisSocket)
	{
		if (Outline)
		{
			PrimComp->bHiddenInGame = false;
			UUEUtil::TogglePrimitiveOutline(PrimComp, Outline, Mode);
		}
		else
		{
			UUEUtil::TogglePrimitiveOutline(PrimComp, false, Mode);
			PrimComp->bHiddenInGame = thisSocket->bHidden;
		}
	}
}


// Called when the game starts
void UElementDataComponent::BeginPlay()
{
	Super::BeginPlay();

	if (SocketComponents.Num() == 0)
	{
		ScanSockets();
	}
	// ...
}

