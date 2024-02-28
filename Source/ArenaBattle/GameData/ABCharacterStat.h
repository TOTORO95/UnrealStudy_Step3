
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ABCharacterStat.generated.h"

USTRUCT(BlueprintType)
struct FABCharacterStat : public FTableRowBase
{
	GENERATED_BODY()

public:
	FABCharacterStat() : MaxHp(0.0f), Attack(0.0f), AttackRange(0.0f), AttackSpeed(0.0f) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MovementSpeed;

	FABCharacterStat operator+(const FABCharacterStat& Other) const
	{
		const float* const ThisPtr = reinterpret_cast<const float* const>(this);
		const float* const OtherPtr = reinterpret_cast<const float* const>(&Other);

		FABCharacterStat Result;
		float* ResultPtr = reinterpret_cast<float*>(&Result);
		int32 StatNum = sizeof(FABCharacterStat) / sizeof(float);
		for (int32 i = 0; i < StatNum; i++)
		{
			ResultPtr[i] = ThisPtr[i] + OtherPtr[i];
		}

		return Result;
	}
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		/*
			불러오기 및 저장을 할때 Ar에 "<<" 시프트 연산자만 사용
		*/
		uint32 uMaxHp = (uint32)MaxHp;	// 저장때만 실행
		Ar.SerializeIntPacked(uMaxHp);	// 저장, 불러오기 둘다실행
		MaxHp = (float)uMaxHp;			// 불러오기때만 실행

		uint32 uAttack = (uint32) Attack;	   // 저장때만 실행
		Ar.SerializeIntPacked(uAttack);		 // 저장, 불러오기 둘다실행
		Attack = (float) uAttack;			   // 불러오기때만 실행

		uint32 uAttackRange = (uint32) AttackRange;	   // 저장때만 실행
		Ar.SerializeIntPacked(uAttackRange);	 // 저장, 불러오기 둘다실행
		AttackRange = (float) uAttackRange;			   // 불러오기때만 실행

		uint32 uAttackSpeed = (uint32) AttackSpeed;	   // 저장때만 실행
		Ar.SerializeIntPacked(uAttackSpeed);		   // 저장, 불러오기 둘다실행
		AttackSpeed = (float) uAttackSpeed;			   // 불러오기때만 실행

		uint32 uMovementSpeed= (uint32) MovementSpeed;	   // 저장때만 실행
		Ar.SerializeIntPacked(uMovementSpeed);			   // 저장, 불러오기 둘다실행
		MovementSpeed = (float) uMovementSpeed;			   // 불러오기때만 실행

		return true;
	}
};

template <>
struct TStructOpsTypeTraits<FABCharacterStat> : public TStructOpsTypeTraitsBase2<FABCharacterStat>
{
	enum
	{
		WithNetSerializer = true
	};
};
