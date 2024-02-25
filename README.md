# Reign Force

## Description

This UE 5.3 prject is a third-person shooter game using reinforcement learning to train enemy AI. 

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
