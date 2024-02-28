
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
			�ҷ����� �� ������ �Ҷ� Ar�� "<<" ����Ʈ �����ڸ� ���
		*/
		uint32 uMaxHp = (uint32)MaxHp;	// ���嶧�� ����
		Ar.SerializeIntPacked(uMaxHp);	// ����, �ҷ����� �Ѵٽ���
		MaxHp = (float)uMaxHp;			// �ҷ����⶧�� ����

		uint32 uAttack = (uint32) Attack;	   // ���嶧�� ����
		Ar.SerializeIntPacked(uAttack);		 // ����, �ҷ����� �Ѵٽ���
		Attack = (float) uAttack;			   // �ҷ����⶧�� ����

		uint32 uAttackRange = (uint32) AttackRange;	   // ���嶧�� ����
		Ar.SerializeIntPacked(uAttackRange);	 // ����, �ҷ����� �Ѵٽ���
		AttackRange = (float) uAttackRange;			   // �ҷ����⶧�� ����

		uint32 uAttackSpeed = (uint32) AttackSpeed;	   // ���嶧�� ����
		Ar.SerializeIntPacked(uAttackSpeed);		   // ����, �ҷ����� �Ѵٽ���
		AttackSpeed = (float) uAttackSpeed;			   // �ҷ����⶧�� ����

		uint32 uMovementSpeed= (uint32) MovementSpeed;	   // ���嶧�� ����
		Ar.SerializeIntPacked(uMovementSpeed);			   // ����, �ҷ����� �Ѵٽ���
		MovementSpeed = (float) uMovementSpeed;			   // �ҷ����⶧�� ����

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
