PYTHON ?= python3
ORCHESTRATOR := $(PYTHON) tools/build/orchestrate.py
LINKER_INCREMENTAL := $(PYTHON) tools/linker/incremental.py
CODEWARRIOR_DRIVER := $(PYTHON) tools/linker/codewarrior_driver.py
ARM9I_SDK_PREPARE := $(PYTHON) tools/linker/arm9i_sdk_prepare.py
ARM9_SOURCE_PROVIDER := $(PYTHON) tools/linker/source_provider.py

ROM ?= $(FSAE_ROM)
BIOS7 ?= $(FSAE_BIOS7)
TWLTOOL ?= $(TWLTOOL_ZIP)
TWLSDK ?= $(TWLSDK_ROOT)

WINDOWS_HOST ?= $(FSAE_WINDOWS_HOST)
WINDOWS_WORKSPACE ?= $(FSAE_WINDOWS_WORKSPACE)
WINDOWS_TOOLCHAIN ?= $(FSAE_WINDOWS_TOOLCHAIN_ROOT)
WINDOWS_ENVIRONMENT ?= $(FSAE_WINDOWS_ENVIRONMENT_SCRIPT)
WINDOWS_COMMAND ?= $(FSAE_WINDOWS_BUILD_COMMAND)

LINKER_SDK_UNITS ?= $(FSAE_LINKER_SDK_UNITS)
ARM_OBJECT_COMPILER ?= $(FSAE_ARM_OBJECT_COMPILER)
MAKELCF ?= $(FSAE_MAKELCF)
MWCCARM ?= $(FSAE_MWCCARM)
MWLDARM ?= $(FSAE_MWLDARM)
ARM9_SOURCE_PROMOTIONS ?= $(if $(FSAE_ARM9_SOURCE_PROMOTIONS),$(FSAE_ARM9_SOURCE_PROMOTIONS),config/linker/arm9-source-promotions.v1.json)
ARM9_SOURCE_CANDIDATES ?= build/linker/source-provider/candidates

PRIVATE_INPUT_ENV = FSAE_ROM="$(ROM)" FSAE_BIOS7="$(BIOS7)" TWLTOOL_ZIP="$(TWLTOOL)"
WINDOWS_ENV = FSAE_WINDOWS_HOST="$(WINDOWS_HOST)" \
	FSAE_WINDOWS_WORKSPACE="$(WINDOWS_WORKSPACE)" \
	FSAE_WINDOWS_TOOLCHAIN_ROOT="$(WINDOWS_TOOLCHAIN)" \
	FSAE_WINDOWS_ENVIRONMENT_SCRIPT="$(WINDOWS_ENVIRONMENT)" \
	FSAE_WINDOWS_BUILD_COMMAND="$(WINDOWS_COMMAND)"
LINKER_ENV = TWLSDK_ROOT="$(TWLSDK)" \
	FSAE_LINKER_SDK_UNITS="$(LINKER_SDK_UNITS)" \
	FSAE_ARM_OBJECT_COMPILER="$(ARM_OBJECT_COMPILER)" \
	MAKELCF="$(MAKELCF)" MWCCARM="$(MWCCARM)" MWLDARM="$(MWLDARM)"

.DEFAULT_GOAL := help
.PHONY: help diagnose prepare map validate match progress-validate serve test \
	public-audit public-test link-bootstrap link-probe link-compare link-batch \
	link-prepare link-real link-smoke arm9i-sdk-inventory arm9i-sdk-extract \
	arm9i-sdk-validate arm9-source-plan arm9-source-compile arm9-source-stage \
	windows-check windows-sync windows-build

help:
	@echo "Four Swords Anniversary public orchestration"
	@echo
	@echo "  make diagnose          Verify user-supplied ROM preparation inputs"
	@echo "  make prepare           Prepare build/rom from verified private inputs"
	@echo "  make map               Regenerate and validate binary/NitroFS maps"
	@echo "  make validate          Validate maps against prepared artifacts"
	@echo "  make match             Run and independently verify exact SDK matches"
	@echo "  make progress-validate Validate dashboard inputs"
	@echo "  make serve             Serve the dashboard on 127.0.0.1:8765"
	@echo "  make test              Run all public test suites"
	@echo "  make public-audit      Reject private files and credentials before publication"
	@echo "  make public-test       Run tests that require no private ROM or SDK input"
	@echo "  make link-bootstrap    Generate private target fallback units under build/"
	@echo "  make link-probe        Select exact source/SDK units, fallback otherwise"
	@echo "  make link-compare      Reconstruct and compare selected units (not a real link)"
	@echo "  make link-batch        Run bootstrap, probe and compare as one checkpoint"
	@echo "  make link-prepare      Prepare validated ARM ELF objects and production LSF"
	@echo "  make link-real         Invoke the licensed CodeWarrior production linker"
	@echo "  make link-smoke        Exercise the licensed CodeWarrior toolchain on fixtures"
	@echo "  make arm9i-sdk-inventory  Verify required external SDK archive members"
	@echo "  make arm9i-sdk-extract    Stage only selected ARM9i SDK members under build/"
	@echo "  make arm9i-sdk-validate   Validate the staged ARM9i SDK member set"
	@echo "  make arm9-source-plan     Plan the default <=16 KiB public ARM9 source batch"
	@echo "  make arm9-source-compile  Compile the planned C inputs with mwccarm"
	@echo "  make arm9-source-stage    Stage linked raw units and run probe only"
	@echo "  make windows-check     Check an explicitly configured Windows host"
	@echo "  make windows-sync      Sync public files to an explicitly configured host"
	@echo "  make windows-build     Run an explicit command on that host"
	@echo
	@echo "Pass private locations through variables or environment; see docs/build-orchestration.md."

diagnose:
	$(PRIVATE_INPUT_ENV) $(ORCHESTRATOR) diagnose $(ARGS)

prepare:
	$(PRIVATE_INPUT_ENV) $(ORCHESTRATOR) prepare $(ARGS)

map:
	FSAE_ROM="$(ROM)" $(ORCHESTRATOR) map $(ARGS)

validate:
	$(ORCHESTRATOR) validate $(ARGS)

match:
	TWLSDK_ROOT="$(TWLSDK)" $(ORCHESTRATOR) match $(ARGS)

progress-validate:
	$(ORCHESTRATOR) progress-validate $(ARGS)

serve:
	$(ORCHESTRATOR) serve $(ARGS)

test:
	$(ORCHESTRATOR) test $(ARGS)

public-audit:
	$(PYTHON) tools/release/public_audit.py --scope worktree

public-test:
	$(PYTHON) -m unittest discover -s analysis/tests -v
	$(PYTHON) -m unittest discover -s tools/analysis/tests -v
	$(PYTHON) -m unittest discover -s tools/rom/tests -p 'test_pipeline.py' -v
	$(PYTHON) -m unittest discover -s tools/match/tests -v
	$(PYTHON) -m unittest discover -s tools/linker/tests -v
	$(PYTHON) -m unittest discover -s tools/progress/tests -v
	$(PYTHON) -m unittest discover -s tools/sdk/tests -v
	$(PYTHON) -m unittest discover -s tools/toolchain/tests -v
	$(PYTHON) -m unittest discover -s tools/windows/tests -v
	$(PYTHON) -m unittest discover -s tools/release/tests -v
	$(PYTHON) -m unittest discover -s tools/build/tests -v

link-bootstrap:
	$(LINKER_INCREMENTAL) bootstrap $(ARGS)

link-probe:
	$(LINKER_ENV) $(LINKER_INCREMENTAL) probe $(ARGS)

link-compare:
	$(LINKER_INCREMENTAL) compare $(ARGS)

link-batch: link-bootstrap link-probe link-compare

link-prepare:
	$(LINKER_ENV) $(CODEWARRIOR_DRIVER) prepare $(ARGS)

link-real:
	$(LINKER_ENV) $(CODEWARRIOR_DRIVER) link $(ARGS)

link-smoke:
	$(LINKER_ENV) $(CODEWARRIOR_DRIVER) smoke $(ARGS)

arm9i-sdk-inventory:
	$(LINKER_ENV) $(ARM9I_SDK_PREPARE) inventory $(ARGS)

arm9i-sdk-extract:
	$(LINKER_ENV) $(ARM9I_SDK_PREPARE) extract $(ARGS)

arm9i-sdk-validate:
	$(ARM9I_SDK_PREPARE) validate $(ARGS)

arm9-source-plan:
	test -n "$(ARM9_SOURCE_PROMOTIONS)"
	$(ARM9_SOURCE_PROVIDER) plan --promotions "$(ARM9_SOURCE_PROMOTIONS)" $(ARGS)

arm9-source-compile:
	test -n "$(ARM9_SOURCE_PROMOTIONS)"
	MWCCARM="$(MWCCARM)" $(ARM9_SOURCE_PROVIDER) compile \
		--promotions "$(ARM9_SOURCE_PROMOTIONS)" $(ARGS)

arm9-source-stage:
	test -n "$(ARM9_SOURCE_PROMOTIONS)"
	$(ARM9_SOURCE_PROVIDER) stage \
		--promotions "$(ARM9_SOURCE_PROMOTIONS)" \
		--candidate-root "$(ARM9_SOURCE_CANDIDATES)" $(ARGS)

windows-check:
	$(WINDOWS_ENV) $(ORCHESTRATOR) windows-check $(ARGS)

windows-sync:
	$(WINDOWS_ENV) $(ORCHESTRATOR) windows-sync $(ARGS)

windows-build:
	$(WINDOWS_ENV) $(ORCHESTRATOR) windows-build $(ARGS)
