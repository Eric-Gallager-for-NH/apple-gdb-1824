/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_EvalOperation.h>

@class NSString;

@interface PBXGDB_ObjCDynamicTypeOperation : PBXGDB_EvalOperation
{
    NSString *_objectAddressString;
}

- (void)perform;
- (void)sequenceDidFinish:(id)arg1;
- (id)objectAddressString;
- (void)setObjectAddressString:(id)arg1;

@end

