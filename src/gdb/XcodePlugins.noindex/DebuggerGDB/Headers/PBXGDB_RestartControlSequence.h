/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_ControlSequence.h>

@interface PBXGDB_RestartControlSequence : PBXGDB_ControlSequence
{
    BOOL _interruptIfNeeded;
}

- (void)execute;
- (void)didFinish;
- (void)_didFinish:(id)arg1;
- (BOOL)interruptIfNeeded;
- (id)init;

@end

