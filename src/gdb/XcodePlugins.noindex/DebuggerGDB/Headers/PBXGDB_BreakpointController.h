/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_Controller.h>

@class DBGDebugSession, NSMapTable, NSMutableArray, NSMutableDictionary, NSMutableSet;

@interface PBXGDB_BreakpointController : PBXGDB_Controller
{
    NSMutableSet *_breakpointSet;
    unsigned long long _pendingBreakpointCount;
    NSMutableDictionary *_watchpointDict;
    NSMapTable *_commandActionsToBreakpoints;
    NSMutableArray *_cppExceptionThrows;
    NSMutableArray *_cppExceptionCatches;
}

+ (id)_createRegexForBreakpoints:(id)arg1;
- (void)_handleDebuggerCommandChanged:(id)arg1;
- (void)_handleActionsAddedOrRemoved:(id)arg1 forBreakpoint:(id)arg2;
- (void)observeValueForKeyPath:(id)arg1 ofObject:(id)arg2 change:(id)arg3 context:(void *)arg4;
- (long long)watchpointNumberForDataValue:(id)arg1;
- (id)dataValueForWatchpointNumber:(long long)arg1;
- (void)setDataValue:(id)arg1 forWatchpointNumber:(long long)arg2;
- (BOOL)useDebuggerSideImplementationForCommandAction:(id)arg1 breakpoint:(id)arg2;
- (id)gdbCommandStringForAction:(id)arg1;
- (id)gdbCommandStringForBreakpoint:(id)arg1;
- (void)resetExceptionBreakpointData;
- (void)rescanBreakpoints;
- (unsigned long long)pendingBreakpointCount;
- (void)resolvedAllMatchesOfBreakpointInfo:(id)arg1;
- (void)enqueueStopOnCPPCatchSequence;
- (void)enqueueDeleteStopOnCPPCatchSequence;
- (void)enqueueStopOnCPPThrowSequence;
- (void)enqueueDeleteStopOnCPPThrowSequence;
- (BOOL)_breakpontsAreEnabledInArray:(id)arg1;
- (void)enqueueSetEnableBreakpointSequenceForBreakpoint:(id)arg1;
- (void)enqueueTurnBreakpointOnOffSequence:(BOOL)arg1;
- (void)_enqueueSetEnableBreakpointSequenceForBreakpoint:(id)arg1;
- (void)enqueueDeleteBreakpointSequenceForBreakpoint:(id)arg1;
- (void)_enqueueDeleteBreakpointSequenceForBreakpointInfo:(id)arg1;
- (BOOL)_findDynamicLinkerImageForBreakpointInfo:(id)arg1;
- (void)enqueueNewBreakpointSequenceForBreakpoint:(id)arg1;
- (void)setCommandForBreakpoint:(id)arg1 gdbNumber:(long long)arg2;
- (void)setConditionAndCommandForBreakpoint:(id)arg1 gdbNumber:(long long)arg2;
- (id)pathsOfProductsThatIncludeBreakpoint:(id)arg1;
- (void)setGDBNumber:(long long)arg1 forBreakpoint:(id)arg2;
- (void)_enqueueNewBreakpointSequenceForBreakpointInfo:(id)arg1;
- (void)enqueBreakpointCommandSequence:(id)arg1 forBreakpointNum:(long long)arg2;
- (void)enqueBreakpointIgnoreCountSequence:(id)arg1;
- (void)enqueBreakpointConditionSequence:(id)arg1;
- (void)_initSequence:(id)arg1 breakpoint:(id)arg2;
- (void)_initSequence:(id)arg1 breakpointInfo:(id)arg2;
- (id)_imageController;
- (void)deleteInfo:(id)arg1;
- (void)setConditionIsValid:(BOOL)arg1 forBreakpointNumber:(long long)arg2;
- (id)breakpointForGDBBreakpointNumber:(long long)arg1;
- (id)infoForBreakpoint:(id)arg1;
@property(readonly) DBGDebugSession *foundationDebugSession;
- (id)init;

@end

