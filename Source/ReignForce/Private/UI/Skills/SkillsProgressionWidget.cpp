// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillsProgressionWidget.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"

#include "UI/Skills/DataAssets/SkillsDataAsset.h"
#include "UI/Skills/SkillDescriptionWidget.h"
#include "UI/Skills/SkillUpgradeWidget.h"
#include "UI/Skills/SkillsCountWidget.h"
#include "UI/Skills/StageProgress.h"
#include "UI/Skills/UpgradesProgressWidget.h"

#include "GameStates/ShooterGameState.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"

#include "Skills/ShooterSkillUpgrade.h"
#include "PlayerCharacter/PlayerCharacter.h"


void USkillsProgressionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetDescriptionVisibility(false);

    if (!(IsSkillsDataAssetValid() && IsValid(SkillDescription))) return;

    auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!IsValid(Character)) return;

	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
    if (IsValid(GameState))
    {
        ProgressState = GameState->GetUpgradesProgressStateComponent();
        if (ProgressState.IsValid())
        {
            SetSkillsAvailableCount(ProgressState->GetSkillsCount());
            SetUpgradesProgressState(ProgressState->GetUpgradesUnlocked(), ProgressState->GetMaxUpgrades());
        }
    }

    UButton* UpgradeSkillButton = SkillDescription->GetUpgradeSkillButton();
    if (IsValid(UpgradeSkillButton))
        UpgradeSkillButton->OnClicked.AddDynamic(this, &USkillsProgressionWidget::UnlockUpgradeStageForSelectedSkill);

    bool bFirstChild = true;
    if (!IsValid(SkillsContainer)) return;
    for (auto &[UpgradeClass, StageClasses] : SkillsDataAsset->PlayerSkills)
    {
        auto UpgradeWidget = CreateWidget<USkillUpgradeWidget>(this, UpgradeClass);
        if (!IsValid(UpgradeWidget)) continue;
        UpgradeWidget->OnSkillSelected.AddDynamic(this, &USkillsProgressionWidget::HandleOnSkillSelected);
        SkillsContainer->AddChild(UpgradeWidget);

        FSkillUpgradeStages& Upgrade = Skills.Add(UpgradeWidget);

        for (auto& StageClass : StageClasses.Collection)
        {
            if (!IsValid(StageClass)) continue;
            auto Stage = NewObject<UShooterSkillUpgrade>(this, StageClass);
            Upgrade.Stages.Add(Stage);
        }

        const auto [AlreadyUpgraded, StageProgress] = GetStagesStatus(Upgrade, Character);
        UpgradeWidget->LoadProgressState(StageProgress, bFirstChild);
        bFirstChild = false;
    }
}

bool USkillsProgressionWidget::IsSkillsDataAssetValid() const
{
	return IsValid(SkillsDataAsset) && SkillsDataAsset->IsValidLowLevel();
}

TArray<USkillUpgradeWidget*> USkillsProgressionWidget::GetSkillUpgrades() const
{
    if (!IsValid(SkillsContainer)) return {};

    TArray<USkillUpgradeWidget*> SkillUpgrades;
    for (UWidget* Widget : SkillsContainer->GetAllChildren())
    {
        auto Skill = Cast<USkillUpgradeWidget>(Widget);
        if (IsValid(Skill)) SkillUpgrades.Add(Skill);
    }

    return MoveTemp(SkillUpgrades);
}

void USkillsProgressionWidget::CloseAndClearDescription()
{
    if (!IsValid(SkillDescription)) return;

    SkillDescription->SetVisibility(ESlateVisibility::Hidden);
    SkillDescription->ClearDescription();
}

void USkillsProgressionWidget::SetSkillsAvailableCount(int32 Count)
{
    if (!IsValid(SkillsCount) || Count < 0) return;
    SkillsCount->SetSkillsCount(Count);
}

void USkillsProgressionWidget::SetUpgradesProgressState(int32 Current, int32 Total)
{
    if (!IsValid(UpgradesProgress) || Current < 0 || Total < 0) return;
    UpgradesProgress->ProvideWithState(Current, Total);
}

void USkillsProgressionWidget::SetDescriptionVisibility(bool bVisible)
{
    if (!IsValid(SkillDescription)) return;

	const ESlateVisibility DescriptionVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    SkillDescription->SetVisibility(DescriptionVisibility);
}

void USkillsProgressionWidget::FillDescription(USkillUpgradeWidget* Skill, const FSkillUpgradeStages& Stages)
{
    if (!(IsValid(SkillDescription) && IsValid(Skill))) return;

    auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!IsValid(Character)) return;

    const int32 Total = Stages.Stages.Num();
    int32 Upgraded = 0;
    TArray<FSkillStageDescriptionDetails> Info;
    UShooterSkillUpgrade* CurentStage = nullptr;
    for (auto& Stage : Stages.Stages)
    {
        if (!IsValid(Stage)) return;

        bool bUpgraded = Stage->IsAlreadyUpgraded(Character);
        if (bUpgraded) ++Upgraded;
        const FText Description = Stage->GetDescription(Character);
        const EStageProgress StageProgress = bUpgraded ? EStageProgress::Completed : EStageProgress::Locked;
        Info.Add({ Description, StageProgress });
        if (!(CurentStage || bUpgraded)) CurentStage = Stage.Get();
    }

    FSkillStageDescriptionDetails* NextToUpgrade = Info.FindByPredicate([] (const FSkillStageDescriptionDetails& Details)
    {
        return Details.StageProgress == EStageProgress::Locked;
    });
    if (NextToUpgrade) NextToUpgrade->StageProgress = EStageProgress::InProgress;

    SetDescriptionVisibility(true);
    SkillDescription->SetTitle(Skill->SkillName, Total, Upgraded);
    SkillDescription->SetSkillImage(Skill->GetBaseSkillIcon());
    SkillDescription->FillStagesList(Info);

    bool bNothingToUpgrade = Upgraded == Total;
    if (!bNothingToUpgrade && IsValid(CurentStage))
    {
        const int32 SkillCost = CurentStage->GetSkillCost();
        SkillDescription->SetSkillUpdateToSpend(SkillCost);
        bool bActionAllowed = CheckIfCanUpgrade(CurentStage, Character);
        SkillDescription->SetUpgradeButtonEnabled(bActionAllowed);
    }

    SkillDescription->SetUpgradeButtonVisibility(!bNothingToUpgrade);
}

void USkillsProgressionWidget::UnlockUpgradeStageForSelectedSkill()
{
    if (!(IsSkillsDataAssetValid() && IsValid(SkillDescription) && ProgressState.IsValid())) return;

    auto OnFinished = [this] { SkillDescription->SetUpgradeButtonEnabled(true); };
    SkillDescription->SetUpgradeButtonEnabled(false);

    auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (!IsValid(Character)) return OnFinished();
    auto [Skill, Upgrade] = GetCurrentSkillState(Character);
    if (!(IsValid(Skill) && IsValid(Upgrade))) return OnFinished();

    const int32 SkillCost = Upgrade->GetSkillCost();
    bool bActionAllowed = CheckIfCanUpgrade(Upgrade, Character);
    if (!bActionAllowed) return OnFinished();

    bool bUpgraded = Upgrade->UpgradeShooter(Character);
    if (!bUpgraded) return OnFinished();

    ProgressState->TryTakeSkills(SkillCost);
    bool bUnlocked = ProgressState->AddUnlokedUpgrades(1);
    if (bUnlocked) ProgressState->AccumulateSpentSkills(SkillCost);

    FSkillUpgradeStages* Stages = Skills.Find(Skill);
    if (!Stages) return OnFinished();
    const auto [AlreadyUpgraded, StageProgress] = GetStagesStatus(*Stages, Character);

    if (StageProgress == Skill->GetProgressState()) FillDescription(Skill, *Stages);
    else Skill->LoadProgressState(StageProgress, Skill->IsSelected());
}

void USkillsProgressionWidget::HandleOnSkillSelected(USkillUpgradeWidget* Skill, bool bState)
{
    bool bAllowToHandle = IsValid(Skill) && Skill->IsSelected() && bState;
    if (!bAllowToHandle) return;

    TArray<USkillUpgradeWidget*> UpgradesToHandle;
    Skills.GetKeys(UpgradesToHandle);
    UpgradesToHandle = UpgradesToHandle.FilterByPredicate([&] (USkillUpgradeWidget* Upgrade)
    {
        return Upgrade->IsSelected() && Upgrade != Skill;
    });
    for (auto& Upgrade : UpgradesToHandle) Upgrade->SetIsSelected(false);

    FSkillUpgradeStages* Stages = Skills.Find(Skill);
    if (!Stages) return;
    FillDescription(Skill, *Stages);
}

bool USkillsProgressionWidget::CheckIfCanUpgrade(UShooterSkillUpgrade* SkillUpgrade, APlayerCharacter* Character)
{
    if (!(IsValid(Character) && IsValid(SkillUpgrade))) return false;
    if (SkillUpgrade->IsAlreadyUpgraded(Character)) return false;
    if (!ProgressState.IsValid()) return false;
    return ProgressState->CanTake(SkillUpgrade->GetSkillCost());
}

TPair<USkillUpgradeWidget*, UShooterSkillUpgrade*> USkillsProgressionWidget::GetCurrentSkillState(APlayerCharacter* Character)
{
    if (!IsValid(Character)) return { nullptr, nullptr };

    USkillUpgradeWidget* Skill = nullptr;
    FSkillUpgradeStages* Stages = nullptr;
    for (auto &[CurSkill, CurStages] : Skills)
    {
        if (!CurSkill->IsSelected()) continue;
        Skill = CurSkill;
        Stages = &CurStages;
        break;
    }

    if (!(IsValid(Skill) && Stages)) return { nullptr, nullptr };
    TObjectPtr<UShooterSkillUpgrade>* UpgradePtr = Stages->Stages.FindByPredicate([&] (const TObjectPtr<UShooterSkillUpgrade>& Stage)
    {
        return !Stage->IsAlreadyUpgraded(Character);
    });
    if (!(UpgradePtr && IsValid(*UpgradePtr))) return { Skill, nullptr };
    UShooterSkillUpgrade* Upgrade = UpgradePtr->Get();
    if (!IsValid(Upgrade)) return { Skill, nullptr };

    return { Skill, Upgrade };
}

TPair<int32, EStageProgress> USkillsProgressionWidget::GetStagesStatus(const FSkillUpgradeStages& Stages, APlayerCharacter* Character) const
{
    const int32 Total = Stages.Stages.Num();
    const int32 AlreadyUpgraded = Stages.Stages.FilterByPredicate([&] (UShooterSkillUpgrade* Step)
    {
        return Step->IsAlreadyUpgraded(Character);
    }).Num();

    EStageProgress StageProgress = EStageProgress::InProgress;
    if (AlreadyUpgraded == 0) StageProgress = EStageProgress::Locked;
    else if (AlreadyUpgraded == Total) StageProgress = EStageProgress::Completed;

    return { AlreadyUpgraded, StageProgress };
}
