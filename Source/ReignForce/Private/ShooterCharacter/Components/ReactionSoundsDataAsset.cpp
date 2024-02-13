#include "ShooterCharacter/Components/ReactionSoundsDataAsset.h"


UReactionSoundsDataAsset::UReactionSoundsDataAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    OnLandedSoundOptions.Volume = 1;
	OnLandedSoundOptions.Tag = TEXT("JumpLanding");

	JumpGruntSoundOptions.Volume = .6f;
	JumpGruntSoundOptions.Tag = TEXT("Jump");

	DamageTakenOptions =
	{
		{ EWeaponType::Unarmed, FSoundOptions() },
		{ EWeaponType::Melee, FSoundOptions() },
		{ EWeaponType::Pistol, FSoundOptions() },
		{ EWeaponType::Rifle, FSoundOptions() },
	};
	
	DamageTakenOptions[EWeaponType::Unarmed].LoudnessRange = 500;
	DamageTakenOptions[EWeaponType::Melee].LoudnessRange = 500;
	DamageTakenOptions[EWeaponType::Pistol].LoudnessRange = 500;
	DamageTakenOptions[EWeaponType::Rifle].LoudnessRange = 500;
	DefaultDamageOptions.LoudnessRange = 500;

	DamageScreamChances =
	{
		{ EWeaponType::Unarmed, 1 },
		{ EWeaponType::Melee, 1 },
		{ EWeaponType::Pistol, 1 },
		{ EWeaponType::Rifle, .5f },
	};

	PerishSoundOptions.LoudnessRange = 1200;

	WhenHeardOptions.Volume = 1;
	WhenHeardOptions.LoudnessRange = 200;
	WhenHeardOptions.Tag = TEXT("Heard");

	WhenSeenOptions.Volume = 1;
	WhenSeenOptions.LoudnessRange = 400;
	WhenSeenOptions.Tag = TEXT("Seen");

	WhenLostSightOptions.Volume = 1;
	WhenLostSightOptions.LoudnessRange = 200;
	WhenLostSightOptions.Tag = TEXT("JoydSound");

	JoyOptions.Volume = 1;
	JoyOptions.LoudnessRange = 500;
	JoyOptions.Tag = TEXT("LostSight");

	AttackGruntOptions.Volume = .6f;
	AttackGruntOptions.LoudnessRange = 150;
	AttackGruntOptions.Tag = TEXT("AttackGrunt");
}
