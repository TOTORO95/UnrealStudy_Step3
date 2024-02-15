// Fill out your copyright notice in the Description page of Project Settings.

#include "Prop/ABFountain.h"

#include "ArenaBattle.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AABFountain::AABFountain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(
		TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/"
			 "SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(
		TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	bReplicates = true;
	NetUpdateFrequency = 1.0f;
	NetCullDistanceSquared = 4000000.0f;
	// NetDormancy = DORM_Initial;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		//FTimerHandle Handle;
		//GetWorld()->GetTimerManager().SetTimer(Handle,
		//	FTimerDelegate::CreateLambda(
		//		[&]()
		//		{
		//			// BigData.Init(BigDataElement, 1000);
		//			// BigDataElement += 1.0f;
		//			// ServerRotationYaw += 1.0f;
		//			// ServerLightColor = FLinearColor(
		//			//	FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::FRandRange(0.0f, 1.0f), 1.0f);
		//			// OnRep_ServerLightColor();
		//			 const FLinearColor NewLightColor = FLinearColor(
		//				FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::FRandRange(0.0f, 1.0f), 1.0f);
		//			ClientRPC_ChangeLightColor(NewLightColor);
		//		// ServerRPC_ChangeLightColor();

		//		}),
		//	1.0f, true, 0.0f);

		//// Server���ִ� Actor�� Onwer�� Client������ ���� 10���� �����̸� �־� ����
		//FTimerHandle Handle2;
		//GetWorld()->GetTimerManager().SetTimer(Handle2,
		//	FTimerDelegate::CreateLambda(
		//		[&]()
		//		{
		//			//for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		//			//{
		//			//	APlayerController* PlayerController = Iterator->Get();
		//			//	if (PlayerController && !PlayerController->IsLocalController())
		//			//	{
		//			//		SetOwner(PlayerController);
		//			//		break;
		//			//	}
		//			//}

		//			for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
		//			{
		//				if (PlayerController && !PlayerController->IsLocalController())
		//				{
		//					SetOwner(PlayerController);
		//					break;
		//				}
		//			}

		//		}),
		//	10.0f, false, -1.0f);
	}
	else
	{
		//// Client�� Proxy���� Server�� �����Ѱ��̱⋚���� Onwer ������ �ǹ̰�����
		//// Network������ LocalFunction�� ȣ���ϰԵ� �ڵ��̴�.
		//SetOwner(GetWorld()->GetFirstPlayerController());	 // �ش������ ������ Onwer�� �����ѰԾƴϰԵȴ�.
		//FTimerHandle Handle;
		//GetWorld()->GetTimerManager().SetTimer(
		//	Handle, FTimerDelegate::CreateLambda([&]() { ServerRPC_ChangeLightColor(); }), 1.0f, true, 0.0f);
	}
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())	   // Server������ �۵��ϰ� AuthorityȮ��
	{
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	else
	{
		// ClientTimeSinceUpdate += DeltaTime;
		// if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER)
		//{
		//	return;
		// }

		// const float EstimateRotationYaw = ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;
		// const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		// FRotator ClientRotator = RootComponent->GetComponentRotation();
		// const float ClientNewYaw = FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);
		// ClientRotator.Yaw = ClientNewYaw;
		// RootComponent->SetWorldRotation(ClientRotator);
	}
}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AABFountain, ServerRotationYaw);
	// DOREPLIFETIME(AABFountain, BigData);
	// DOREPLIFETIME_CONDITION(AABFountain, ServerLightColor, COND_InitialOnly);
	DOREPLIFETIME(AABFountain, ServerLightColor);
}

//void AABFountain::OnActorChannelOpen(FInBunch& InBunch, UNetConnection* Connection)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//	Super::OnActorChannelOpen(InBunch, Connection);
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//bool AABFountain::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
//{
//	bool NetRelevantResult = Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
//	if (!NetRelevantResult)
//	{
//		AB_LOG(LogABNetwork, Log, TEXT("Not Relevant:[%s] %s"), *RealViewer->GetName(), *SrcLocation.ToCompactString())
//	}
//	return NetRelevantResult;
//}
//
//void AABFountain::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//	Super::PreReplication(ChangedPropertyTracker);
//}

void AABFountain::OnRep_ServerRotationYaw()
{
	// AB_LOG(LogABNetwork, Log, TEXT("Yaw : %f"), ServerRotationYaw);

	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;
	RootComponent->SetWorldRotation(NewRotator);

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.0f;
}

void AABFountain::OnRep_ServerLightColor()
{
	if (!HasAuthority())
	{
		AB_LOG(LogABNetwork, Log, TEXT("Light Color : %s"), *ServerLightColor.ToString());
	}
	UPointLightComponent* PointLight = Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));
	if (PointLight)
	{
		PointLight->SetLightColor(ServerLightColor);
	}
}

void AABFountain::MulticastRPC_ChangeLightColor_Implementation(const FLinearColor& NewLinearColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("Light Color : %s"), *NewLinearColor.ToString());

	UPointLightComponent* PointLight = Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));
	if (PointLight)
	{
		PointLight->SetLightColor(NewLinearColor);
	}
}

void AABFountain::ServerRPC_ChangeLightColor_Implementation()
{
	const FLinearColor NewLightColor =
		FLinearColor(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::FRandRange(0.0f, 1.0f), 1.0f);
	MulticastRPC_ChangeLightColor(NewLightColor);
}

void AABFountain::ClientRPC_ChangeLightColor_Implementation(const FLinearColor& NewLinearColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("Light Color : %s"), *NewLinearColor.ToString());

	UPointLightComponent* PointLight = Cast<UPointLightComponent>(GetComponentByClass(UPointLightComponent::StaticClass()));
	if (PointLight)
	{
		PointLight->SetLightColor(NewLinearColor);
	}
}
