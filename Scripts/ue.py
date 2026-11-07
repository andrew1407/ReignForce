#!/usr/bin/env python3
"""Cross-platform Unreal project automation (Windows / Linux / macOS).

The project (*.uproject) is detected from this script's parent dir, the engine
from the uproject's EngineAssociation (or the UE_ROOT env var / --engine flag).
Uses only the standard library, so the engine's bundled Python runs it as is.

Usage:
    ue.py info                                 show detected project/engine
    ue.py build [editor|game] [-c CONFIG]      compile a target (default: editor Development)
          [--if-needed]                        skip when the editor module is up to date
    ue.py editor                               open the project in the editor
    ue.py test [FILTER] [--build MODE]         run automation tests headless, compiling the editor
                                               first only if needed (MODE: auto | always | never)
                                               (FILTER: project (default) | all | <test name prefix>)
    ue.py package [-c CONFIG]                  build + cook + stage + pak + archive (default: Shipping)
    ue.py graph [NODE] [-c CONFIG]             run Scripts/BuildGraph.xml (default node: Package)
    ue.py python [ARGS...]                     run the engine's bundled Python
"""

import argparse
import configparser
import json
import os
import platform
import re
import subprocess
import sys
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
PROJECT_DIR = SCRIPTS_DIR.parent

HOST = {"Windows": "Win64", "Linux": "Linux", "Darwin": "Mac"}[platform.system()]
COOK_PLATFORM = {"Win64": "Windows", "Linux": "Linux", "Mac": "Mac"}[HOST]
IS_WIN = HOST == "Win64"


def fail(message):
    print(f"error: {message}", file=sys.stderr)
    sys.exit(1)


# region Project & engine discovery

def find_project():
    projects = sorted(PROJECT_DIR.glob("*.uproject"))
    if not projects:
        fail(f"no .uproject found in {PROJECT_DIR}")
    return projects[0]


def engine_candidates(association):
    """Yields possible engine roots for an EngineAssociation (e.g. "5.8" or a source build GUID)."""
    if os.environ.get("UE_ROOT"):
        yield Path(os.environ["UE_ROOT"])

    home = Path.home()
    launcher_manifests = {
        "Win64": [Path(os.environ.get("ProgramData", "C:/ProgramData")) / "Epic/UnrealEngineLauncher/LauncherInstalled.dat"],
        "Mac": [home / "Library/Application Support/Epic/UnrealEngineLauncher/LauncherInstalled.dat"],
        "Linux": [],
    }[HOST]
    for manifest in launcher_manifests:
        try:
            for entry in json.loads(manifest.read_text(encoding="utf-8"))["InstallationList"]:
                if entry.get("AppName") == f"UE_{association}":
                    yield Path(entry["InstallLocation"])
        except (OSError, ValueError, KeyError):
            pass

    if IS_WIN:
        import winreg
        keys = [
            (winreg.HKEY_LOCAL_MACHINE, rf"SOFTWARE\EpicGames\Unreal Engine\{association}", "InstalledDirectory"),
            (winreg.HKEY_CURRENT_USER, r"Software\Epic Games\Unreal Engine\Builds", association),
        ]
        for hive, key, value in keys:
            try:
                with winreg.OpenKey(hive, key) as handle:
                    yield Path(winreg.QueryValueEx(handle, value)[0])
            except OSError:
                pass
        yield Path(os.environ.get("ProgramFiles", "C:/Program Files")) / f"Epic Games/UE_{association}"
    else:
        # Source/registered builds on Linux & macOS
        ini_dir = home / (".config/Epic" if HOST == "Linux" else "Library/Application Support/Epic")
        install_ini = configparser.ConfigParser(strict=False)
        install_ini.optionxform = str
        if install_ini.read(ini_dir / "UnrealEngine/Install.ini") and install_ini.has_option("Installations", association):
            yield Path(install_ini.get("Installations", association))
        if HOST == "Mac":
            yield Path(f"/Users/Shared/Epic Games/UE_{association}")


def find_engine(project, override=None):
    association = json.loads(project.read_text(encoding="utf-8")).get("EngineAssociation", "")
    candidates = [Path(override)] if override else engine_candidates(association)
    for root in candidates:
        if (root / "Engine/Build/BatchFiles").is_dir():
            return root.resolve()
    fail(f"Unreal Engine '{association}' not found; set UE_ROOT or pass --engine")

# endregion


class Unreal:
    def __init__(self, engine_root, project):
        self.root = engine_root
        self.project = project
        self.name = project.stem
        batch = engine_root / "Engine/Build/BatchFiles"
        self.uat = batch / ("RunUAT.bat" if IS_WIN else "RunUAT.sh")
        self.ubt = batch / ("Build.bat" if IS_WIN else f"{HOST}/Build.sh")
        binaries = engine_root / "Engine/Binaries" / HOST
        self.editor = {
            "Win64": binaries / "UnrealEditor.exe",
            "Linux": binaries / "UnrealEditor",
            "Mac": binaries / "UnrealEditor.app/Contents/MacOS/UnrealEditor",
        }[HOST]
        # Console editor build on Windows: blocks the shell and returns a proper exit code
        self.editor_cmd = binaries / "UnrealEditor-Cmd.exe" if IS_WIN else self.editor
        python_dir = engine_root / "Engine/Binaries/ThirdParty/Python3" / HOST
        self.python = python_dir / "python.exe" if IS_WIN else python_dir / "bin/python3"

    def run(self, *command):
        command = [str(part) for part in command]
        print(">", subprocess.list2cmdline(command) if IS_WIN else " ".join(command), flush=True)
        return subprocess.call(command)

    def build(self, editor=True, config="Development"):
        target = f"{self.name}Editor" if editor else self.name
        return self.run(self.ubt, target, HOST, config, f"-project={self.project}", "-waitmutex")

    def editor_build_outdated(self):
        """True when the project editor module is missing or older than any of its sources."""
        module = {
            "Win64": f"UnrealEditor-{self.name}.dll",
            "Linux": f"libUnrealEditor-{self.name}.so",
            "Mac": f"UnrealEditor-{self.name}.dylib",
        }[HOST]
        binary = PROJECT_DIR / "Binaries" / HOST / module
        if not binary.exists():
            return True
        built_at = binary.stat().st_mtime
        sources = [self.project, *(PROJECT_DIR / "Source").rglob("*"), *(PROJECT_DIR / "Plugins").glob("*/Source/**/*")]
        return any(path.is_file() and path.stat().st_mtime > built_at for path in sources)

    def build_editor_if_needed(self):
        if not self.editor_build_outdated():
            print("editor build is up to date, skipping compilation")
            return 0
        return self.build(editor=True)

    def open_editor(self):
        subprocess.Popen([str(self.editor), str(self.project)])
        return 0

    def test(self, test_filter="project", build_mode="auto"):
        if build_mode != "never":
            status = self.build(editor=True) if build_mode == "always" else self.build_editor_if_needed()
            if status:
                return status

        test_scope = {"project": f"RunTests {self.name}", "all": "RunAll"}.get(test_filter, f"RunTests {test_filter}")
        report_dir = PROJECT_DIR / "Build/Tests"
        report_dir.mkdir(parents=True, exist_ok=True)
        index = report_dir / "index.json"
        index.unlink(missing_ok=True)

        status = self.run(
            self.editor_cmd, self.project,
            f"-ExecCmds=Automation {test_scope}", "-TestExit=Automation Test Queue Empty",
            f"-ReportExportPath={report_dir}", f"-abslog={report_dir / 'Tests.log'}",
            "-unattended", "-nopause", "-nullrhi", "-nosplash", "-nosound", "-stdout", "-log",
        )
        # The editor exit code alone doesn't reflect test failures, the exported report does
        try:
            report = json.loads(index.read_text(encoding="utf-8-sig"))
        except (OSError, ValueError):
            print(f"no test report written to {index}", file=sys.stderr)
            return status or 1
        print(f"tests: {report.get('succeeded', 0)} passed, {report.get('succeededWithWarnings', 0)} with warnings, "
              f"{report.get('failed', 0)} failed, {report.get('notRun', 0)} not run")
        return 1 if report.get("failed", 0) else status

    def package(self, config="Shipping"):
        # One BuildCookRun call builds the game & editor targets itself, no separate compile needed
        return self.run(
            self.uat, "BuildCookRun", f"-project={self.project}", f"-platform={HOST}", f"-clientconfig={config}",
            "-build", "-cook", "-stage", "-pak", "-archive", "-allmaps", "-utf8output",
            f"-archivedirectory={PROJECT_DIR / 'Build/Archived'}", f"-stagingdirectory={PROJECT_DIR / 'Build/Stage'}",
        )

    def graph(self, node="Package", config="Shipping"):
        return self.run(
            self.uat, "BuildGraph", f"-script={SCRIPTS_DIR / 'BuildGraph.xml'}", f"-target={node}",
            f"-set:ProjectFile={self.project}", f"-set:ProjectName={self.name}", f"-set:ProjectDir={PROJECT_DIR}",
            f"-set:Platform={HOST}", f"-set:CookPlatform={COOK_PLATFORM}", f"-set:Config={config}",
            f"-set:EditorCmd={self.editor_cmd}", "-utf8output",
        )


def check_assets():
    manifest = json.loads((PROJECT_DIR / "ThirdPartyAssets.json").read_text(encoding="utf-8"))
    missing = 0
    for pack in manifest["packs"]:
        present = any((PROJECT_DIR / pack["dir"]).glob("**/*.uasset"))
        missing += not present
        print(f"[{'ok' if present else 'MISSING'}] {pack['dir']}: {pack['name']} ({pack['source']})")
    if missing:
        print(f"\n{missing} pack(s) missing. {manifest['description']}")
    return 1 if missing else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--engine", help="engine root dir (overrides auto-detection & UE_ROOT)")
    commands = parser.add_subparsers(dest="command", required=True)

    commands.add_parser("info")
    commands.add_parser("editor")

    build = commands.add_parser("build")
    build.add_argument("target", nargs="?", default="editor", choices=["editor", "game"])
    build.add_argument("-c", "--config", default="Development", choices=["Debug", "DebugGame", "Development", "Shipping"])
    build.add_argument("--if-needed", action="store_true", help="editor only: skip when the module binary is up to date")

    test = commands.add_parser("test")
    test.add_argument("filter", nargs="?", default="project")
    test.add_argument("--build", dest="build_mode", default="auto", choices=["auto", "always", "never"],
                      help="auto (default): compile only when the editor module is missing or outdated")
    test.add_argument("--skip-build", dest="build_mode", action="store_const", const="never", help="same as --build never")

    package = commands.add_parser("package")
    package.add_argument("-c", "--config", default="Shipping", choices=["DebugGame", "Development", "Shipping"])

    graph = commands.add_parser("graph")
    graph.add_argument("node", nargs="?", default="Package", choices=["CompileEditor", "CompileGame", "Cook", "Package", "Test", "All"])
    graph.add_argument("-c", "--config", default="Shipping", choices=["DebugGame", "Development", "Shipping"])

    python = commands.add_parser("python")
    python.add_argument("args", nargs=argparse.REMAINDER)

    commands.add_parser("assets", help="lists the third-party content packs (ThirdPartyAssets.json) missing from Content")

    args = parser.parse_args()
    if args.command == "assets":
        return check_assets()
    project = find_project()
    unreal = Unreal(find_engine(project, args.engine), project)

    if args.command == "info":
        print(f"project: {unreal.project}\nengine:  {unreal.root}\nhost:    {HOST}")
        return 0
    if args.command == "editor":
        return unreal.open_editor()
    if args.command == "build":
        if args.if_needed and args.target == "editor" and args.config == "Development":
            return unreal.build_editor_if_needed()
        return unreal.build(editor=args.target == "editor", config=args.config)
    if args.command == "test":
        return unreal.test(args.filter, args.build_mode)
    if args.command == "package":
        return unreal.package(args.config)
    if args.command == "graph":
        return unreal.graph(args.node, args.config)
    if args.command == "python":
        return unreal.run(unreal.python, *args.args)


if __name__ == "__main__":
    sys.exit(main())
