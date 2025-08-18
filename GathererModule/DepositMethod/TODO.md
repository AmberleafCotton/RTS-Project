# DepositMethod TODO

## Issues

### 1. TargetResource usage in Deposit methods
- Question: Do we need a separate TargetResource stored in DepositMethod, or should all deposit operations reference `UGathererModule::TargetResource`?
- Current: Deposit functions receive `ARTS_Actor* Target` parameter and can access `GathererModule->TargetResource` if needed.
- Proposal: Remove any duplicated `TargetResource` state from DepositMethod to avoid desync; rely on `UGathererModule::TargetResource` or explicit parameters.
- Priority: Medium
- Next steps: Audit `InstantDeposit` and `NormalDeposit` for any internal Target state; remove if redundant.

### 2. Movement responsibilities centralized
- Note: Movement has been centralized in `UGathererModule`. Ensure DepositMethod has no leftover movement state (bindings, cached path, etc.).
- Priority: High

### 3. Event callback simplification
- Note: If `OnMovementCompleted` in DepositMethod is unused after centralization, remove it and call `CompleteDepositing()` from `UGathererModule` flow as needed.
- Priority: Medium
