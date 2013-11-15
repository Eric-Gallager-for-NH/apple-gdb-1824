/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_EvaluationSequence.h>

@class NSString, PBXGDB_MIMemoryViewerReadCommand;

@interface PBXGDB_FetchMemoryForViewerSequence : PBXGDB_EvaluationSequence
{
    id _resultHandler;
    NSString *_viewerIdentifier;
    PBXGDB_MIMemoryViewerReadCommand *_miCommand;
}

- (BOOL)handleErrorForMICommand:(id)arg1;
- (void)gotMemory:(id)arg1;
- (void)execute;
- (BOOL)interruptIfNeeded;
- (void)setThreadContext:(long long)arg1 andStackFrameContext:(long long)arg2;
- (id)initWithAddress:(id)arg1 numberOfBytes:(unsigned long long)arg2 resultHandler:(id)arg3;

@end

