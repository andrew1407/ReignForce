# Reign Force

<p align="center">
  <a href="https://github.com/andrew1407/ReignForce/releases">
    <img src="./Doc/Resources/Icons/icon.ico" width="140" alt="Reign Force Logo" />
  </a>
</p>

<p>
  <a href="https://www.unrealengine.com"><img src="https://img.shields.io/badge/Unreal%20Engine-5.8-313131?logo=unrealengine&logoColor=white" alt="Unreal Engine 5.8" /></a>
  <a href="https://github.com/Andrew1407/ReignForce/releases"><img src="https://img.shields.io/github/v/release/Andrew1407/ReignForce?label=release" alt="Release" /></a>
</p>

## Description

This UE 5.8 project is a third-person shooter game using reinforcement learning to train enemy AI. It is a practical part of the **[master's thesis](https://ela.kpi.ua/items/944163bd-9528-4faf-b506-f0f11145addc)**.

There's two game modes to handle reinforcement learning using **[AICommandsStrategy](./Source/ReignForce/Public/ReinforcementLearning/Interfaces/AICommandsStrategy.h)** interface to request a predicted command for enemy AI:

1. **[AILearningGameMode](./Source/ReignForce/Public/GameModes/AILearningGameMode.h)** uses WebSocket connection to train model on a server during gameplay collecting and operating gained episodic data experience bunches.

2. **[AIModelsGameMode](./Source/ReignForce/Public/GameModes/AIModelsGameMode.h)** loads and then connects to pre-trained model(s) of ONNX format (uses the Neural Network Engine (NNE) with the ONNX Runtime CPU backend to load runtime models).

There are [three behavioral patterns](./Source/ReignForce/Public/ReinforcementLearning/TemperamentType.h) (temperament types) to estimate enemy AI action for a reward, and each of them has its own estimator for it:

- [Agressive](./Source/ReignForce/Private//ReinforcementLearning/RewardEstimators/RewardEstimatorAgressive.cpp)
- [Retreat](./Source/ReignForce/Private//ReinforcementLearning/RewardEstimators/RewardEstimatorRetreat.cpp)
- [Moderate](./Source/ReignForce/Private//ReinforcementLearning/RewardEstimators/RewardEstimatorModerate.cpp)

The game modes and temperament types can be switched and set up in Unreal Editor. Here's a scheme of **VLAD** (validate, learn, act, distinguish) system:

![Project scheme](./Doc/Resources/project-components.png)

To train the model the project is presented as a thick client using WebSocket protocol. It connects to the Python WebSocket server (see more [here](./TrainingServer/)).

## Scripts

Process automation lives in the **[Scripts](./Scripts)** folder: a single cross-platform **[ue.py](./Scripts/ue.py)** (Windows, Linux, macOS) plus thin launchers that run it with the engine's bundled Python, so nothing extra has to be installed. The engine is detected from the `.uproject` `EngineAssociation` (Epic launcher manifest / registry / `Install.ini`); set the `UE_ROOT` env var or pass `--engine <dir>` to override it.

```batch
.\Scripts\ue.bat <command> [options]    :: Windows
```

```sh
./Scripts/ue.sh <command> [options]     # Linux / macOS
```

| Command | Description |
| --- | --- |
| `info` | Prints the detected project & engine. |
| `build [editor\|game] [-c CONFIG]` | Compiles the editor (default) or game target; config defaults to **Development**. |
| `editor` | Opens the project in Unreal Editor. |
| `test [FILTER] [--skip-build]` | Builds the editor target and runs automation tests headless. `FILTER`: **project** (default, ReignForce tests from the **[tests](./Source/ReignForce/Private/Tests/)** dir), **all**, or a test name prefix. Report & log go to `Build/Tests`; the exit code reflects failed tests. |
| `package [-c CONFIG]` | Builds, cooks, stages, paks and archives the game to `Build/Archived` (config defaults to **Shipping**). |
| `graph [NODE] [-c CONFIG]` | Runs the **[BuildGraph](./Scripts/BuildGraph.xml)** pipeline: `CompileEditor`, `CompileGame`, `Cook`, `Package` (default), `Test` or `All`. Completed nodes are reused on re-runs. |
| `python [ARGS...]` | Runs the engine's bundled Python. |

## Tests

Automation specs live in the **[tests](./Source/ReignForce/Private/Tests/)** dir (editor builds only), grouped by test name prefix so a level can be run alone (e.g. `.\Scripts\ue.bat test ReignForce.Unit`):

| Prefix | Scope |
| --- | --- |
| `ReignForce.Unit` | Pure logic without a world: reward estimators, AI command model inputs & prediction, shooter stats, skills progression ranks, upgrades balance, round difficulty, locomotion blend space mapping & diagonal strafes, gamepad mapping & prompt icons, projectile spread. |
| `ReignForce.Integration` | Gameplay objects in a PIE session of the test map: cooldown timers, character damage & death, weapon slots (weapons destroyed with their owner), AI metrics of a live character, firearms, settings menu (Apply enabled only on changes, stays open), level opening while paused (no endless loading) & music stop. |
| `ReignForce.E2E` | Gameplay scenarios on the test map: the player runs, jumps (no running arms in the air) and moves along every diagonal (blended alike), an enemy is spawned and checked to be AI possessed, to spot the player and to stop once killed by them; a round start on the main game map is timed for hitches. |

## Gameplay

### Main menu

Default (without game progress saves):

![Main menu new game](./Doc/Resources/Gameplay/main-menu-1.png)

Default (with game progress saves):

![Main menu game saves](./Doc/Resources/Gameplay/main-menu-2.png)

Exiting confirmation modal window:

![Main menu exit modal](./Doc/Resources/Gameplay/main-menu-exit.png)

Play the new game having previous game saves confirmation modal window:

![Main menu new game modal](./Doc/Resources/Gameplay/main-menu-new-game.png)

### Gameplay

Gameplay on a main map (melee attack):

![Melee attack 1](./Doc/Resources/Gameplay/attack-melee-1.png)

![Melee attack 2](./Doc/Resources/Gameplay/attack-melee-2.png)

Gameplay on a main map (firearms attack):

![Firearms attack 1](./Doc/Resources/Gameplay/attack-firearms-1.png)

![Firearms attack 2](./Doc/Resources/Gameplay/attack-firearms-2.png)

![Firearms attack 3](./Doc/Resources/Gameplay/attack-firearms-3.png)

Saving game during gameplay (success):

![Successful game save](./Doc/Resources/Gameplay/gameplay-save-success.png)

Saving game during round, when it's started (failure):

![Successful game save](./Doc/Resources/Gameplay/gameplay-save-during-round.png)

### Skills (progression) menu

![Skills progression 1](./Doc/Resources/Gameplay/skills-1.png)

![Skills progression 2](./Doc/Resources/Gameplay/skills-2.png)

![Skills progression 3](./Doc/Resources/Gameplay/skills-3.png)

### Pause menu

Pause menu during main game:

![Pause menu](./Doc/Resources/Gameplay/pause-menu.png)

Save progress connfirmation modal window:

![Pause menu save progress](./Doc/Resources/Gameplay/pause-menu-save.png)

Save progress (success):

![Pause menu save progress success](./Doc/Resources/Gameplay/pause-menu-save-success.png)

Save progress (failure):

![Pause menu save progress success](./Doc/Resources/Gameplay/pause-menu-save-failure.png)

Restart level connfirmation modal window:

![Restart level](./Doc/Resources/Gameplay/pause-menu-restart.png)

Go to the main menu connfirmation modal window:

![Go to the main menu](./Doc/Resources/Gameplay/pause-menu-go-to-main.png)

Exiting game connfirmation modal window:

![Exiting game](./Doc/Resources/Gameplay/pause-menu-exit.png)

Exiting game and saving a progress connfirmation modal window:

![Exiting game and saving a progress](./Doc/Resources/Gameplay/pause-menu-save-exit.png)
