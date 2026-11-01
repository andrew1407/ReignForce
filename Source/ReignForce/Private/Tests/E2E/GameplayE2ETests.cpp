#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BrainComponent.h"

#include "Tests/TestUtils.h"
#include "Enemy/EnemyCharacter.h"
#include "Enemy/ShooterAIController.h"
#include "ShooterCharacter/Animation/ShooterAnimInstance.h"
#include "Weapons/Components/CombatSystem.h"


using namespace TestUtils;

namespace
{
    constexpr float RUN_SPEED_TO_JUMP = 250;
    constexpr double RUN_UP_TIMEOUT = 4;
    constexpr double AIR_TIMEOUT = 3;
    constexpr double DIAGONAL_RUN_SECONDS = .8;
    constexpr double AI_POSSESSION_TIMEOUT = 5;
    constexpr double AI_SIGHT_TIMEOUT = 10;
    constexpr float ENEMY_DISTANCE = 700;

    struct FDiagonal
    {
        const TCHAR* Name;
        float Forward;
        float Right;
    };

    // Forward & back pairs: the player returns near the start after each pair
    const TArray<FDiagonal> DIAGONALS = {
        { TEXT("ForwardRight"), 1, 1 },
        { TEXT("BackwardLeft"), -1, -1 },
        { TEXT("ForwardLeft"), 1, -1 },
        { TEXT("BackwardRight"), -1, 1 },
    };

    void MovePlayer(AShooterCharacter* Player, float Forward, float Right)
    {
        const FRotator ControlYaw(0, Player->GetControlRotation().Yaw, 0);
        Player->AddMovementInput(FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::X), Forward);
        Player->AddMovementInput(FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::Y), Right);
    }
}

BEGIN_DEFINE_SPEC(FGameplayE2ETests, "ReignForce.E2E.Gameplay",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    TWeakObjectPtr<AShooterCharacter> Player;
    TWeakObjectPtr<AEnemyCharacter> Enemy;
    FVector StartLocation;

    UShooterAnimInstance* GetPlayerAnimation() const
    {
        return Player.IsValid() && Player->GetMesh() ? Cast<UShooterAnimInstance>(Player->GetMesh()->GetAnimInstance()) : nullptr;
    }

    template <typename TValue>
    TValue GetAnimValue(const TCHAR* Name, TValue Default = TValue())
    {
        const TValue* Value = GetPropertyPtr<TValue>(GetPlayerAnimation(), Name);
        return Value ? *Value : Default;
    }

    /** Runs the diagonals one after another, checking the locomotion blend space input of each */
    void RunDiagonal(int32 Index, const FDoneDelegate& Done)
    {
        if (!DIAGONALS.IsValidIndex(Index) || !Player.IsValid()) return Done.Execute();
        const FDiagonal Diagonal = DIAGONALS[Index];
        // Every diagonal starts from the same spot (the test map floor is small, walls would bend the run)
        Player->TeleportTo(StartLocation, Player->GetActorRotation());
        Player->GetCharacterMovement()->StopMovementImmediately();
        const double StartTime = FPlatformTime::Seconds();
        const double Duration = DIAGONAL_RUN_SECONDS;

        FDoneDelegate OnRun = FDoneDelegate::CreateLambda([this, Index, Diagonal, Done]
        {
            const FVector2D Direction = GetAnimValue<FVector2D>(TEXT("MovementDirection"));
            const FVector2D Expected = Player.IsValid() ? UShooterAnimInstance::ToBlendSpaceDirection(Player->GetMovementDirection(), GetAnimValue<bool>(TEXT("bBackwardStrafes"))) : FVector2D::ZeroVector;
            const FString What = FString::Printf(TEXT("%s blend input %s (movement %s)"), Diagonal.Name, *Direction.ToString(), *Expected.ToString());
            // Both cardinal jogs of the quadrant take part (not a single one, nor idle)
            TestTrue(What + TEXT(" forward jog side"), FMath::Sign(Direction.X) == Diagonal.Forward && FMath::Abs(Direction.X) > .15f);
            TestTrue(What + TEXT(" right jog side"), FMath::Sign(Direction.Y) == Diagonal.Right && FMath::Abs(Direction.Y) > .15f);
            TestTrue(What + TEXT(" mapped from the movement like any other quadrant"), Direction.Equals(Expected, .1));
            TestTrue(What + TEXT(" jog weight"), FMath::Abs(Direction.X) + FMath::Abs(Direction.Y) > .5f);
            RunDiagonal(Index + 1, Done);
        });

        WaitUntil(OnRun, [WeakPlayer = Player, Diagonal, StartTime, Duration]
        {
            if (!WeakPlayer.IsValid()) return true;
            MovePlayer(WeakPlayer.Get(), Diagonal.Forward, Diagonal.Right);
            return FPlatformTime::Seconds() - StartTime >= Duration;
        }, Duration + 1);
    }

    void SpawnEnemyInFrontOfPlayer()
    {
        UWorld* World = GetGameWorld();
        if (!Player.IsValid()) return;
        const FVector PlayerLocation = Player->GetActorLocation();
        const FVector Location = PlayerLocation + Player->GetActorForwardVector() * ENEMY_DISTANCE + FVector(0, 0, 50);
        const FRotator FacingPlayer = (PlayerLocation - Location).GetSafeNormal2D().Rotation();
        Enemy = Cast<AEnemyCharacter>(SpawnBlueprintActor(World, ENEMY_BLUEPRINT_PATH, Location, FacingPlayer));
        // Same as the enemy spawner: the AI controller is created explicitly after spawning
        if (Enemy.IsValid() && !Enemy->GetController()) Enemy->SpawnDefaultController();
    }

END_DEFINE_SPEC(FGameplayE2ETests);

void FGameplayE2ETests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done)
    {
        FDoneDelegate OnOpened = FDoneDelegate::CreateLambda([this, Done]
        {
            Player = EnsurePlayerShooter(GetGameWorld());
            if (!TestTrue("Player character possessed on the test map", Player.IsValid())) return Done.Execute();
            StartLocation = FVector(0, 0, Player->GetActorLocation().Z);
            Player->TeleportTo(StartLocation, Player->GetActorRotation());
            Player->GetCharacterMovement()->StopMovementImmediately();
            // A freshly spawned player falls onto the floor & defines its stats first
            WaitUntil(Done, [WeakPlayer = Player]
            {
                return !WeakPlayer.IsValid() || (!WeakPlayer->GetCharacterMovement()->IsFalling() && WeakPlayer->GetHealthPoints() > 0);
            }, AI_POSSESSION_TIMEOUT);
        });
        OpenTestMap(*this, OnOpened);
    });

    AfterEach([this]
    {
        if (Enemy.IsValid()) Enemy->Destroy();
        Enemy.Reset();
        if (Player.IsValid() && IsValid(Player->GetCombatSystem())) Player->GetCombatSystem()->SetRotationLock(false);
    });

    Describe("PlayerLocomotion", [this]
    {
        It("UsesShooterAnimation", [this]
        {
            TestNotNull("Shooter anim instance", GetPlayerAnimation());
        });

        LatentIt("StopsRunningArmsInTheAir", [this](const FDoneDelegate& Done)
        {
            if (!TestNotNull("Shooter anim instance", GetPlayerAnimation())) return Done.Execute();

            FDoneDelegate OnLanded = FDoneDelegate::CreateLambda([this, Done]
            {
                TestFalse("Landed", Player.IsValid() && Player->GetCharacterMovement()->IsFalling());
                TestFalse("Animation left the air state", GetAnimValue<bool>(TEXT("bFalling"), true));
                Done.Execute();
            });

            FDoneDelegate OnInAir = FDoneDelegate::CreateLambda([this, OnLanded]
            {
                // The upper body holding poses switch to running ones by Speed: it has to drop in the air while still moving
                TestTrue("Animation in the air state", GetAnimValue<bool>(TEXT("bFalling")));
                TestEqual("No ground speed in the air (no running arms)", GetAnimValue<float>(TEXT("Speed"), -1), 0.f);
                TestTrue("Still moving horizontally (jump on the run)", GetAnimValue<float>(TEXT("SpeedXY")) > 10);

                WaitUntil(OnLanded, [WeakPlayer = Player]
                {
                    return !WeakPlayer.IsValid() || !WeakPlayer->GetCharacterMovement()->IsFalling();
                }, AIR_TIMEOUT);
            });

            FDoneDelegate OnRunning = FDoneDelegate::CreateLambda([this, OnInAir]
            {
                TestTrue("Ground speed while running", GetAnimValue<float>(TEXT("Speed")) > 10);
                if (!Player.IsValid()) return OnInAir.Execute();
                Player->Jump();
                WaitUntil(OnInAir, [WeakPlayer = Player]
                {
                    if (!WeakPlayer.IsValid()) return true;
                    MovePlayer(WeakPlayer.Get(), 1, 0);
                    const bool bFalling = WeakPlayer->GetCharacterMovement()->IsFalling();
                    // Checked after the anim update of the falling frame
                    const auto Animation = Cast<UShooterAnimInstance>(WeakPlayer->GetMesh()->GetAnimInstance());
                    const bool* bAnimFalling = GetPropertyPtr<bool>(Animation, TEXT("bFalling"));
                    return bFalling && bAnimFalling && *bAnimFalling;
                }, AIR_TIMEOUT);
            });

            WaitUntil(OnRunning, [WeakPlayer = Player]
            {
                if (!WeakPlayer.IsValid()) return true;
                MovePlayer(WeakPlayer.Get(), 1, 0);
                return WeakPlayer->GetVelocity().Size2D() > RUN_SPEED_TO_JUMP;
            }, RUN_UP_TIMEOUT);
        });

        LatentIt("BlendsAllDiagonalsAlike", [this](const FDoneDelegate& Done)
        {
            if (!TestNotNull("Shooter anim instance", GetPlayerAnimation())) return Done.Execute();
            // Aim focus keeps the character facing the camera, so moving diagonally strafes (the diagonal blend space case)
            Player->GetCombatSystem()->SetRotationLock(true);
            RunDiagonal(0, Done);
        });
    });

    Describe("EnemyAI", [this]
    {
        LatentIt("IsPossessedByShooterAI", [this](const FDoneDelegate& Done)
        {
            SpawnEnemyInFrontOfPlayer();
            if (!TestTrue("Enemy spawned", Enemy.IsValid())) return Done.Execute();

            FDoneDelegate OnPossessed = FDoneDelegate::CreateLambda([this, Done]
            {
                auto Controller = Enemy.IsValid() ? Enemy->GetController<AShooterAIController>() : nullptr;
                if (TestNotNull("Shooter AI controller", Controller))
                {
                    TestNotNull("Behavior running", Controller->GetBrainComponent());
                    TestNotNull("Blackboard", Controller->GetBlackboardComponent());
                    TestTrue("Enemy is in another team than the player", !Player.IsValid() || Enemy->GetGenericTeamId() != Player->GetGenericTeamId());
                }
                Done.Execute();
            });
            WaitUntil(OnPossessed, [WeakEnemy = Enemy]
            {
                return !WeakEnemy.IsValid() || WeakEnemy->GetController<AShooterAIController>() != nullptr;
            }, AI_POSSESSION_TIMEOUT);
        });

        LatentIt("SpotsThePlayer", [this](const FDoneDelegate& Done)
        {
            SpawnEnemyInFrontOfPlayer();
            if (!TestTrue("Enemy spawned", Enemy.IsValid())) return Done.Execute();

            FDoneDelegate OnSpotted = FDoneDelegate::CreateLambda([this, Done]
            {
                auto Controller = Enemy.IsValid() ? Enemy->GetController<AShooterAIController>() : nullptr;
                TestTrue("Enemy focused on the player by sight", Controller && Controller->GetTargetFocus() == Player.Get());
                Done.Execute();
            });
            WaitUntil(OnSpotted, [WeakEnemy = Enemy, WeakPlayer = Player]
            {
                if (!WeakEnemy.IsValid() || !WeakPlayer.IsValid()) return true;
                auto Controller = WeakEnemy->GetController<AShooterAIController>();
                return Controller && Controller->GetTargetFocus() == WeakPlayer.Get();
            }, AI_SIGHT_TIMEOUT);
        });

        LatentIt("StopsWhenKilledByThePlayer", [this](const FDoneDelegate& Done)
        {
            SpawnEnemyInFrontOfPlayer();
            if (!TestTrue("Enemy spawned", Enemy.IsValid())) return Done.Execute();

            FDoneDelegate OnSettled = FDoneDelegate::CreateLambda([this, Done]
            {
                if (!Enemy.IsValid() || !Player.IsValid()) return Done.Execute();
                UGameplayStatics::ApplyDamage(Enemy.Get(), Enemy->GetHealthPoints() * 2, Player->GetController(), Player.Get(), UDamageType::StaticClass());
                TestTrue("Enemy perished", Enemy->IsDead());
                TestFalse("Player unharmed", Player->IsDead());

                FDoneDelegate OnAfterDeath = FDoneDelegate::CreateLambda([this, Done]
                {
                    TestTrue("Dead enemy doesn't move", !Enemy.IsValid() || Enemy->GetCharacterMovement()->Velocity.Size2D() < 1);
                    TestTrue("Dead enemy stays dead", !Enemy.IsValid() || Enemy->IsDead());
                    Done.Execute();
                });
                WaitFor(OnAfterDeath, 1);
            });
            WaitUntil(OnSettled, [WeakEnemy = Enemy]
            {
                return !WeakEnemy.IsValid() || (WeakEnemy->GetHealthPoints() > 0 && !WeakEnemy->GetCharacterMovement()->IsFalling());
            }, AI_POSSESSION_TIMEOUT);
        });
    });
}

#endif
