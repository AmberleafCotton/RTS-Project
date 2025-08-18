# 🔧 GathererModule - Remaining Issues

> **Current issues and improvements for the GathererModule system**

This document tracks the remaining issues, improvements, and architectural decisions that need to be addressed in the GathererModule system.

---

## 🚨 **Critical Issues (High Priority)**

### ❌ **7. ResourceGathered Logic Error - Not Adding to Current Amount**
**Status**: **CRITICAL** - Needs immediate attention  
**Location**: GathererModule.cpp line 87  
**Priority**: Critical (breaks core gathering mechanics)

**Issue**: `ResourceGathered()` sets `CurrentResourceAmount = ResourceAmount` instead of adding to it

**Problem**: 
- **Current**: `CurrentResourceAmount = ResourceAmount;` (overwrites)
- **Should Be**: `CurrentResourceAmount += ResourceAmount;` (adds to existing)
- **Impact**: If MaxStorage = 2, unit will never deposit because it never accumulates resources
- **Example**: Gather 1 wood → CurrentResourceAmount = 1, gather another 1 wood → CurrentResourceAmount = 1 (not 2!)

**Fix**: Change assignment to addition in `ResourceGathered()` function

---

### ❌ **8. ResourceGathered/ResourceDeposited Should Be Event Handlers, Not Logic Controllers**
**Status**: **HIGH** - Architectural improvement needed  
**Location**: GathererModule.cpp (ResourceGathered, ResourceDeposited functions)  
**Priority**: High (architectural improvement)

**Issue**: These functions contain business logic instead of just being event handlers

**Problem**:
- **Current**: Functions contain complex logic (state changes, deposit decisions, gathering continuation)
- **Should Be**: Simple event handlers that just update state and broadcast events
- **Better Architecture**: Logic should be in separate functions like `HandleResourceGathered()`, `HandleResourceDeposited()`
- **Separation of Concerns**: Event handlers vs business logic should be separate

**Fix**: Refactor to separate event handling from business logic

---

## 🏗️ **Architectural Improvements (Medium Priority)**

### ❌ **9. Empty GatherMethod Base Class for Maximum Universality**
**Status**: **MEDIUM** - Architectural improvement  
**Location**: GatherMethod.cpp (Gather_Implementation function)  
**Priority**: High (architectural improvement)

**Issue**: GatherMethod base class contains logic that should be in children

**Problem**:
- **Current**: Base class handles target comparison, timer reset, module setup
- **Should Be**: Base class should be completely empty skeleton
- **Benefit**: Each child method becomes completely individual and self-contained
- **Impact**: Maximum flexibility for different gathering methods

**Fix**: Move all logic to NormalGathering, leave base class empty

---

### ❌ **10. Move Target Comparison Logic to Appropriate Level**
**Status**: **MEDIUM** - Architectural improvement  
**Location**: GatherMethod.cpp (bSameResource check)  
**Priority**: Medium (architectural improvement)

**Issue**: Target comparison logic is in wrong place

**Problem**:
- **Current**: Checking if target == current in GatherMethod base
- **Should Be**: This logic should be in GathererModule or individual children
- **Better Flow**: Each click should refresh whole thing via ExecuteGather
- **Architecture**: ExecuteGather should handle deposit vs gather decision based on current state

**Fix**: Remove from base class, implement in appropriate level

---

### ❌ **11. Simplify Timer Reset Logic**
**Status**: **MEDIUM** - Simplification needed  
**Location**: GatherMethod.cpp (timer reset in Gather_Implementation)  
**Priority**: Medium (simplification)

**Issue**: Timer reset logic is overly complex

**Problem**:
- **Current**: Complex logic for when to reset timer
- **Should Be**: Simple ClearTimer() call in base class
- **Universal Approach**: Always clear timer unless child method specifically doesn't want to
- **StopGather**: Should also reset timer (already happening)

**Fix**: Simplify to just ClearTimer() in base class

---

## 🎯 **High Priority Architectural Decisions**

### ❌ **6. Resource Storage Design Decision**
**Status**: **HIGH** - Design decision needed  
**Location**: GathererModule.cpp (ResourceGathered function)  
**Priority**: High (affects core gameplay balance)

**Issue**: Need to decide between stack-based vs amount-based storage system

**Description**: 
- **Current**: Using `CurrentResourceAmount` and `MaxResourceStorage` (amount-based)
- **Alternative**: Stack-based system (e.g., 3x StackAmount where StackAmount = 5)
- **Challenge**: Different resources have different gathering rates (1 wood vs 5 gold per stack)
- **Balance**: Harder to balance with different max storage per resource type

**Options**:
1. **Keep Amount-Based**: Simple, flexible, easy to balance
2. **Switch to Stack-Based**: More realistic, matches gathering mechanics
3. **Hybrid System**: Configurable per resource type

**Decision Needed**: Choose storage system approach

---

## 🔧 **API and Integration Issues**

### ❌ **14. API Mismatch With Existing Callers**
**Status**: **HIGH** - Integration blocker  
**Priority**: High (prevents integration breakage)

**Issue**: External code (e.g., `UOrderComponent`) calls `ExecuteGathererModule()` and `StopGathererModule()`, but current API is `ExecuteGather()` and `StopGather()`

**Fix**: Add wrapper functions in `UGathererModule` to forward calls to the new names

---

### ❌ **15. Movement Delegate Binding Signature**
**Status**: **HIGH** - Compile blocker  
**Priority**: High (compile blocker)

**Issue**: Using `AddDynamic` with `OnRequestFinished` and a handler signature `OnMovementCompleted(EPathFollowingResult::Type)` is incorrect

**Fix**: Use `AddUObject(this, &UGathererModule::OnMoveRequestFinished)` with signature `void OnMoveRequestFinished(FAIRequestID, const FPathFollowingResult& Result)`; keep declaration in cpp to avoid header include churn

---

### ❌ **16. Broken UE_LOG Multiline Literals**
**Status**: **HIGH** - Compile blocker  
**Priority**: High (compile blocker)

**Issue**: `ResourceDeposited()` has UE_LOG calls spanning multiple lines inside `TEXT()`

**Fix**: Consolidate to single-line `TEXT()` format strings

---

### ❌ **17. Delegate Type Consistency (Amount float vs int32)**
**Status**: **MEDIUM** - Type consistency  
**Priority**: Medium

**Issue**: `FOnResourceGathered` uses `float` for amount; code uses `int32`

**Fix**: Change delegate to `int32` or cast consistently when broadcasting; prefer `int32` for resources

---

### ❌ **18. Null Safety Guards**
**Status**: **MEDIUM** - Safety improvement  
**Priority**: Medium

**Issue**: Missing null checks on `GatherMethod`/`DepositMethod` during initialization and execution

**Fix**: Guard before calling `Initialize...`, `Gather(...)`, `Deposit(...)`

---

### ❌ **19. Remove Unused Child Movement Callbacks**
**Status**: **MEDIUM** - Code cleanup  
**Priority**: Medium

**Issue**: `GatherMethod::OnMovementCompleted` and `DepositMethod::OnMovementCompleted` are now unused after centralizing movement

**Fix**: Remove callbacks and related cached AI/path references if not used elsewhere

---

## 🎨 **Optimizations and UX Improvements**

### ❌ **13. Add IsInRange Check Before Movement (Optional Optimization)**
**Status**: **LOW** - UX optimization  
**Location**: GathererModule.cpp (MoveToLocation function)  
**Priority**: Low (optimization)

**Issue**: Unnecessary movement when already in range

**Problem**:
- **Current**: Always move to location, even if already in range
- **Should Be**: Check if already in range before moving
- **Benefit**: Skip unnecessary movement for 2nd+ gathering attempts
- **Example**: Unit already at wood resource, click gather again → skip movement, start gathering immediately
- **Implementation**: Add IsInRange() check before MovementModule->MoveToLocation()

**Opinion**: Good optimization for better UX, especially for repeated gathering on same resource

**Fix**: Add range check in MoveToLocation, skip movement if already in range

---

## 🧹 **Cleanup and Maintenance**

### ❌ **4. Divide by Zero Warning in Blueprint**
**Status**: **LOW** - Blueprint warning  
**Priority**: Low

**Issue**: Blueprint is showing "Divide by zero: Divide_DoubleDouble" warning

**Location**: Blueprint side (W_Gatherer_C)

**Fix**: Check Blueprint logic for division operations that might result in zero

---

### ❌ **5. Log Cleanup Needed**
**Status**: **LOW** - Code cleanup  
**Priority**: Low (after all issues are resolved)

**Issue**: Excessive logging is bloating the output and files

**Location**: Multiple files throughout the system

**Fix**: Remove debug logs and keep only essential error/warning logs

---

### ❌ **20. Clean DIRTYNOTES From TODO**
**Status**: **LOW** - Documentation cleanup  
**Priority**: Low

**Issue**: Raw code notes appended under `DIRTYNOTES` in this file

**Fix**: Remove after implementing Tasks 9–11 and refactors

---

## 📊 **Issue Summary**

### **Priority Breakdown**
- **🚨 Critical**: 1 issue (Resource accumulation bug)
- **🔴 High**: 6 issues (Architectural improvements, API fixes)
- **🟡 Medium**: 4 issues (Code cleanup, optimizations)
- **🟢 Low**: 3 issues (Documentation, warnings)

### **Category Breakdown**
- **Critical Bugs**: 1 issue
- **Architectural Improvements**: 4 issues
- **API/Integration**: 5 issues
- **Optimizations**: 1 issue
- **Cleanup**: 3 issues

### **Estimated Effort**
- **Quick Fixes** (< 1 hour): 6 issues
- **Medium Effort** (1-4 hours): 8 issues
- **Major Refactoring** (4+ hours): 2 issues

---

## 🎯 **Recommended Action Plan**

### **Phase 1: Critical Fixes (Immediate)**
1. **Fix Resource Accumulation Bug (#7)** - Critical functionality
2. **Fix Compile Blockers (#15, #16)** - Development can't continue
3. **Add API Wrappers (#14)** - Integration compatibility

### **Phase 2: Architectural Improvements (Next Sprint)**
1. **Refactor Event Handlers (#8)** - Clean separation of concerns
2. **Empty Base Class (#9)** - Maximum flexibility
3. **Move Target Logic (#10)** - Better architecture
4. **Simplify Timer Logic (#11)** - Code cleanup

### **Phase 3: Polish and Cleanup (Future)**
1. **Add Range Check (#13)** - UX optimization
2. **Clean up Logs (#5)** - Performance improvement
3. **Remove Unused Code (#19, #20)** - Code maintenance
4. **Fix Blueprint Warning (#4)** - Quality improvement

---

## 📚 **Related Documentation**

- **[Completed Fixes](./FIXED.md)** - All resolved issues and improvements
- **[Main System README](../README.md)** - Complete system overview
- **[GatherMethod README](../GatherMethod/README.md)** - Gathering strategy documentation
- **[DepositMethod README](../DepositMethod/README.md)** - Depositing strategy documentation

---

*This document tracks the remaining work needed to complete the GathererModule system. Issues are prioritized by impact and effort required.*

