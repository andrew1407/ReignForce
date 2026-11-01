// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillsProgressionWidget.h"
#include "Kismet/GameplayStatics.h"

#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "Components/AudioComponent.h"

#include "UI/Skills/DataAssets/SkillsDataAsset.h"
#include "UI/Skills/SkillDescriptionWidget.h"
#include "UI/Skills/SkillUpgradeWidget.h"
#include "UI/Skills/SkillUpgradeConfirmationWidget.h"
#include "UI/Skills/SkillsCountWidget.h"
#include "UI/Skills/StageProgress.h"
#include "UI/Skills/UpgradesProgressWidget.h"

#include "GameStates/ShooterGameState.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"

#include "Skills/ShooterSkillUpgrade.h"
#include "PlayerCharacter/PlayerCharacter.h"


USkillsProgressionWidget::USkillsProgressionWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    UpgradeConfirmationClass = USkillUpgradeConfirmationWidget::StaticClass();
}

void USkillsProgressionWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // Skills grid scrolls when the screen is too short for all of its rows
    WrapInScrollBox(SkillsContainer);
}

void USkillsProgressionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetDescriptionVisibility(false);

    // Close button (and back action) asks the owner (HUD) to close the menu
    if (IsValid(CloseMenuButton))
        CloseMenuButton->OnClicked.AddUniqueDynamic(this, &UReignForceActivatableWidget::RequestClose);
    RegisterFocusableButton(CloseMenuButton);
    if (IsValid(SkillDescription)) RegisterFocusableButton(SkillDescription->GetUpgradeSkillButton());

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
        UpgradeSkillButton->OnClicked.AddDynamic(this, &USkillsProgressionWidget::RequestUpgradeConfirmation);

    bool bFirstChild = true;
    if (!IsValid(SkillsContainer)) return;
    for (auto &[UpgradeClass, StageClasses] : SkillsDataAsset->PlayerSkills)
    {
        auto UpgradeWidget = CreateWidget<USkillUpgradeWidget>(this, UpgradeClass);
        if (!IsValid(UpgradeWidget)) continue;
        UpgradeWidget->OnSkillSelected.AddDynamic(this, &USkillsProgressionWidget::HandleOnSkillSelected);
        UpgradeWidget->OnUpgradeRequested.AddDynamic(this, &USkillsProgressionWidget::HandleSkillUpgradeRequested);
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

        if (UButton* SkillButton = UpgradeWidget->GetActionButton())
        {
            FCustomWidgetNavigationDelegate OnNavigation;
            OnNavigation.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(USkillsProgressionWidget, HandleSkillNavigation));
            for (EUINavigation Direction : { EUINavigation::Left, EUINavigation::Right, EUINavigation::Up, EUINavigation::Down })
                SkillButton->SetNavigationRuleCustom(Direction, OnNavigation);
        }
    }
}

UWidget* USkillsProgressionWidget::HandleSkillNavigation(EUINavigation Direction)
{
    TArray<USkillUpgradeWidget*> SkillUpgrades = GetSkillUpgrades();
    const int32 Current = SkillUpgrades.IndexOfByPredicate([] (const USkillUpgradeWidget* Skill)
    {
        return IsValid(Skill->GetActionButton()) && Skill->GetActionButton()->HasAnyUserFocus();
    });
    if (Current == INDEX_NONE) return nullptr;
    auto ButtonAt = [&] (int32 Index) -> UWidget* { return SkillUpgrades.IsValidIndex(Index) ? SkillUpgrades[Index]->GetActionButton() : nullptr; };

    // Skills are laid out in reading order, so the previous & next ones continue on the neighbour rows
    if (Direction == EUINavigation::Left) return ButtonAt(Current - 1);
    if (Direction == EUINavigation::Right) return ButtonAt(Current + 1);

    const bool bDown = Direction == EUINavigation::Down;
    const FGeometry& CurrentGeometry = SkillUpgrades[Current]->GetCachedGeometry();
    const FVector2f CurrentCenter = CurrentGeometry.GetAbsolutePositionAtCoordinates(FVector2f(.5f));
    const float RowThreshold = CurrentGeometry.GetAbsoluteSize().Y * .5f;

    UWidget* Closest = nullptr;
    float ClosestRowDistance = TNumericLimits<float>::Max();
    float ClosestColumnDistance = TNumericLimits<float>::Max();
    for (int32 i = 0; i < SkillUpgrades.Num(); ++i)
    {
        const FVector2f Center = SkillUpgrades[i]->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2f(.5f));
        const float RowDistance = bDown ? Center.Y - CurrentCenter.Y : CurrentCenter.Y - Center.Y;
        if (i == Current || RowDistance < RowThreshold) continue;
        // The nearest row first, then the closest column in it
        const float ColumnDistance = FMath::Abs(Center.X - CurrentCenter.X);
        bool bNearerRow = RowDistance < ClosestRowDistance - RowThreshold;
        bool bSameRow = FMath::Abs(RowDistance - ClosestRowDistance) <= RowThreshold;
        if (bNearerRow || (bSameRow && ColumnDistance < ClosestColumnDistance))
        {
            Closest = ButtonAt(i);
            ClosestRowDistance = RowDistance;
            ClosestColumnDistance = ColumnDistance;
        }
    }
    if (IsValid(Closest)) return Closest;
    return bDown ? CloseMenuButton.Get() : nullptr;
}

UWidget* USkillsProgressionWidget::NativeGetDesiredFocusTarget() const
{
    // Selected skill keeps the focus, the first one is focused when the menu is opened
    TArray<USkillUpgradeWidget*> SkillUpgrades = GetSkillUpgrades();
    USkillUpgradeWidget* const* Selected = SkillUpgrades.FindByPredicate([] (USkillUpgradeWidget* Skill) { return Skill->IsSelected(); });
    USkillUpgradeWidget* Target = Selected ? *Selected : (SkillUpgrades.IsEmpty() ? nullptr : SkillUpgrades[0]);
    if (IsValid(Target) && IsValid(Target->GetActionButton())) return Target->GetActionButton();
    return Super::NativeGetDesiredFocusTarget();
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

	PlayOnUpgradeSound(StageProgress);

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

void USkillsProgressionWidget::RequestUpgradeConfirmation()
{
    if (IsChildMenuOpened() || !IsValid(UpgradeConfirmationClass) || !IsValid(SkillDescription)) return;

    auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    auto [Skill, Upgrade] = GetCurrentSkillState(Character);
    if (!(IsValid(Skill) && IsValid(Upgrade) && CheckIfCanUpgrade(Upgrade, Character))) return;

    const FSkillUpgradeStages* Stages = Skills.Find(Skill);
    const int32 Total = Stages ? Stages->Stages.Num() : 0;
    const int32 Stage = Stages ? Stages->Stages.IndexOfByKey(Upgrade) + 1 : 0;
    const int32 Cost = Upgrade->GetSkillCost();

    const FText Title = FText::Format(FText::FromString(TEXT("Upgrade {0}?")), FText::FromName(Skill->SkillName));
    const FText Details = FText::Format(
        FText::FromString(TEXT("Stage {0} of {1}: {2}\nCost: {3} {3}|plural(one=skill point,other=skill points)")),
        Stage, Total, Upgrade->GetDescription(Character), Cost
    );

    auto Confirmation = CreateWidget<USkillUpgradeConfirmationWidget>(GetOwningPlayer(), UpgradeConfirmationClass);
    if (!IsValid(Confirmation)) return;
    Confirmation->Setup(SkillDescription->GetUpgradeSkillButton(), Title, Details);
    Confirmation->OnConfirmation.AddDynamic(this, &USkillsProgressionWidget::HandleUpgradeConfirmation);
    PushChildMenu(Confirmation, 20);
}

void USkillsProgressionWidget::HandleSkillUpgradeRequested(USkillUpgradeWidget* Skill)
{
    if (IsValid(Skill) && Skill->IsSelected()) RequestUpgradeConfirmation();
}

void USkillsProgressionWidget::HandleUpgradeConfirmation(bool bAccepted)
{
    CloseChildMenu();
    if (bAccepted) UnlockUpgradeStageForSelectedSkill();

    // Navigation continues from the skill the upgrade was asked for
    UWidget* Target = GetDesiredFocusTarget();
    if (IsValid(Target)) Target->SetFocus();
}

bool USkillsProgressionWidget::CheckIfCanUpgrade(UShooterSkillUpgrade* SkillUpgrade, APlayerCharacter* Character)
{
    if (!(IsValid(Character) && IsValid(SkillUpgrade))) return false;
    bool bAllowedToUpgrade = SkillUpgrade->CanUpgrade(Character) && !SkillUpgrade->IsAlreadyUpgraded(Character);
    if (!bAllowedToUpgrade) return false;
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

void USkillsProgressionWidget::PlayOnUpgradeSound(EStageProgress StageProgress)
{
    if ((ProgressState.IsValid() && !IsValid(SkillDescription))) return;

    ESkillUpgradeActionType UpgradeType = ESkillUpgradeActionType::SingleSkill;
    if (ProgressState->IsFullyUpgraded()) UpgradeType = ESkillUpgradeActionType::Fully;
    else if (StageProgress == EStageProgress::Completed) UpgradeType = ESkillUpgradeActionType::Group;

    USoundBase* UpgradeSound = SkillDescription->GetUpgradeActionSound(UpgradeType);
    if (!IsValid(UpgradeSound)) return;

    constexpr float PitchMultiplier = 1;
    constexpr float Volume = 1;
	UGameplayStatics::SpawnSound2D(GetWorld(), UpgradeSound, Volume, PitchMultiplier);
}
