// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/EditorAnimationUtils.h"
#include "Animation/BlendSpace.h"


#if WITH_EDITOR
bool UEditorAnimationUtils::ResampleBlendSpace(UBlendSpace* BlendSpace)
{
	if (!IsValid(BlendSpace)) return false;
	BlendSpace->Modify();
	BlendSpace->ValidateSampleData();
	BlendSpace->ResampleData();
	return BlendSpace->MarkPackageDirty();
}
#endif
