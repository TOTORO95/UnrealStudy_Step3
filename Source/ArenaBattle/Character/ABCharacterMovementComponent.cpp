// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/ABCharacterMovementComponent.h"

#include "ArenaBattle.h"
#include "GameFramework/Character.h"

UABCharacterMovementComponent::UABCharacterMovementComponent()
{
	TeleportOffset = 600.0f;
	TeleportCoolTime = 3.0f;
	bPressedTeleport = false;
	bDidTeleport = false;
}

void UABCharacterMovementComponent::SetTeleportCommand()
{
	bPressedTeleport = true;
}

FNetworkPredictionData_Client* UABCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UABCharacterMovementComponent* MutableThis = const_cast<UABCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FABNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

void UABCharacterMovementComponent::ABTeleport()
{
	if (CharacterOwner)
	{
		AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));
		FVector TargetLocation = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * TeleportOffset;
		CharacterOwner->TeleportTo(TargetLocation, CharacterOwner->GetActorRotation(), false, true);
		bDidTeleport = true;

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle,
			FTimerDelegate::CreateLambda(
				[&]
				{
					bDidTeleport = false;
					AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport End"));
				}),
			TeleportCoolTime, false, -1.0f);
	}
}

void UABCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	if (bPressedTeleport && !bDidTeleport)
	{
		ABTeleport();
	}
	if (bPressedTeleport)
	{
		bPressedTeleport = false;
	}
}

void UABCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bPressedTeleport = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bDidTeleport= (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	if (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		if (bPressedTeleport && !bDidTeleport)
		{
			AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));
			ABTeleport();
		}
	}

}

FSavedMovePtr FABNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new FABSavedMove_Character());
}

FABNetworkPredictionData_Client_Character::FABNetworkPredictionData_Client_Character(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

void FABSavedMove_Character::Clear()
{
	Super::Clear();

	bPressedTeleport = false;
	bDidTeleport = false;
}

void FABSavedMove_Character::SetInitialPosition(ACharacter* Character)
{
	Super::SetInitialPosition(Character);
	UABCharacterMovementComponent* ABMovement = Cast<UABCharacterMovementComponent>(Character->GetCharacterMovement());
	if (ABMovement)
	{
		bPressedTeleport = ABMovement->bPressedTeleport;
		bDidTeleport = ABMovement->bDidTeleport;
	}
}

uint8 FABSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	if (bDidTeleport)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}
