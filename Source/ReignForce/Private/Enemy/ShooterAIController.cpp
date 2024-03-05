// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/ShooterAIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "Navigation/CrowdFollowingComponent.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AIPerceptionSystem.h"
#include "Perception/AIPerceptionComponent.h"

#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"

#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/TeamID.h"

#include "Weapons/Types/Weapon.h"
#include "ReinforcementLearning/Interfaces/AICommandsStrategy.h"


AShooterAIController::AShooterAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    Blackboard = CreateDefaultSubobject<UBlackboardComponent>(GET_MEMBER_NAME_CHECKED(AShooterAIController, Blackboard));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(GET_MEMBER_NAME_CHECKED(AShooterAIController, BehaviorTreeComponent));

    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(GET_MEMBER_NAME_CHECKED(AShooterAIController, PerceptionComponent));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(GET_MEMBER_NAME_CHECKED(AShooterAIController, SightConfig));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(GET_MEMBER_NAME_CHECKED(AShooterAIController, HearingConfig));
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(GET_MEMBER_NAME_CHECKED(AShooterAIController, DamageConfig));

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->ConfigureSense(*DamageConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    SightConfig->SightRadius = 1500;
    SightConfig->LoseSightRadius = 2000;
    SightConfig->PeripheralVisionAngleDegrees = 45;
    SightConfig->SetMaxAge(3);
    SightConfig->PointOfViewBackwardOffset = 0;

    HearingConfig->HearingRange = 2000;
    HearingConfig->SetMaxAge(3);

    DamageConfig->SetMaxAge(5);

    FocusedTargetKey = TEXT("FocusedTarget");
    PositionToInvestigateKey = TEXT("InvestigatablePosition");
    MemoizedSenseKey = TEXT("MemoizedSense");
    IsTargetVisibleKey = TEXT("IsTargetVisible");
    LastSeenTargetPositionKey = TEXT("LastSeenTargetPosition");
    CombatCommandKey = TEXT("CombatCommand");

    IgnoreAllyDistanceHearing = 800;
    TemperamentType = ETemperamentType::Moderate;

    SenseHandlers =
    {
        { UAISense_Sight::StaticClass(), FSenseHandle::CreateUObject(this, &AShooterAIController::ReactToSight) },
        { UAISense_Hearing::StaticClass(), FSenseHandle::CreateUObject(this, &AShooterAIController::ReactToHearing) },
        { UAISense_Damage::StaticClass(), FSenseHandle::CreateUObject(this, &AShooterAIController::ReactToDamage) },
    };

    OnTargetVisibilityChanged.AddDynamic(this, &AShooterAIController::TriggerAICommandsFetch);
    OnTargetVisibilityChanged.AddDynamic(this, &AShooterAIController::HandleShooterTargetVisibility);
}

void AShooterAIController::BeginPlay()
{
    Super::BeginPlay();

    if (IsValid(PerceptionComponent))
        PerceptionComponent->OnTargetPerceptionInfoUpdated.AddDynamic(this, &AShooterAIController::OnPerceptionInfoUpdated);
}

void AShooterAIController::BeginDestroy()
{
    if (IsValid(this) && IsValid(GetWorld()))
    {
        if (SightTimerHandle.IsValid() && GetWorldTimerManager().IsTimerActive(SightTimerHandle))
            GetWorldTimerManager().ClearTimer(SightTimerHandle);

        if (AICommandTimerHandle.IsValid() && GetWorldTimerManager().IsTimerActive(AICommandTimerHandle))
            GetWorldTimerManager().ClearTimer(AICommandTimerHandle);
    }

    Super::BeginDestroy();
}

FGenericTeamId AShooterAIController::GetGenericTeamId() const
{
    if (auto TeamAgent = Cast<IGenericTeamAgentInterface>(GetPawn()))
        return TeamAgent->GetGenericTeamId();
    return static_cast<uint8>(ETeamID::None);
}

EShooterAICommand AShooterAIController::GetShooterAICommand() const
{
    if (!IsValid(Blackboard) || CombatCommandKey.IsNone()) return EShooterAICommand::None;
    return static_cast<EShooterAICommand>(Blackboard->GetValueAsEnum(CombatCommandKey));
}

void AShooterAIController::SetShooterAICommand(EShooterAICommand Command)
{
    if (!IsValid(Blackboard) || CombatCommandKey.IsNone()) return;
    const EShooterAICommand PreviousCommand = GetShooterAICommand();
    Blackboard->SetValueAsEnum(CombatCommandKey, static_cast<uint8>(Command));
    if (!PossessedShooter.IsValid()) return;
    OnShooterAICommandChanged.Broadcast(Command, PreviousCommand, PossessedShooter.Get());
}

void AShooterAIController::StopCommandsFetchTimer()
{
    bool bTimerRunning = GetWorldTimerManager().IsTimerActive(AICommandTimerHandle);
    if (bTimerRunning) GetWorldTimerManager().ClearTimer(AICommandTimerHandle);
}

AActor* AShooterAIController::GetTargetFocus() const
{
    if (!IsValid(Blackboard) || FocusedTargetKey.IsNone()) return nullptr;
    UObject* Target = Blackboard->GetValueAsObject(FocusedTargetKey);
    return Cast<AActor>(Target);
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    PossessedShooter = Cast<AShooterCharacter>(GetPawn());
    if (PossessedShooter.IsValid())
    {
        PossessedShooter->OnPerish.AddDynamic(this, &AShooterAIController::HandleShooterPerish);
        PossessedShooter->OnDamageApplied.AddDynamic(this, &AShooterAIController::HandleAppliedDamage);
    }

    WeaponNoiseTagToIgnore = TEXT("Melee.");
    WeaponNoiseTagToIgnore += GetNameSafe(InPawn);

    if (IsValid(PerceptionComponent)) PerceptionComponent->RequestStimuliListenerUpdate();
    if (!IsValid(BehaviorTree)) return;
    if (IsValid(Blackboard)) Blackboard->InitializeBlackboard(*BehaviorTree->GetBlackboardAsset());
    if (IsValid(BehaviorTreeComponent)) BehaviorTreeComponent->StartTree(*BehaviorTree);
}

void AShooterAIController::OnUnPossess()
{
    Super::OnUnPossess();

    if (PossessedShooter.IsValid())
    {
        PossessedShooter->OnPerish.RemoveDynamic(this, &AShooterAIController::HandleShooterPerish);
        PossessedShooter->OnDamageApplied.RemoveDynamic(this, &AShooterAIController::HandleAppliedDamage);
    }

    StopCommandsFetchTimer();

    if (IsValid(this) && IsValid(GetWorld()))
    {
        if (SightTimerHandle.IsValid() && GetWorldTimerManager().IsTimerActive(SightTimerHandle))
            GetWorldTimerManager().ClearTimer(SightTimerHandle);

        if (AICommandTimerHandle.IsValid() && GetWorldTimerManager().IsTimerActive(AICommandTimerHandle))
            GetWorldTimerManager().ClearTimer(AICommandTimerHandle);
    }

    PossessedShooter = nullptr;
}

ETeamAttitude::Type AShooterAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    auto ControlledTeamAgent = Cast<IGenericTeamAgentInterface>(GetPawn());
    if (!ControlledTeamAgent) return ETeamAttitude::Type::Neutral;

    auto OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
    if (!OtherTeamAgent) return ETeamAttitude::Type::Neutral;
    
    int32 ComtrolledTeamId = ControlledTeamAgent->GetGenericTeamId().GetId();
    int32 OtherTeamId = OtherTeamAgent->GetGenericTeamId().GetId();
    bool bSameTeam = ComtrolledTeamId == OtherTeamId;
    return bSameTeam ? ETeamAttitude::Type::Friendly : ETeamAttitude::Type::Hostile;
}

void AShooterAIController::SetTargetVisibility(bool bState)
{
    if (!IsValid(Blackboard) || IsTargetVisibleKey.IsNone()) return;
    bool bVisible = Blackboard->GetValueAsBool(IsTargetVisibleKey);
    if (bVisible == bState) return;

    if (bState) Blackboard->SetValueAsBool(IsTargetVisibleKey, bState);
    else Blackboard->ClearValue(IsTargetVisibleKey);
    OnTargetVisibilityChanged.Broadcast(bState, PossessedShooter.Get(), Cast<AActor>(Blackboard->GetValueAsObject(FocusedTargetKey)));
}

void AShooterAIController::ResetTargetFocus(AActor* Target, bool bApplyToTargetVisibility)
{
    if (!IsValid(this) || !IsValid(Blackboard) || FocusedTargetKey.IsNone()) return;
    UObject* TargetState = Blackboard->GetValueAsObject(FocusedTargetKey);
    if (Target == TargetState) return;

    if (Target)
    {
        Blackboard->SetValueAsObject(FocusedTargetKey, Target);
    }
    else
    {
        Blackboard->ClearValue(FocusedTargetKey);
        if (!LastSeenTargetPositionKey.IsNone()) Blackboard->ClearValue(LastSeenTargetPositionKey);
        SetShooterAICommand(EShooterAICommand::None);
    }

    if (bApplyToTargetVisibility) SetTargetVisibility(!!Target);
    OnTargetFocusChanged.Broadcast(PossessedShooter.Get(), Target);
}

void AShooterAIController::OnPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    if (!PossessedShooter.IsValid() || PossessedShooter->IsDead()) return;

    const FAIStimulus& Stimulus = UpdateInfo.Stimulus;
    bool bSelfSource = UpdateInfo.Target == GetPawn();
    if (bSelfSource) return;
    TSubclassOf<UAISense> SenseType = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);

    if (FSenseHandle* SenseHandle = SenseHandlers.Find(SenseType))
        SenseHandle->ExecuteIfBound(UpdateInfo);
}

void AShooterAIController::TriggerAICommandsFetch(bool bState, AShooterCharacter* ShooterSelf, AActor* Target)
{
    FTimerManager& TimerManager = GetWorldTimerManager();
    bool bTimerRunning = TimerManager.IsTimerActive(AICommandTimerHandle);
    if (!bState && bTimerRunning) return TimerManager.ClearTimer(AICommandTimerHandle);
    if (bState && !bTimerRunning)
    {
        AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
        bool bAiStrategy = UKismetSystemLibrary::DoesImplementInterface(GameMode, UAICommandsStrategy::StaticClass());
        if (!bAiStrategy) return;
        const float Frequency = IAICommandsStrategy::Execute_GetRequestFrequency(GameMode);
        TimerManager.SetTimer(AICommandTimerHandle, [this, GameMode, ShooterSelf]
        {
            IAICommandsStrategy::Execute_RequestCommand(GameMode, ShooterSelf);
        }, Frequency, true, 0.0f);
    }
}

void AShooterAIController::HandleShooterTargetVisibility(bool bState, AShooterCharacter* ShooterSelf, AActor* Target)
{
    if (!IsValid(ShooterSelf) || bState) return;
    ShooterSelf->StopAttacking();
}

void AShooterAIController::HandleShooterPerish(AShooterCharacter* ShooterSelf, AActor* Cause)
{
    StopCommandsFetchTimer();
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    bool bAiStrategy = UKismetSystemLibrary::DoesImplementInterface(GameMode, UAICommandsStrategy::StaticClass());
    if (bAiStrategy) IAICommandsStrategy::Execute_RequestCommand(GameMode, ShooterSelf);
}

void AShooterAIController::HandleAppliedDamage(float Damage, AShooterCharacter* ShooterSelf, AActor* HitTaret, bool IsLethal)
{
    bool bTriggerOnKillAction = IsLethal && HitTaret->IsA(AShooterCharacter::StaticClass());
    if (bTriggerOnKillAction)
    {
        if (auto TeamAgent = Cast<IGenericTeamAgentInterface>(HitTaret))
        {
            bool bSameTeam = ShooterSelf->GetGenericTeamId() == TeamAgent->GetGenericTeamId();
            if (bSameTeam) return;
        }

        StopCommandsFetchTimer();

        AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
        bool bAiStrategy = UKismetSystemLibrary::DoesImplementInterface(GameMode, UAICommandsStrategy::StaticClass());
        if (bAiStrategy) IAICommandsStrategy::Execute_RequestCommand(GameMode, ShooterSelf);

        GetWorldTimerManager().SetTimerForNextTick([this]
        {
            constexpr float CleanupDelay = .5f;
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, [this] { ResetTargetFocus(nullptr); }, CleanupDelay, false);
        });
    }
}

void AShooterAIController::ReactToSight(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    AActor* Target = UpdateInfo.Target.Get();
    auto Shooter = Cast<AShooterCharacter>(Target);
    if (IsValid(Shooter) && Shooter->IsDead()) return;
    if (!IsValid(Blackboard) || FocusedTargetKey.IsNone()) return;

    const FAIStimulus& Stimulus = UpdateInfo.Stimulus;
    FTimerManager& TimerManager = GetWorldTimerManager();
    if (Stimulus.WasSuccessfullySensed())
    {
        if (PossessedShooter.IsValid()) PossessedShooter->ReactToSeen();
        UObject* TargetFocus = Blackboard->GetValueAsObject(FocusedTargetKey);
        bool bTargetFocusValid = IsValid(TargetFocus);
        if (bTargetFocusValid && TargetFocus != Target) return;

        TimerManager.ClearTimer(SightTimerHandle);
        if (bTargetFocusValid) SetTargetVisibility(true);
        else ResetTargetFocus(Target);

        if (!LastSeenTargetPositionKey.IsNone()) Blackboard->ClearValue(LastSeenTargetPositionKey);
    }
    else
    {
        SetTargetVisibility(false);
        if (!LastSeenTargetPositionKey.IsNone()) Blackboard->SetValueAsVector(LastSeenTargetPositionKey, Target->GetActorLocation());
        auto ClearFocus = [this] { ResetTargetFocus(nullptr, false); };
        TimerManager.SetTimer(SightTimerHandle, ClearFocus, SightConfig->GetMaxAge(), false);
        if (PossessedShooter.IsValid()) PossessedShooter->ReactToLostSight();
    }
}

void AShooterAIController::ReactToHearing(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    if (!IsValid(Blackboard) || PositionToInvestigateKey.IsNone() || MemoizedSenseKey.IsNone() || FocusedTargetKey.IsNone()) return;

    const FAIStimulus& Stimulus = UpdateInfo.Stimulus;
    const FString TagStr = Stimulus.Tag.ToString();
    bool bShouldIgnore = TagStr.EndsWith(WeaponNoiseTagToIgnore);
    if (bShouldIgnore) return;

    AActor* Target = UpdateInfo.Target.Get();
    AActor* ToCheck = Target->IsA(AWeapon::StaticClass()) ? Target->GetAttachParentActor() : Target;
    bool bSameInstance = IsValid(ToCheck) && ToCheck->IsA(PossessedShooter->GetClass());
    if (bSameInstance)
    {
        const float Distance = FVector::Dist(Target->GetActorLocation(), PossessedShooter->GetActorLocation());
        if (Distance < IgnoreAllyDistanceHearing) return;
    }

    UClass* HearingSenseClass = UAISense_Hearing::StaticClass();
    UClass* MemoizedSense = Blackboard->GetValueAsClass(MemoizedSenseKey);
    bool bNotHearingSense = MemoizedSense != HearingSenseClass;

    if (Stimulus.WasSuccessfullySensed())
    {
        UObject* TargetFocus = Blackboard->GetValueAsObject(FocusedTargetKey);
        bool bTargetValid = IsValid(TargetFocus);
        bool bShouldReact = !bTargetValid && bNotHearingSense;

        if (bShouldReact && PossessedShooter.IsValid()) PossessedShooter->ReactToHeard();

        if (bTargetValid && UpdateInfo.Target == TargetFocus) return;
        Blackboard->SetValueAsVector(PositionToInvestigateKey, Stimulus.StimulusLocation);
        Blackboard->SetValueAsClass(MemoizedSenseKey, HearingSenseClass);
    }
    else
    {
        if (bNotHearingSense) return;
        Blackboard->ClearValue(PositionToInvestigateKey);
        Blackboard->ClearValue(MemoizedSenseKey);
    }
}

void AShooterAIController::ReactToDamage(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    if (!IsValid(Blackboard) || PositionToInvestigateKey.IsNone() || MemoizedSenseKey.IsNone() || FocusedTargetKey.IsNone()) return;

    const FAIStimulus& Stimulus = UpdateInfo.Stimulus;
    UClass* DamageSenseClass = UAISense_Damage::StaticClass();
    UClass* MemoizedSense = Blackboard->GetValueAsClass(MemoizedSenseKey);
    if (Stimulus.WasSuccessfullySensed())
    {
        UObject* TargetFocus = Blackboard->GetValueAsObject(FocusedTargetKey);
        if (IsValid(TargetFocus))
        {
            if (UpdateInfo.Target == TargetFocus) return;
            bool bWepon = UpdateInfo.Target->IsA(AWeapon::StaticClass());
            if (bWepon && UpdateInfo.Target->GetAttachParentActor() == TargetFocus) return;
        }

        Blackboard->SetValueAsVector(PositionToInvestigateKey, UpdateInfo.Target->GetActorLocation());
        Blackboard->SetValueAsClass(MemoizedSenseKey, DamageSenseClass);
    }
    else
    {
        if (MemoizedSense != DamageSenseClass) return;
        Blackboard->ClearValue(PositionToInvestigateKey);
        Blackboard->ClearValue(MemoizedSenseKey);
    }
}
