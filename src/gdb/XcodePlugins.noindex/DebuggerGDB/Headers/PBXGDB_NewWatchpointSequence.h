/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_BaseBreakpointSequence.h>

@class NSString;

@interface PBXGDB_NewWatchpointSequence : PBXGDB_BaseBreakpointSequence
{
    NSString *_expression;
    unsigned int _tries;
}

+ (id)sequenceWithExpression:(id)arg1;
- (BOOL)handleErrorForMICommand:(id)arg1;
- (void)execute;
- (void)_watchpointWasEnabled:(id)arg1;

@end

