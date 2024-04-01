# Reign Force

<p align="center">
  <a href="https://github.com/andrew1407/ReignForce/releases">
    <img src="./Doc/Resources/Icons/icon.ico" width="140" alt="Reign Force Logo" />
  </a>
</p>

## Description

This UE 5.3 prject is a third-person shooter game using reinforcement learning to train enemy AI. It is a practical part of the **[master's thesis](https://ela.kpi.ua/items/944163bd-9528-4faf-b506-f0f11145addc)**.

There's two game modes to handle reinforcement learning using **[AICommandsStrategy](./Source/ReignForce/Public/ReinforcementLearning/Interfaces/AICommandsStrategy.h)** interface to request a predicted command for enemy AI:

1. **[AILearningGameMode](./Source/ReignForce/Public/GameModes/AILearningGameMode.h)** uses WebSocket connection to train model on a server during gameplay collecting and operating gained episodic data experience bunches.

2. **[AIModelsGameMode](./Source/ReignForce/Public/GameModes/AIModelsGameMode.h)** loads and then connects to pre-trained model(s) of ONNX format (uses NNE plugin to load runtime models).

There are [three behavioral patterns](./Source/ReignForce/Public/ReinforcementLearning/TemperamentType.h) (temperament types) to estimate enemy AI action for a reward, and each of them has its own estimator for it:

- [Agressive](./Source/ReignForce/Private//ReinforcementLearning/RewardEstimators/RewardEstimatorAgressive.cpp)
- [Retreat](./Source/ReignForce/Private//ReinforcementLearning/RewardEstimators/RewardEstimatorRetreat.cpp)
- [Moderate](./Source/ReignForce/Private//ReinforcementLearning/RewardEstimators/RewardEstimatorModerate.cpp)

The game modes and temperament types can be switched and set up in Unreal Editor. Here's a scheme of **VLAD** (validate, learn, act, distinguish) system:

![Project scheme](./Doc/Resources/project-components.png)

To train the model the project is presented as a thick client using WebSocket protocol. It connects to the Python WebSocket server (see more [here](./TrainigServer/)).

## Scripts

The project contains three .bat scripts for Windows platforms for a process automation in the **[ScriptsWin](./ScriptsWin)** folder. All main configurable parameters are present in teh **[config.bat](./ScriptsWin/config.bat)** script.

1. To run project compilation use **[run.bat](./ScriptsWin/run.bat)**:

```batch
.\ScriptsWin\run.bat <mode>
```

or (from the ScriptsWin dir):

```batch
.\run.bat <mode>
```

It can take one optional argument for unreal run mode. By default it runs editor dev. build.

2. To run tests use **[test.bat](./ScriptsWin/test.bat)**:

```batch
.\ScriptsWin\test.bat <testname> <skipbuild>
```

or (from the ScriptsWin dir):

```batch
.\test.bat <testname> <skipbuild>
```

It can take one optional argument for unreal run mode:
+ Test context name; by default runs all; besides exact test name the script argument takes **"projectonly"** option to run ReignForce tests only (this tests are contained in the **[tests](./Source/ReignForce/Private/Tests/)** dir);
+ Skip build flag; by default accomplishes the build; by passing **"skipbuild"** trigger test only (without build) otherwise the script runs build before tests; 

3. To build game use **[build_game.bat](./ScriptsWin/build_game.bat)**:

```batch
.\ScriptsWin\build_game.bat <mode>
```

or (from the ScriptsWin dir):

```batch
.\build_game.bat <mode>
```

An optional flag here is a build mode which is set as **"Shipping"** by default; can be defined as **"Development"** as well.

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
