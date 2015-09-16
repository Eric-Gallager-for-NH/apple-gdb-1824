/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import "PBXGDB_DataValue2.h"

@class NSMutableDictionary, NSMutableSet;

@interface PBXGDB_ContainingDataValue2 : PBXGDB_DataValue2
{
    NSMutableDictionary *_varObjToDataValueDict;
    NSMutableSet *_secondaryUpdateDataValues;
    long long _pendingSecondaryUpdateDVs;
    NSMutableSet *_newlySetDynamicTypeDataValues;
}

- (void)requestUpdateValues;
- (void)addDataValueWithNewSetDynamicType:(id)arg1;
- (void)didUpdateValues;
- (void)dataValueDidSecondaryUpdate:(id)arg1;
- (void)invalidateAllChildDataValuesCache;
- (id)secondaryUpdateDataValues;
- (id)topLevelDataValues;
- (id)dataValueForVarObjName:(id)arg1;
- (void)addVarObjName:(id)arg1 forDataValue:(id)arg2;
- (void)updateVarObjsMappingInDV:(id)arg1;
- (void)debuggingDidEnd:(id)arg1;
- (void)dealloc;
- (id)initWithDebugger:(id)arg1;

@end
