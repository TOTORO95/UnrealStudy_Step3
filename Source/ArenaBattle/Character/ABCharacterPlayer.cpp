// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/ABCharacterPlayer.h"

#include "ABCharacterControlData.h"
#include "ABCharacterMovementComponent.h"
#include "ArenaBattle.h"
#include "Camera/CameraComponent.h"
#include "CharacterStat/ABCharacterStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "Interface/ABGameInterface.h"
#include "Net/UnrealNetwork.h"
#include "Physics/ABCollision.h"
#include "UI/ABHUDWidget.h"

AABCharacterPlayer::AABCharacterPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UABCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeActionControlRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl'"));
	if (nullptr != InputChangeActionControlRef.Object)
	{
		ChangeControlAction = InputChangeActionControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove'"));
	if (nullptr != InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook'"));
	if (nullptr != InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove'"));
	if (nullptr != InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionTeleportRef(
		TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Teleport.IA_Teleport'"));
	if (nullptr != InputActionTeleportRef.Object)
	{
		TeleportAction = InputActionTeleportRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;
	bCanAttack = true;
}

void AABCharacterPlayer::BeginPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}

	SetCharacterControl(CurrentCharacterControlType);
}

void AABCharacterPlayer::SetDead()
{
	Super::SetDead();

	GetWorldTimerManager().SetTimer(DeadTimerHandle, this, &AABCharacterPlayer::ResetPlayer, 5.0f, false);
	// APlayerController* PlayerController = Cast<APlayerController>(GetController());
	// if (PlayerController)
	//{
	//	DisableInput(PlayerController);
	// }
}

void AABCharacterPlayer::PossessedBy(AController* NewController)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	PrintOwnerName(LOG_CALLINFO);
	Super::PossessedBy(NewController);
	UpdateMeshFromPlayerState();

	PrintOwnerName(LOG_CALLINFO);
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::PostNetInit()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s %s"), TEXT("Begin"), *GetName());
	Super::PostNetInit();
	PrintOwnerName(LOG_CALLINFO);
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::OnRep_Owner()
{
	Super::OnRep_Owner();
	PrintOwnerName(LOG_CALLINFO);
}

void AABCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(
		ChangeControlAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ChangeCharacterControl);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Attack);
	EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Teleport);
}

void AABCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		SetCharacterControl(ECharacterControlType::Shoulder);
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		SetCharacterControl(ECharacterControlType::Quater);
	}
}

void AABCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UABCharacterControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void AABCharacterPlayer::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = CharacterControlData->bDoCollisionTest;
}

void AABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	if (!bCanAttack)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	if (!bCanAttack)
	{
		return;
	}
	FVector2D MovementVector = Value.Get<FVector2D>();

	float InputSizeSquared = MovementVector.SquaredLength();
	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();
	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVector.Normalize();
		MovementVectorSizeSquared = 1.0f;
	}
	else
	{
		MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorSize);
}

void AABCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::Attack()
{
	// ProcessComboCommand();
	if (bCanAttack)
	{
		if (!HasAuthority())
		{
			bCanAttack = false;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AABCharacterPlayer::ResetAttack, AttackTime, false);
			PlayAttackAnimation();
		}

		ServerRPCAttack(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());

		// 아래의 공격판정 로직은 서버에서 진행
		// bCanAttack = false;
		// GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		// FTimerHandle Handle;
		// GetWorld()->GetTimerManager().SetTimer(Handle,
		//	FTimerDelegate::CreateLambda(
		//		[&]
		//		{
		//			bCanAttack = true;
		//			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		//		}),
		//	AttackTime, false, -1.0f);
		// UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		// if (AnimInstance)
		//{
		//	AnimInstance->Montage_Play(ComboActionMontage);
		//}
	}
}

void AABCharacterPlayer::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(ComboActionMontage);
	}
}

void AABCharacterPlayer::AttackHitCheck()
{
	if (IsLocallyControlled())	  // 소유권을 가진 Client에서 판정을 진행하도록 변경
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const float AttackDamage = Stat->GetTotalStat().Attack;
		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(
			OutHitResult, Start, End, FQuat::Identity, CCHANNEL_ABACTION, FCollisionShape::MakeSphere(AttackRadius), Params);

		float HitCheckTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		if (HasAuthority())
		{
			FColor DebugColor = HitDetected ? FColor::Green : FColor::Red;
			DrawDebugAttackRange(DebugColor, Start, End, Forward);
			if (HitDetected)
			{
				AttackHitConfirm(OutHitResult.GetActor());
			}
		}
		else
		{
			if (HitDetected)
			{
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
			}
			else
			{
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}
		}
	}
}

void AABCharacterPlayer::AttackHitConfirm(AActor* HitActor)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	if (HasAuthority())
	{
		const float AttackDamage = Stat->GetTotalStat().Attack;
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
	}
}

void AABCharacterPlayer::DrawDebugAttackRange(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward)
{
#if ENABLE_DRAW_DEBUG
	float AttackRange = Stat->GetTotalStat().AttackRange;
	float AttackRadius = Stat->GetAttackRadius();
	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius,
		FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);
#endif
}

void AABCharacterPlayer::ClientRPCPlayAnimaiton_Implementation(AABCharacterPlayer* CharacterToPlay)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	if (CharacterToPlay)
	{
		CharacterToPlay->PlayAttackAnimation();
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(const FHitResult& HitResult, float HitCheckTime)
{
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(
	FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}
void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(const FHitResult& HitResult, float HitCheckTime)
{
	AActor* HitActor = HitResult.GetActor();
	if (IsValid(HitActor))
	{
		const FVector HitLocation = HitResult.Location;
		const FBox HitBox = HitActor->GetComponentsBoundingBox();
		const FVector ActorBoxCenter = (HitBox.Min + HitBox.Max) * 0.5f;
		if (FVector::DistSquared(HitLocation, ActorBoxCenter) <= AcceptCheckDistance * AcceptCheckDistance)
		{
			AttackHitConfirm(HitActor);
		}
		else
		{
			AB_LOG(LogABNetwork, Warning, TEXT("%s"), TEXT("HitTest Rejected!!!!!!!!!!"));
		}
#if ENABLE_DRAW_DEBUG
		DrawDebugPoint(GetWorld(), ActorBoxCenter, 50.0f, FColor::Cyan, false, 5.0f);
		DrawDebugPoint(GetWorld(), HitLocation, 50.0f, FColor::Magenta, false, 5.0f);
#endif
		DrawDebugAttackRange(FColor::Green, HitResult.TraceStart, HitResult.TraceEnd, HitActor->GetActorForwardVector());
	}
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(
	FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, FVector_NetQuantizeNormal TraceDir, float HitCheckTime)
{
	DrawDebugAttackRange(FColor::Red, TraceStart, TraceEnd, TraceDir);
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	return (AttackStartTime - LastAttackStartTime) > (AttackTime - 0.4f);
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	bCanAttack = false;
	OnRep_CanAttack();

	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;
	AB_LOG(LogABNetwork, Log, TEXT("Lag Time : %d"), AttackTimeDifference);
	AttackTimeDifference = FMath::Clamp(AttackTimeDifference, 0.0f, AttackTime - 0.01f);

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle, this, &AABCharacterPlayer::ResetAttack, AttackTime - AttackTimeDifference, false);

	LastAttackStartTime = AttackTime;
	PlayAttackAnimation();
	// MulticastRPCAttack();
	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (!PlayerController)
		{
			continue;
		}
		if (GetController() != PlayerController)	// 현재 logic수행중인 controller는 제외
		{
			if (!PlayerController->IsLocalController())	   // Client일때만
			{
				AABCharacterPlayer* OtherPlayer = Cast<AABCharacterPlayer>(PlayerController->GetPawn());
				if (OtherPlayer)
				{
					OtherPlayer->ClientRPCPlayAnimaiton(this);
				}
			}
		}
	}
}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	if (!IsLocallyControlled())
	{
		PlayAttackAnimation();
	}

	// AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	// if (HasAuthority())
	//{
	//	bCanAttack = false;
	//	// C++에서Server에서는 property replicate 이벤트가 자동으로 호출되지않고 클라이언트에서만 호출되기때문에 서버에서 호출하려면
	//	// 명시적으로 해주어야함
	//	OnRep_CanAttack();
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(Handle,
	//		FTimerDelegate::CreateLambda(
	//			[&]
	//			{
	//				bCanAttack = true;
	//				OnRep_CanAttack();
	//			}),
	//		AttackTime, false, -1.0f);
	// }

	// UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// if (AnimInstance)
	//{
	//	AnimInstance->Montage_Play(ComboActionMontage);
	// }
}

void AABCharacterPlayer::OnRep_CanAttack()
{
	if (bCanAttack)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
}

void AABCharacterPlayer::Teleport()
{
	AB_LOG(LogABTeleport, Log, TEXT("%s"), TEXT("Begin"));
	UABCharacterMovementComponent* ABMovement = Cast<UABCharacterMovementComponent>(GetCharacterMovement());
	if (ABMovement)
	{
		ABMovement->SetTeleportCommand();
	}
}

void AABCharacterPlayer::PrintOwnerName()
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner : %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner : %s"), TEXT("Have Not Onwer"));
	}
}

void AABCharacterPlayer::PrintOwnerName(const FString& InFunctionName)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG2(LogABNetwork, Log, TEXT("%s Owner : %s"), *InFunctionName, *OwnerActor->GetName());
	}
	else
	{
		AB_LOG2(LogABNetwork, Log, TEXT("%s Owner : %s"), *InFunctionName, TEXT("Have Not Onwer"));
	}
}

void AABCharacterPlayer::SetupHUDWidget(UABHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateStat(Stat->GetBaseStat(), Stat->GetModifierStat());
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp(), Stat->GetMaxHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}

void AABCharacterPlayer::ResetPlayer()
{
	UAnimInstance* AnimInstnace = GetMesh()->GetAnimInstance();
	if (AnimInstnace)
	{
		AnimInstnace->StopAllMontages(0.0f);
	}
	Stat->SetLevelStat(1);
	Stat->ResetStat();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	SetActorEnableCollision(true);
	// HpBar->SetHiddenInGame(false);
	if (HasAuthority())
	{
		IABGameInterface* ABGameMode = GetWorld()->GetAuthGameMode<IABGameInterface>();
		if (ABGameMode)
		{
			FTransform NewTransform = ABGameMode->GetRandomStartTransform();
			TeleportTo(NewTransform.GetLocation(), NewTransform.GetRotation().Rotator());
		}
	}
}

void AABCharacterPlayer::ResetAttack()
{
	bCanAttack = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

float AABCharacterPlayer::TakeDamage(
	float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Stat->GetCurrentHp() <= 0.0f)
	{
		IABGameInterface* ABGameMode = GetWorld()->GetAuthGameMode<IABGameInterface>();
		if (ABGameMode)
		{
			ABGameMode->OnPlayerKilled(EventInstigator, GetController(), this);
		}
		return 0.0f;
	}

	return ActualDamage;
}

void AABCharacterPlayer::UpdateMeshFromPlayerState()
{
	int32 MeshIndex = FMath::Clamp(GetPlayerState()->PlayerId % PlayerMeshes.Num(), 0, PlayerMeshes.Num() - 1);
	MeshHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		PlayerMeshes[MeshIndex], FStreamableDelegate::CreateUObject(this, &AABCharacterBase::MeshLoadCompleted));
}

void AABCharacterPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UpdateMeshFromPlayerState();
}
