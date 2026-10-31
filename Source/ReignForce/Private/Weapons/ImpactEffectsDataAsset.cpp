// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ImpactEffectsDataAsset.h"


const FSurfaceImpactEffect& UImpactEffectsDataAsset::GetEffect(EPhysicalSurface Surface) const
{
    const FSurfaceImpactEffect* Effect = Surfaces.Find(Surface);
    return Effect ? *Effect : Default;
}
