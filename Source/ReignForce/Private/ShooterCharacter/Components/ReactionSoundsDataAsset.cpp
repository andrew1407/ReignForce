#include "ShooterCharacter/Components/ReactionSoundsDataAsset.h"


UReactionSoundsDataAsset::UReactionSoundsDataAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    OnLandedSoundOptions.Volume = 1;
	OnLandedSoundOptions.Tag = TEXT("JumpLanding");

	JumpGruntSoundOptions.Volume = .6f;
	JumpGruntSoundOptions.Tag = TEXT("Jump");

	DamageTakenOptions =
	{
		{ EWeaponType::UNARMED, FSoundOptions() },
		{ EWeaponType::MELEE, FSoundOptions() },
		{ EWeaponType::PISTOL, FSoundOptions() },
		{ EWeaponType::RIFLE, FSoundOptions() },
	};
	
	DamageTakenOptions[EWeaponType::UNARMED].LoudnessRange = 500;
	DamageTakenOptions[EWeaponType::MELEE].LoudnessRange = 500;
	DamageTakenOptions[EWeaponType::PISTOL].LoudnessRange = 500;
	DamageTakenOptions[EWeaponType::RIFLE].LoudnessRange = 500;
	DefaultDamageOptions.LoudnessRange = 500;

	DamageScreamChances =
	{
		{ EWeaponType::UNARMED, 1 },
		{ EWeaponType::MELEE, 1 },
		{ EWeaponType::PISTOL, 1 },
		{ EWeaponType::RIFLE, .5f },
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
