# GathererModule TODO List

## Critical Bugs Found

### 7. ResourceGathered Logic Error - Not Adding to Current Amount
**Issue**: `ResourceGathered()` sets `CurrentResourceAmount = ResourceAmount` instead of adding to it
**Location**: GathererModule.cpp line 87
**Problem**: 
- **Current**: `CurrentResourceAmount = ResourceAmount;` (overwrites)
- **Should Be**: `CurrentResourceAmount += ResourceAmount;` (adds to existing)
- **Impact**: If MaxStorage = 2, unit will never deposit because it never accumulates resources
- **Example**: Gather 1 wood → CurrentResourceAmount = 1, gather another 1 wood → CurrentResourceAmount = 1 (not 2!)
**Priority**: Critical (breaks core gathering mechanics)
**Fix**: Change assignment to addition

### 8. ResourceGathered/ResourceDeposited Should Be Event Handlers, Not Logic Controllers
**Issue**: These functions contain business logic instead of just being event handlers
**Location**: GathererModule.cpp (ResourceGathered, ResourceDeposited functions)
**Problem**:
- **Current**: Functions contain complex logic (state changes, deposit decisions, gathering continuation)
- **Should Be**: Simple event handlers that just update state and broadcast events
- **Better Architecture**: Logic should be in separate functions like `HandleResourceGathered()`, `HandleResourceDeposited()`
- **Separation of Concerns**: Event handlers vs business logic should be separate
**Priority**: High (architectural improvement)
**Fix**: Refactor to separate event handling from business logic

## Architectural Improvements

### 9. Empty GatherMethod Base Class for Maximum Universality
**Issue**: GatherMethod base class contains logic that should be in children
**Location**: GatherMethod.cpp (Gather_Implementation function)
**Problem**:
- **Current**: Base class handles target comparison, timer reset, module setup
- **Should Be**: Base class should be completely empty skeleton
- **Benefit**: Each child method becomes completely individual and self-contained
- **Impact**: Maximum flexibility for different gathering methods
**Priority**: High (architectural improvement)
**Fix**: Move all logic to NormalGathering, leave base class empty

### 10. Move Target Comparison Logic to Appropriate Level
**Issue**: Target comparison logic is in wrong place
**Location**: GatherMethod.cpp (bSameResource check)
**Problem**:
- **Current**: Checking if target == current in GatherMethod base
- **Should Be**: This logic should be in GathererModule or individual children
- **Better Flow**: Each click should refresh whole thing via ExecuteGather
- **Architecture**: ExecuteGather should handle deposit vs gather decision based on current state
**Priority**: Medium (architectural improvement)
**Fix**: Remove from base class, implement in appropriate level

### 11. Simplify Timer Reset Logic
**Issue**: Timer reset logic is overly complex
**Location**: GatherMethod.cpp (timer reset in Gather_Implementation)
**Problem**:
- **Current**: Complex logic for when to reset timer
- **Should Be**: Simple ClearTimer() call in base class
- **Universal Approach**: Always clear timer unless child method specifically doesn't want to
- **StopGather**: Should also reset timer (already happening)
**Priority**: Medium (simplification)
**Fix**: Simplify to just ClearTimer() in base class

### 13. Add IsInRange Check Before Movement (Optional Optimization)
**Issue**: Unnecessary movement when already in range
**Location**: GathererModule.cpp (MoveToLocation function)
**Problem**:
- **Current**: Always move to location, even if already in range
- **Should Be**: Check if already in range before moving
- **Benefit**: Skip unnecessary movement for 2nd+ gathering attempts
- **Example**: Unit already at wood resource, click gather again → skip movement, start gathering immediately
- **Implementation**: Add IsInRange() check before MovementModule->MoveToLocation()
**Priority**: Low (optimization)
**Opinion**: Good optimization for better UX, especially for repeated gathering on same resource
**Fix**: Add range check in MoveToLocation, skip movement if already in range

## High Priority Architectural Tasks

### 6. Resource Storage Design Decision
**Issue**: Need to decide between stack-based vs amount-based storage system
**Location**: GathererModule.cpp (ResourceGathered function)
**Description**: 
- **Current**: Using `CurrentResourceAmount` and `MaxResourceStorage` (amount-based)
- **Alternative**: Stack-based system (e.g., 3x StackAmount where StackAmount = 5)
- **Challenge**: Different resources have different gathering rates (1 wood vs 5 gold per stack)
- **Balance**: Harder to balance with different max storage per resource type
**Options**:
1. **Keep Amount-Based**: Simple, flexible, easy to balance
2. **Switch to Stack-Based**: More realistic, matches gathering mechanics
3. **Hybrid System**: Configurable per resource type
**Priority**: High (affects core gameplay balance)
**Dependencies**: None

## Issues Found During Testing

### 22. Flow Review – ExecuteGather → NormalGathering (Method_01)
Final decision: keep a single movement-complete path that always re-enters `ExecuteGather()`; `GatherMethod` controls policy/flow.

Chosen loop (Option A – method-driven with re-entry):
- ExecuteGather(Target): sets state Gathering, calls `GatherMethod->Gather(Target)`
- NormalGathering::Gather(Target):
  - Pre-checks: owner valid, gatherable valid, storage/type policy
  - If storage full or type mismatch → `GathererModule->RequestDeposit()` and return
  - Else request slot; if within acceptance radius → `StartGathering()`; else → `MoveToLocation(SlotLocation)`
- Movement success: `UGathererModule::OnMovementCompleted` ALWAYS calls `ExecuteGather(Target)` (no branching by state)
- StartGathering(): set timer and `RequiredGatheringTime`; tick → `CompleteGathering()` → `HarvestResource`
- On harvest: `ResourceGathered(Amount, Type)`; method decides next step:
  - If method’s policy says full → `RequestDeposit()`
  - Else → `RequestContinueGather()`
- DepositMethod::Deposit(Target): idempotent
  - If already at deposit location → `CompleteDepositing()`
  - Else → `MoveToLocation(DepositLocation)`; on movement success the loop re-enters `ExecuteGather()`, which leads method to request `Deposit()` again and it completes.

### 23. Ownership of Storage Policy and Transition Orchestration (Final Decision)
Decision: Method-driven policy with neutral module and single re-entry on movement success. No separate arrival callbacks or dual bindings.

Key points:
- GatherMethod owns storage policy and readiness checks (range, tools, type switching)
- GathererModule is coordinator: holds light state, owns movement, broadcasts events
- Movement completion always re-enters `ExecuteGather()`; methods handle next intent
- DepositMethod is idempotent and proximity-aware; no separate movement-complete hook

Implementation plan (Tasks 22–23):
1) GatherMethod pre-checks (NormalGathering)
   - Ensure `GatherableModule` is set per target
   - Early-return to `RequestDeposit()` when storage full or carrying different type than target
   - Guard `Owner` validity before distance checks
2) Slot handling
   - Request slot once per target; consider tracking `CurrentGatheringTarget` to avoid thrash
3) Start/Complete/Tick gathering
   - Start timer with `RequiredGatheringTime = GatherableModule->GatheringTime`
   - On complete: call `ResourceGathered(Amount, Type)`, then branch via method policy to `RequestDeposit()` or `RequestContinueGather()`
4) DepositMethod behavior (NormalDeposit)
   - On `Deposit(Target)`: compute deposit location, check in-range (match acceptance radius); if in-range → `CompleteDepositing()`; else → `MoveToLocation()`
   - Idempotent: safe to call repeatedly; no internal movement event bindings
5) Movement completion
   - Keep `UGathererModule::OnMovementCompleted(...)` calling `ExecuteGather(Target)` on success
   - On non-success: unbind events and optionally retry/abort (log + early return for now)
6) API & integration
   - Add wrappers `ExecuteGathererModule()` and `StopGathererModule()` to maintain compatibility with existing callers
   - Ensure delegates use consistent types (resource amount as `int32`)
7) Events as handlers
   - `ResourceGathered` and `ResourceDeposited` only update state + broadcast (no branching)

Acceptance criteria:
- Repeated call loop gather → deposit → gather works with a single movement-complete re-entry
- Arriving at deposit location triggers deposit without special arrival hook (via method re-check + idempotent deposit)
- Switching target resource while carrying different type first deposits, then proceeds to new gather
- Movement failure does not leave stale bindings
- No compile warnings from mismatched delegate signatures or multiline UE_LOG
- Existing code calling old API names still works

Files to change:
- `GathererModule.h/.cpp`: wrappers, movement failure handling, minor state guards
- `GatherMethod/NormalGathering.h/.cpp`: pre-checks, gatherable caching, owner guard, policy decisions
- `DepositMethod/NormalDeposit.h/.cpp`: in-range check, idempotent `Deposit()`, no event bindings
- `DepositMethod/InstantDeposit.cpp`: sanity pass (no movement, stays idempotent)
- `Components/OrderComponent/*`: ensure wrappers are used or adapt calls

### 23a. Decision: Adopt Method-Driven Policy (Go Forward Plan)
Summary (agreed):
- GatherMethod is the “brain”: owns readiness (e.g., IsInRange), storage policy (fullness), and method-specific prerequisites (tools, etc.).
- GathererModule is the coordinator: movement + event wiring; executes transitions requested by methods.
- Deposit methods remain simple strategies.

Immediate refactor checklist:
- NG-1 (GatherableModule readiness): Ensure method sets/validates GatherableModule before using timing data. Status: Pending.
- NG-6 (Owner guard): Ensure Owner is valid before distance checks. Status: Pending.
- Storage pre-check in `NormalGathering::Gather()`:
  - If method-local storage is full → `RequestDeposit()` and return (avoid movement loop). Status: Pending.
- Resource-type switching policy:
  - If carrying type A and target is type B → decide (likely deposit first). Status: Pending.
- Arrival callbacks (Option B):
  - Add `OnArrivedAtGather()` and `OnArrivedAtDeposit()`; stop re-entering generic `ExecuteGather()` on movement success. Status: Pending.

What to include in each Gather method (per-call gate):
- IsInRange? (method-specific; some methods may not require range)
- IsStorageFull? (method-local MaxStackStorage/CurrentStackStorage)
- Do we have required tools/conditions? (optional)
- Is slot reserved/held, or do we need to request one?

Naming convention (agreed direction):
- Short class names (e.g., `UGatherMethod_01`, `UGatherMethod_02`).
- Rich metadata: DisplayName + ToolTip summarize behavior; separate markdown catalog with examples.

Refactor plan (phased):
1) Method-local storage scaffold in `NormalGathering` (fields: `MaxStackStorage`, `CurrentStackStorage`) and pre-check in `Gather()` to deposit when full.
2) Guard rails: ensure Owner and GatherableModule are valid before use; tighten distance check.
3) Resource-type switching policy inside `NormalGathering` (deposit when mismatch).
4) Arrival callbacks in module and method/deposit to avoid `ExecuteGather` re-entry.
5) Extract naming metadata (DisplayName/ToolTip) and create `docs/Methods.md` catalog.

### 24. NormalGathering (Method_01) – Readiness Checklist and Refactor Plan
Summary of intent (from discussion):
- Method is the brain: decides gather vs deposit vs move; owns storage policy and readiness checks.
- Module coordinates movement and transitions based on method requests.

Current checks in `NormalGathering::Gather()`:
- Present: GathererModule null guard.
- Present: Slot request and distance check → StartGathering or MoveToLocation.
- Missing: GatherableModule readiness before using `GatheringTime` in `StartGathering()`.
- Missing: Method-local storage fields (`MaxStackStorage`, `CurrentStackStorage`).
- Missing: Pre-check at start of `Gather()` to `RequestDeposit()` when full.
- Missing: Resource-type switching policy (carrying A, targeting B → deposit?).
- Missing: Owner validity guard for distance check (avoid ZeroVector fallback).
- Consider: Avoid re-requesting slot when already held.

Refactor steps for NormalGathering (target):
1) Add method-local storage:
   - `int32 MaxStackStorage = 1;`
   - `int32 CurrentStackStorage = 0;`
2) At start of `Gather()`:
   - Validate `GatherableModule` once per target (cache per `CurrentGatheringTarget`).
   - If storage full → `GathererModule->RequestDeposit();` and return.
   - If Owner is null → return with warning.
3) On `CompleteGathering()`:
   - Increment `CurrentStackStorage` by harvested logical stack amount(1 per full gather); call `ResourceGathered` for eventing only(send ResourceAmount (not number of s tacks)gathered).
   - Decide next step: `RequestDeposit` vs `RequestContinueGather`.
4) Resource-type switching:
   - If carrying type differs from target type, prefer deposit first (method policy).
5) Slot handling:
   - Track if a slot is already held for `CurrentGatheringTarget`; avoid re-requesting.
6) Consider arrival callbacks (future step from 23a) to reduce re-entrance into `Gather()`.

Nuances / potential stalls:
- NG-1: GatherableModule not initialized in `NormalGathering::Gather()` but used in `StartGathering()` and `CompleteGathering()`; must ensure it’s set before use
- NG-2: Movement failure (non-success `FPathFollowingResult`) leaves events bound and no recovery; consider unbinding and retry/fallback
- NG-3: Slot acquisition every Gather call may thrash; consider tracking slot ownership to avoid repeated `TakeSlot`
- NG-4: After `RequestDeposit()`, arrival at deposit location does not trigger `CompleteDepositing()`; deposit method no longer gets a movement-complete callback; define a deposit arrival hook
- NG-5: Resource type switching (carrying A, target B) no longer auto-deposits; policy should live in method; current `NormalGathering` does not handle it
- NG-6: Owner null-path in distance check uses ZeroVector, could start gathering incorrectly; better to early-return if no Owner

Proposed fixes:
- F-1: In `NormalGathering::Gather()`, set `GatherableModule = URTSModuleFunctionLibrary::GetGatherableModule(Target)` if null
- F-2: In `UGathererModule::OnMovementCompleted`, handle non-success: `UnbindMovementEvents()` and decide retry/abort
- F-3: Track `CurrentGatheringTarget`/slot state to avoid re-requesting slot if unchanged
- F-4: Introduce a deposit arrival pathway: on move success while depositing, call `DepositMethod->CompleteDepositing()` before resuming gather
- F-5: Add method-level policy for resource-type switching (auto-deposit when carrying different type)
- F-6: Guard `Owner` in distance check; if null, return and/or log

### 4. Divide by Zero Warning in Blueprint
**Issue**: Blueprint is showing "Divide by zero: Divide_DoubleDouble" warning
**Location**: Blueprint side (W_Gatherer_C)
**Fix**: Check Blueprint logic for division operations that might result in zero
**Priority**: Low

### 5. Log Cleanup Needed
**Issue**: Excessive logging is bloating the output and files
**Location**: Multiple files throughout the system
**Fix**: Remove debug logs and keep only essential error/warning logs
**Priority**: Low (after all issues are resolved)

## Completed Fixes

### ✅ Movement Delegate Binding Signature (Engine Native Delegate)
- Issue: Using `AddDynamic` with `UPathFollowingComponent::OnRequestFinished` caused `__Internal_AddDynamic` compile error; handler signature also mismatched.
- Fix: Switched to `AddUObject` with a handler `OnMoveRequestFinished(FAIRequestID, const FPathFollowingResult&)` and bridged to `OnMovementCompleted(Result.Code)`.
- Status: Resolved

### ✅ Gameplay Tag Issue
**Issue**: `"PlayerResources"` tag was missing, causing deposit failures
**Fix**: Changed to `"Module.PlayerResources"` tag
**Status**: Resolved

### ✅ Initialization Order Issue
**Issue**: Owner wasn't set before initializing child methods
**Fix**: Proper initialization order in `GathererModule::InitializeModule_Implementation()`
**Status**: Resolved

### ✅ Task 12: Move MoveToLocation to GathererModule
**Issue**: Movement logic was scattered across different classes
**Location**: GatherMethod.cpp, DepositMethod.cpp (MoveToLocation calls)
**Fix**: Centralized movement logic in GathererModule with MovementModule integration
**Implementation**: 
- Added `MoveToLocation()`, `StopMovement()`, `OnMovementCompleted()` to GathererModule
- Added `BindMovementEvents()`, `UnbindMovementEvents()` for centralized event handling
- Removed movement functions from GatherMethod and DepositMethod classes
- Updated NormalGathering and NormalDeposit to call `GathererModule->MoveToLocation()`
- **Simplified OnMovementCompleted**: Now just calls `ExecuteGather()` to let it handle all logic
**Status**: Resolved
**Benefits**: Single place for movement logic, easier to manage, centralized event binding/unbinding, simplified completion handling

## Notes

- The core gather → deposit → gather loop is now working correctly
- All major functionality is operational
- These are mostly polish/optimization issues rather than critical bugs
- Resource storage design is a key architectural decision that affects gameplay balance
- Critical bugs #7 and #8 need immediate attention as they break core functionality
- Architectural improvements #9-11 will make the system more modular and flexible
- Task #13 is a nice UX optimization for repeated gathering
- Task #12 has been successfully completed with centralized movement logic




DIRTYNOTES

	// Base implementation - just set up modules
	GatherableModule = URTSModuleFunctionLibrary::GetGatherableModule(TargetResource); 
	
	if (!GatherableModule)
	{
		return;
	}
	
	// Check if we're already gathering this resource
	bool bSameResource = (CurrentGatheringTarget == TargetResource);
	
	// Update current target
	CurrentGatheringTarget = TargetResource;
	RequiredGatheringTime = GatherableModule->GatheringTime;
	
	// Reset timer if different resource or no current target
	if (!bSameResource)
	{
		CurrentGatheringTime = 0.f;
		// Clear any existing timer
		if (GathererModule && GathererModule->GetWorld())
		{
			GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
		}
	}
	
	// Child classes will handle the specific gathering logic



	// should be used by exact method that wants to use it

	// Requesting might move it to child class that actually uses it. Will think about it.


## New Issues

### 14. API Mismatch With Existing Callers
- Issue: External code (e.g., `UOrderComponent`) calls `ExecuteGathererModule()` and `StopGathererModule()`, but current API is `ExecuteGather()` and `StopGather()`
- Fix: Add wrapper functions in `UGathererModule` to forward calls to the new names
- Priority: High (prevents integration breakage)

### 15. Movement Delegate Binding Signature
- Issue: Using `AddDynamic` with `OnRequestFinished` and a handler signature `OnMovementCompleted(EPathFollowingResult::Type)` is incorrect
- Fix: Use `AddUObject(this, &UGathererModule::OnMoveRequestFinished)` with signature `void OnMoveRequestFinished(FAIRequestID, const FPathFollowingResult& Result)`; keep declaration in cpp to avoid header include churn
- Priority: High (compile blocker)

### 16. Broken UE_LOG Multiline Literals
- Issue: `ResourceDeposited()` has UE_LOG calls spanning multiple lines inside `TEXT()`
- Fix: Consolidate to single-line `TEXT()` format strings
- Priority: High (compile blocker)

### 17. Delegate Type Consistency (Amount float vs int32)
- Issue: `FOnResourceGathered` uses `float` for amount; code uses `int32`
- Fix: Change delegate to `int32` or cast consistently when broadcasting; prefer `int32` for resources
- Priority: Medium

### 18. Null Safety Guards
- Issue: Missing null checks on `GatherMethod`/`DepositMethod` during initialization and execution
- Fix: Guard before calling `Initialize...`, `Gather(...)`, `Deposit(...)`
- Priority: Medium

### 19. Remove Unused Child Movement Callbacks
- Issue: `GatherMethod::OnMovementCompleted` and `DepositMethod::OnMovementCompleted` are now unused after centralizing movement
- Fix: Remove callbacks and related cached AI/path references if not used elsewhere
- Priority: Medium

### 20. Clean DIRTYNOTES From TODO
- Issue: Raw code notes appended under `DIRTYNOTES` in this file
- Fix: Remove after implementing Tasks 9–11 and refactors
- Priority: Low

## High-Priority Architectural Decision

### 21. Define Responsibilities of ResourceGathered/ResourceDeposited and Storage Ownership
**Context**: The functions `UGathererModule::ResourceGathered()` and `UGathererModule::ResourceDeposited()` should likely act as event handlers that set state to “after gathered” / “after deposited”, broadcast events, and optionally call `ExecuteGather()` to continue the loop. They should not decide whether to deposit or continue gathering. Decisions like storage fullness, stacks, and “when to deposit” may belong to the active `GatherMethod` (e.g., `UNormalGathering`). The `UGathererModule` can hold `CurrentResourceAmount` but should not enforce storage limits.

**Current**:
- `ResourceGathered()` currently mutates amount and decides to deposit or gather again based on `MaxResourceStorage`.
- `ResourceDeposited()` resets amount and restarts the loop.
- `UGathererModule` owns `MaxResourceStorage` and uses it to branch logic.

**Issues**:
- Mixed responsibilities: event handling + business logic in the same functions.
- Storage policy (max, stacks) is method-specific but handled in `UGathererModule`.
- Harder to extend new gathering methods with different storage/stack behavior.

**Options**:
1) Keep logic in `UGathererModule` (status quo)
- Pros: Central decision point; fewer calls across classes
- Cons: Less flexible; couples gather/deposit policy to module; harder to extend

2) Move policy to `GatherMethod` (Recommended)
- Pros: Maximum flexibility per method (amount vs stacks, unique rules);
  `GathererModule` becomes neutral coordinator; cleaner SoC
- Cons: Slightly more code in methods; need clear method-to-module signaling

3) Hybrid: `GathererModule` holds current amount as passive data, while methods compute and instruct next step explicitly
- Pros: Clear ownership; explicit transitions (`RequestDeposit()`, `RequestContinueGather()`)
- Cons: Requires a small signaling API

**My Suggestions**:
- Treat `ResourceGathered()` and `ResourceDeposited()` as pure event handlers: Agree
  - Update simple state only (e.g., `CurrentResourceAmount`, `CurrentResourceType`) Agree
  - Broadcast their delegates Agree
  - Optionally call `ExecuteGather()` (no storage branching) Not sure
- Move storage branching (full/stack logic) into `GatherMethod` implementations Agree
- Add signaling API from methods to module if needed (`RequestDeposit`, `RequestContinueGather`) I don't get it, any code example?
- Decide final home for storage configuration (module vs method) Method
- Update docs to reflect new responsibilities add as task to do inside this task
- Revisit Critical Bug #7 after decision (accumulation still needs correction; clamp policy likely in methods), yes i think its gonna be fixed when we make acumulation inside GatherMethod.


### 21a. Implementation Notes and Examples
- Where to call ExecuteGather?
  - Never from `ResourceGathered()` / `ResourceDeposited()`; keep them event-only
  - Movement success always calls `ExecuteGather()` (single entrypoint)
  - Methods signal intent via `RequestDeposit()` / `RequestContinueGather()` only

- Minimal signaling API in the module: IMPLEMENTED (RequestDeposit, RequestContinueGather)
```cpp
// GathererModule.h
void RequestDeposit();            // sets state and triggers deposit
void RequestContinueGather();     // sets state and triggers gather
```
```cpp
// GathererModule.cpp
void UGathererModule::RequestDeposit() {
	CurrentState = EGathererState::Depositing;
	if (DepositMethod && TargetResource.IsValid()) {
		DepositMethod->Deposit(TargetResource.Get());
	}
}

void UGathererModule::RequestContinueGather() {
	CurrentState = EGathererState::Gathering;
	if (GatherMethod && TargetResource.IsValid()) {
		// Either call the method directly or route via ExecuteGather for a single entrypoint
		GatherMethod->Gather(TargetResource.Get());
		// or: ExecuteGather(TargetResource.Get());
	}
}
```

- Method-driven decision point (example in `NormalGathering::CompleteGathering`):
```cpp
and here we would also call ResourceGathered.
if (IsStorageFullByMethodRules()) {
	GathererModule->RequestDeposit();
} else {
	GathererModule->RequestContinueGather();
}
```

- Event-only handlers in the module: IMPLEMENTED (ResourceGathered/Deposited refactor, accumulation fix)
```cpp
void UGathererModule::ResourceGathered(int32 Amount, EResourceType Type) {
	CurrentResourceAmount += Amount;
	CurrentResourceType = Type;
	OnResourceGathered.Broadcast(TargetResource.Get(), Amount);
	// No branching here
}

Agree

void UGathererModule::ResourceDeposited(int32 DepositedAmount, EResourceType Type) {
	CurrentResourceAmount = 0;
	OnResourceDeposited.Broadcast(Type, DepositedAmount);
	// Optionally auto-loop here, but better to let methods drive next step
}
Agree
```

- Summary:
  - Methods decide the policy (stacks/limits/when to deposit) AGREE
  - Module executes the requested transition and maintains light state
  - `ResourceGathered`/`ResourceDeposited` remain clean, predictable event points - perfect

