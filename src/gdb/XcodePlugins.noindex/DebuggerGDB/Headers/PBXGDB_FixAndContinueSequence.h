/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_CommandSequence.h>

@class NSString;

@interface PBXGDB_FixAndContinueSequence : PBXGDB_CommandSequence
{
    NSString *_objectFile;
    NSString *_sourceFile;
    NSString *_patchFile;
    BOOL _didFix;
    id _target;
    NSString *_errorMessage;
    NSString *_shlib;
}

+ (id)sequenceWithPatchFile:(id)arg1 sourceFile:(id)arg2 objectFile:(id)arg3 shlib:(id)arg4;
- (BOOL)didFix;
- (void)setTarget:(id)arg1;
- (id)target;
- (void)didFinish;
- (void)end;
- (BOOL)handleErrorForMICommand:(id)arg1;
- (void)setErrorMessage:(id)arg1;
- (id)errorMessage;
- (void)execute;
- (void)_didFinish:(id)arg1;
- (BOOL)interruptIfNeeded;
- (void)setSourceFile:(id)arg1;
- (void)setPatchFile:(id)arg1;
- (void)setObjectFile:(id)arg1;
- (void)setShlib:(id)arg1;
- (id)shlib;
- (id)sourceFile;
- (id)patchFile;
- (id)objectFile;

@end

