/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import "PBXGDB_EvaluationSequence.h"

@class NSString;

@interface PBXGDB_InsertPluginSequence : PBXGDB_EvaluationSequence
{
    NSString *_pluginPath;
}

- (void)execute;
- (void)loadInferiorPluginFinished:(id)arg1;
- (id)pluginPath;
- (void)setPluginPath:(id)arg1;
- (void)dealloc;
- (BOOL)needsEvaluationSafetyCheck;

@end
